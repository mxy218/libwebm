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
BUILD_TYPE  supported build type: {static, static-debug}
TARGET      supported target platform compilation
Environment variables:
WORKSPACE   directory where the build is done.
CC          C compiler toolchain
CXX         C/C++ compiler toolchain
EOF
}

log_err() {
  echo "[$(date +'%Y-%m-%dT%H:%M:%S%z')]: $*" >&2
}

#######################################
# Create build directory
# Globals:
#   BUILD_TYPE   static-debug | static
#   WORKSPACE    directory where build is done. it is preferred to avoid build
#                inside the repo
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
# Trap . Cleans up build output.
# Globals:
#   BUILD_DIR     build directory
#   LIBWEBM_ROOT  repository's root path
#######################################
cleanup() {
  [[ -n "${BUILD_DIR}" ]] && \
    find "${BUILD_DIR}" \( -name "*.[ao]" -o -name "*.l[ao]" \) -exec rm -f {} +
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
        # should pick up compilers installed via package system (e.g. debian).
        export PATH="/usr/lib/ccache:${PATH}"
        ;;
    esac
  fi
}

################################################################################
LIBWEBM_ROOT="$(realpath "$(dirname "$0")/..")"
WORKSPACE=${WORKSPACE:-"$(mktemp -d)"}

echo "Building libwebm in ${WORKSPACE}"

if [[ ! -d "${WORKSPACE}" ]]; then
  log_err "${WORKSPACE} directory does not exist"
  exit 1
fi

BUILD_TYPE=${1:?"Build type not defined.$(usage)"}
TARGET=${2:?"Target not defined.$(usage)"}

trap cleanup EXIT
setup_ccache

case "${TARGET}" in
  x86_64-unknown-linux-gnu)
    make -C "${LIBWEBM_ROOT}" -f Makefile.unix
    ;;
  cmake*)
    [[ -f "${LIBWEBM_ROOT}/CMakeLists.txt" ]] || exit 0
    case "${BUILD_TYPE}" in
      *debug) opts="${opts} -DCMAKE_BUILD_TYPE=Debug" ;;
      *) opts="${opts} -DCMAKE_BUILD_TYPE=Release" ;;
    esac

    case "${TARGET}" in
      *clang) opts="${opts} -DCMAKE_CXX_COMPILER=clang++" ;;
      *i686-w64-mingw32 | *x86_64-w64-mingw32)
        if [[ -f build/mingw-w64_toolchain.cmake ]]; then
          opts="${opts} -DCMAKE_TOOLCHAIN_FILE=/build/mingw-w64_toolchain.cmake"
          opts="${opts} -DMINGW_PREFIX=${TARGET#cmake-}"
          log_err "This configuration currently fails to build"
          exit 0
        elif [[ -f build/x86_64-mingw-gcc.cmake ]]; then
          case "${TARGET}" in
            *i686*)
              opts+="-DCMAKE_TOOLCHAIN_FILE=build/x86-mingw-gcc.cmake"
              ;;
            *x86_64*)
              opts+="-DCMAKE_TOOLCHAIN_FILE=build/x86_64-mingw-gcc.cmake"
              ;;
            *);;
          esac
        else
          log_err "This configuration currently fails to build"
          exit 0
        fi
        ;;
      *);;
    esac
    BUILD_DIR="$(make_build_dir)"
    set -e
    pushd "${BUILD_DIR}"
    cmake "${opts}" "${LIBWEBM_ROOT}"
    make VERBOSE=1
    popd
    set +e
    ;;
  *)
    log_err "${TARGET} TARGET not supported"
    usage
    exit 1
    ;;
esac
