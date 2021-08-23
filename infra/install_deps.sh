#!/bin/bash

## This is a throw-away script to check some things in the instance
set -x

# Check installed packages in vm
sudo dpkg-query -l

LATEST_PACKAGES=(
  # Global Tools
  "clang"
  "cmake"
  "g++-mingw-w64"
  "g++-mingw-w64-i686"
  "libc6-dbg:i386"
  "linux-libc-dev:i386"
  "unzip"
  "valgrind"
  # webp2 deps
  "freeglut3-dev"
  "libpng-dev"
  "libjpeg-dev"
  "libtiff-dev"
  "libgif-dev"
  "libwebp-dev"
  "libpthread-stubs0-dev"
  "libsdl-dev"
  "libgtest-dev"
  "python3-dev"
  "swig"
)

sudo apt update
sudo apt-cache search "${LATEST_PACKAGES[@]}"
