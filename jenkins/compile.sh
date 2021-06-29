#!/bin/bash
# Copyright (c) 2010, Google Inc. All rights reserved.
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

PATH=$PATH:/opt/clang-7.0.1/bin
export PATH

LIBWEBM_ROOT=$(pwd)
WORKSPACE=${WORKSPACE:-"${LIBWEBM_ROOT}/.."}

[[ -f "${LIBWEBM_ROOT}/jenkins/compile.sh" ]] || \
  ( echo "${0} should be called from the root of the repo" >&2 && exit 1 )

[[ -d "${WORKSPACE}" ]] || \
  (echo "${WORKSPACE} directory not defined" >&2 && exit 1 )

#######################################
# Create build directory
# Globals:
#   BUILD_TYPE   static-debug | static
#   WORKSPACE    workspace where build is done. it is preferred to avoid build inside the repo
# Arguments:
#   None
# Outputs:
#   build dir path.
# Returns:
#   mkdir result
#######################################
function make_build_dir() {
  local build_dir_base
  build_dir_base="${WORKSPACE}/build-${BUILD_TYPE}"
  [[ -d "${build_dir_base}" ]] && rm -rf "${build_dir_base}"
  mkdir -p "${build_dir_base}"
  ls "${build_dir_base}"
}

#######################################
# Cleanup files from the backup directory.
# Trap function. Cleans up build output.
# Globals:
#   BUILD_DIR     build dir
#   LIBWEBM_ROOT  compilation workspace
# Arguments:
#   None
#######################################
function cleanup() {
  [[ -n "${BUILD_DIR}" ]] && \
    find "${BUILD_DIR}" \( -name "*.[ao]" -o -name "*.l[ao]" \) -exec rm -f {} +
  make -C "${LIBWEBM_ROOT}" -f Makefile.unix clean
  git clean -dfx 
}

#######################################
# Setup ccache for toolchain.
# Globals:
#   CC - C compiler toolchain 
#   CXX - C/C++ compiler toolchain
#######################################
function setup_ccache() {
  if [ -x "$(which ccache)" ]; then
    case "$CC" in
      # clang | 'clang -m32')
        clang*)
        export CCACHE_CPP2=yes
        # avoid the PATH trick for clang since there's a local install in /opt
        # with no symlink, TODO? the rest should be all right as they're
        # host-triplets with nothing similar installed.
        CC="ccache $CC"
        CXX="ccache $CXX"
        ;;
      *)
        # should pick up compilers installed via the debian package system
        export PATH=/usr/lib/ccache:$PATH
        ;;
    esac
  fi
}

#######################################
# Usage message.
# Outputs:
#   Usage information
#######################################
function usage() {
  echo ""
  echo "Usage: ${0##*/} BUILD_TYPE TARGET"
  echo "Options:"
  echo "BUILD_TYPE  supported build type: {static, static-debug}"
  echo "TARGET      supported target platform compilation"
  echo "Environment variables:"
  echo "WORKSPACE "
  echo "CC"
  echo "CXX"
  echo ""
}

BUILD_TYPE=${1:? "Build type not defined.$(usage)"}
TARGET=${2:? "Target not defined.$(usage)"}

trap cleanup EXIT
setup_ccache

case "${TARGET}" in
  x86_64-unknown-linux-gnu)
    make -C "${LIBWEBM_ROOT}" -f Makefile.unix
    ;;
  cmake*)
    [ -f CMakeLists.txt ] || exit 0
    case "${BUILD_TYPE}" in
      *debug) opts="${opts} -DCMAKE_BUILD_TYPE=Debug" ;;
      *) opts="${opts} -DCMAKE_BUILD_TYPE=Release" ;;
    esac

    case "${TARGET}" in
      *clang) opts="${opts} -DCMAKE_CXX_COMPILER=clang++" ;;
      *i686-w64-mingw32 | *x86_64-w64-mingw32)
        if [ -f build/mingw-w64_toolchain.cmake ]; then
          opts="${opts} -DCMAKE_TOOLCHAIN_FILE=/build/mingw-w64_toolchain.cmake"
          opts="${opts} -DMINGW_PREFIX=${TARGET#cmake-}"
          echo "This configuration currently fails to build"
          exit 0
        elif [ -f build/x86_64-mingw-gcc.cmake ]; then
          case "${TARGET}" in
            *i686*)
              opts="${opts} -DCMAKE_TOOLCHAIN_FILE=build/x86-mingw-gcc.cmake"
              ;;
            *x86_64*)
              opts="${opts} -DCMAKE_TOOLCHAIN_FILE=build/x86_64-mingw-gcc.cmake"
              ;;
          esac
          # opts="${opts} -DMINGW_PREFIX=${TARGET#cmake-}"
        else
          echo "This configuration currently fails to build"
          exit 0
        fi
        ;;
    esac
    BUILD_DIR=$(make_build_dir)
    pushd "${BUILD_DIR}" || exit 1
    cmake "${opts}" "${LIBWEBM_ROOT}" || exit 1
    make VERBOSE=1 || exit 1
    popd || exit 1
    ;;
  *)
    echo "${TARGET} TARGET not supported" >&2
    usage
    exit 1;;
esac
