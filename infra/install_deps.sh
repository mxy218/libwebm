#!/bin/bash

################################################################################
# Open Codecs Startup script.
#
# Repositories:
# https://chromium.googlesource.com/webm/libwebm
################################################################################

# Packages via apt
LATEST_PACKAGES=(
  "clang"
  "cmake"
  "g++-mingw-w64"
  "g++-mingw-w64-i686"
  "linux-libc-dev:i386"
  "unzip"
)

ANDROID_ZIP_URL="https://dl.google.com/android/repository/android-ndk-r21e-linux-x86_64.zip"

# This script is run as root.
set -x

########################################
# Download and unzip android-ndk in /opt/
# Environment variable:
# ANDROID_ZIP_URL downloads url
########################################
install_android_ndk() {
  local android_ndk_zip_url
  local android_ndk_zip_file
  local android_ndk_zip_dir


  android_ndk_zip_url="${ANDROID_ZIP_URL}"
  android_ndk_zip_file="${android_ndk_zip_url##*/}"
  android_ndk_zip_dir="/opt/android-ndk"

  [[ -d "${android_ndk_zip_dir}" ]] && sudo rm -rf "${android_ndk_zip_dir}"

  wget "${android_ndk_zip_url}"
  sudo unzip "${android_ndk_zip_file}" -d "/opt/"
  [[ -f "${android_ndk_zip_file}" ]] && rm -f "${android_ndk_zip_file}"
  sudo mv "${android_ndk_zip_dir}"* "${android_ndk_zip_dir}"
}

sudo apt update
sudo apt install -y "${LATEST_PACKAGES[@]}"

install_android_ndk
