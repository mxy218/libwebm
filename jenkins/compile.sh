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
WORKSPACE=$(pwd)

[[ -f "${WORKSPACE}/jenkins/compile.sh" ]] || \
  ( echo "${0} should be called from the root of the repo" && exit 1 )

#######################################
# Create build directory
# Globals:
#   config - static-debug | static
#   WORKSPACE - repo root
# Arguments:
#   None
# Outputs:
#   build dir path.
# Returns:    
#   mkdir result
#######################################
function make_build_dir() {
  local build_dir_base
  build_dir_base="build-${config}"

  [[ -d "${build_dir_base}" ]] && rm -rf "${build_dir_base}"
  echo "${WORKSPACE}/${build_dir_base}"
  mkdir -p "${build_dir_base}"
}

#######################################
# Cleanup files from the backup directory.
# Trap function. Cleans up build output.
# Globals:
#   BUILD_DIR - build dir
#   WORKSPACE - compilation workspace
# Arguments:
#   None
#######################################
function cleanup() {
  cd "${WORKSPACE}" || return 1
  find "${BUILD_DIR}" \( -name "*.[ao]" -o -name "*.l[ao]" \) -exec rm -f {} +
  make -f Makefile.unix clean
}

#######################################
# Setup ccache for compiler.
# Globals:
#   CC
#   CXX
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

config=${1:?missing config: static-debug or static}
host=${2:?missing host: e.g. x86_64}
BUILD_DIR=$(make_build_dir)

trap cleanup EXIT
setup_ccache

echo "building on ${BUILD_DIR}"
case "${host}" in
  x86_64-unknown-linux-gnu)
    make -f Makefile.unix
    ;;
  cmake*)
    [ -f CMakeLists.txt ] || exit 0
    case "$config" in
      *debug) opts="${opts} -DCMAKE_BUILD_TYPE=Debug" ;;
      *) opts="${opts} -DCMAKE_BUILD_TYPE=Release" ;;
    esac

    case "${host}" in
      *clang) opts="${opts} -DCMAKE_CXX_COMPILER=clang++" ;;
      *i686-w64-mingw32 | *x86_64-w64-mingw32)
        if [ -f build/mingw-w64_toolchain.cmake ]; then
          opts="${opts} -DCMAKE_TOOLCHAIN_FILE=/build/mingw-w64_toolchain.cmake"
          opts="${opts} -DMINGW_PREFIX=${host#cmake-}"
          echo "This configuration currently fails to build"
          exit 0
        elif [ -f build/x86_64-mingw-gcc.cmake ]; then
          case "${host}" in
            *i686*)
              opts="${opts} -DCMAKE_TOOLCHAIN_FILE=build/x86-mingw-gcc.cmake"
              ;;
            *x86_64*)
              opts="${opts} -DCMAKE_TOOLCHAIN_FILE=build/x86_64-mingw-gcc.cmake"
              ;;
          esac
          # opts="${opts} -DMINGW_PREFIX=${host#cmake-}"
        else
          echo "This configuration currently fails to build"
          exit 0
        fi
        ;;
    esac
    pushd "${BUILD_DIR}" || exit 1
    echo "cmake ${opts} .."
    cmake "${opts}" .. || exit 1
    popd || exit 1
    [[ -f "Makefile" ]] || (echo "Makefile was not found on dir $(dirs +0)"  && exit 1)
    make VERBOSE=1 || exit 1
    ;;
  *) 
    echo "${host} host not supported" && exit 1;;
esac
