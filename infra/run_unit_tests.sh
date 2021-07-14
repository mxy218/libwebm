#!/bin/bash
# Copyright (c) 2021, Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#
#   * Neither the name of Google nor the names of its contributors may
#     be used to endorse or promote products derived from this software
#     without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

set -xeo pipefail
readonly GOOGLETEST_REPO="https://github.com/google/googletest.git"
readonly LIBWEBM_ROOT="$(realpath "$(dirname "$0")/..")"
WORKSPACE=${WORKSPACE:-"$(mktemp -d)"}

# shellcheck source=infra/common.sh
source "${LIBWEBM_ROOT}/infra/common.sh"

usage() {
  cat << EOF
Usage: $(basename "$0") TARGET
Options:
TARGET supported targets: (x86-asan, x86-ubsan, x86_64-asan, x86_64-ubsan,
       x86_64-valgrind)
Global variables:
WORKSPACE directory where the build is done
EOF
}

#######################################
# Run valgrind
#######################################
run_valgrind() {
  valgrind \
    --leak-check=full \
    --show-reachable=yes \
    --track-origins=yes \
    --error-exitcode=1 \
    "$@"
}

#######################################
# Ensure GoogleTest repository is setup
#
# Globals:
#  GOOGLETEST_REPO googletest repository url
#  WORKSPACE directory where the build is done
#######################################
ensure_googletest() {
  local googletest_dir
  googletest_dir="${WORKSPACE}/googletest"

  [[ -d "${googletest_dir}" ]] \
    || git clone --depth 1 "${GOOGLETEST_REPO}" "${googletest_dir}"

  if ! git -C "${googletest_dir}" pull; then
    rm -rf "${googletest_dir}"
    git clone --depth 1 "${GOOGLETEST_REPO}" "${googletest_dir}"
  fi

  opts+=("-DGTEST_SRC_DIR=${googletest_dir}")
}

################################################################################
echo "Unit testing libwebm in ${WORKSPACE}"

if [[ ! -d "${WORKSPACE}" ]]; then
  log_err "${WORKSPACE} directory does not exist"
  exit 1
fi

TARGET=${1:? "$(
  echo
  usage
)"}
BUILD_DIR="${WORKSPACE}/tests-${TARGET}"

# Create a fresh build directory.
trap cleanup EXIT
make_build_dir "${BUILD_DIR}"

case "${TARGET}" in
  x86-*) CXX='clang++ -m32' ;;
  x86_64-*) CXX=clang++ ;;
  *)
    log_err "${TARGET} should have x86 or x86_64 prefix."
    usage
    exit 1
    ;;
esac
# cmake (3.4.3) will only accept the -m32 variants when used via the CXX env
# var.
export CXX
opts+=("-DCMAKE_BUILD_TYPE=Debug" "-DENABLE_TESTS=ON")
case "${TARGET}" in
  *-asan) opts+=("-DCMAKE_CXX_FLAGS=-fsanitize=address") ;;
  *-ubsan) opts+=("-DCMAKE_CXX_FLAGS=-fsanitize=integer") ;;
  *) ;; # No additional flags needed.
esac

ensure_googletest
# Using pushd instead of -S/-B due to backward compatibility with CMake < 3.13.x
pushd "${BUILD_DIR}"
cmake "${LIBWEBM_ROOT}" "${opts[@]}"
make -j 4
popd

SANITIZER_LOG="${BUILD_DIR}/sanitizer_log"
UNIT_TESTS=()
while IFS='' read -r line; do
  UNIT_TESTS+=("${line}")
done < <(find "${BUILD_DIR}" -name '*_tests')

export LIBWEBM_TEST_DATA_PATH="${LIBWEBM_ROOT}/testing/testdata"
case "${TARGET}" in
  *-asan | *-ubsan)
    rm -f "${SANITIZER_LOG}"
    for test in "${UNIT_TESTS[@]}"; do
      "${test}" \
        --gtest_output="xml:${BUILD_DIR}/$(basename "${test}")_detail.xml" \
        3<&1 1>&2 2>&3 | tee -a "${SANITIZER_LOG}"
    done
    ;;
  *-valgrind)
    for test in "${UNIT_TESTS[@]}"; do
      run_valgrind --error-exitcode=1 "${test}" \
        --gtest_output="xml:${BUILD_DIR}/$(basename "${test}")_detail.xml"
    done
    ;;
  *)
    log_err "Unrecognized TARGET:${TARGET}."
    usage
    exit 1
    ;;
esac

if [[ -x "$(command -v asan_symbolize)" ]]; then
  ASAN_SYMBOLIZE_TOOL=asan_symbolize
else
  ASAN_SYMBOLIZE_TOOL=asan_symbolize.py
fi

case "${TARGET}" in
  *-asan)
    asanlog_symbolized="${BUILD_DIR}/asan_log.asanlog_symbolized"
    grep -v 'Invalid VP9' "${SANITIZER_LOG}" > "${SANITIZER_LOG}.2" || true
    "${ASAN_SYMBOLIZE_TOOL}" "${BUILD_DIR}" c++filt < "${SANITIZER_LOG}.2" > \
      "${asanlog_symbolized}"
    cat "${asanlog_symbolized}"
    ;;
  *) ;; # No other sanitizer options are required
esac
