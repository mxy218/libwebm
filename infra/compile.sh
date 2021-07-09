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

usage() {
  cat<< EOF
Usage: compile.sh BUILD_TYPE TARGET
Options:
BUILD_TYPE  supported build type (static, static-debug)
TARGET      supported target platform compilation: (native, native-clang,
            i686-w64-mingw32, x86_64-w64-mingw32, native-Makefile.unix)
Environment variables:
WORKSPACE   directory where the build is done
CC          C compiler
CXX         C/C++ compiler
EOF
}

log_err() {
  echo "[$(date +'%Y-%m-%dT%H:%M:%S%z')]: $*" >&2
}

#######################################
# Create build directory
# Globals:
#   BUILD_TYPE   static-debug | static
#   WORKSPACE    directory where build is done
# Arguments:
#   None
# Outputs:
#   build dir path.
# Returns:
#   mkdir result
#######################################
make_build_dir() {
  local build_dir_base
  build_dir_base="${WORKSPACE}/build-${BUILD_TYPE}"
  [[ -d "${build_dir_base}" ]] && rm -rf "${build_dir_base}"
  mkdir -p "${build_dir_base}"
  echo "${build_dir_base}"
}

#######################################
# Cleanup files from the backup directory.
# Globals:
#   BUILD_DIR     build directory
#   LIBWEBM_ROOT  repository's root path
#######################################
cleanup() {
  # BUILD_DIR is not completely removed to allow for binary artifacts to be
  # extracted.
  [[ -n "${BUILD_DIR}" ]] \
    && find "${BUILD_DIR}" \( -name "*.[ao]" -o -name "*.l[ao]" \) -exec rm \
    -f {} +
  make -C "${LIBWEBM_ROOT}" -f Makefile.unix clean
}

#######################################
# Setup ccache for toolchain.
# Globals:
#   CC - C compiler toolchain
#   CXX - C/C++ compiler toolchain
#######################################
setup_ccache() {
  if [[ -x "$(command -v ccache)" ]]; then
    case "${CC}" in
      clang*)
        export CCACHE_CPP2=yes
        CC="ccache ${CC}"
        CXX="ccache ${CXX}"
        ;;
      *)
        # should pick up compilers installed via package system (e.g. debian,
        # apt).
        export PATH="/usr/lib/ccache:${PATH}"
        ;;
    esac
  fi
}

################################################################################
set -e
LIBWEBM_ROOT="$(realpath "$(dirname "$0")/..")"
WORKSPACE=${WORKSPACE:-"$(mktemp -d)"}

echo "Building libwebm in ${WORKSPACE}"

if [[ ! -d "${WORKSPACE}" ]]; then
  log_err "${WORKSPACE} directory does not exist"
  exit 1
fi

BUILD_TYPE=${1:?"Build type not defined.$(echo; usage)"}
TARGET=${2:?"Target not defined.$(echo; usage)"}

trap cleanup EXIT
setup_ccache

case "${TARGET}" in
  native-Makefile.unix)
    make -C "${LIBWEBM_ROOT}" -f Makefile.unix
    ;;
  *)
    opts=()
    case "${BUILD_TYPE}" in
      static) opts+=("-DCMAKE_BUILD_TYPE=Release") ;;
      *debug) opts+=("-DCMAKE_BUILD_TYPE=Debug") ;;
      *)
        log_err "${BUILD_TYPE} build type not supported"
        usage
        exit 1
        ;;
    esac

    TOOLCHAIN_FILE_FLAG="-DCMAKE_TOOLCHAIN_FILE=${LIBWEBM_ROOT}/build"
    case "${TARGET}" in
      clang) opts+=("-DCMAKE_CXX_COMPILER=clang++") ;;
      native) ;; # No additional flags needed.
      i686-w64-mingw32)
        opts+=("${TOOLCHAIN_FILE_FLAG}/x86-mingw-gcc.cmake")
        ;;
      x86_64-w64-mingw32)
        opts+=("${TOOLCHAIN_FILE_FLAG}/x86_64-mingw-gcc.cmake")
        ;;
      *)
        log_err "${TARGET} TARGET not supported"
        usage
        exit 1
        ;;
    esac
    BUILD_DIR="$(make_build_dir)"
    pushd "${BUILD_DIR}"
    cmake "${opts[@]}" "${LIBWEBM_ROOT}"
    make VERBOSE=1
    popd
    ;;
esac
