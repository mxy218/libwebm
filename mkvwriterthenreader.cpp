// Copyright (c) 2013 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "mkvwriterthenreader.hpp"

#ifdef _MSC_VER
#include <share.h>  // for _SH_DENYWR
#endif

#include <cstdio>
#include <new>

namespace mkvmuxer {

MkvWriterThenReader::MkvWriterThenReader() : file_(NULL),
                                             writes_allowed_(true) {
}

MkvWriterThenReader::~MkvWriterThenReader() {
  Close();
}

int32 MkvWriterThenReader::Write(const void* buffer, uint32 length) {
  if (file_ == NULL || !writes_allowed_)
    return -1;

  if (length == 0)
    return 0;

  if (buffer == NULL)
    return -1;

  const size_t bytes_written = fwrite(buffer, 1, length, file_);

  return (bytes_written == length) ? 0 : -1;
}

bool MkvWriterThenReader::Open(const char* filename, bool create_temp_file) {
  if (filename == NULL && !create_temp_file)
    return false;

  if (file_ != NULL)
    return false;

#ifdef _MSC_VER
  file_ = create_temp_file ? tmpfile() : _fsopen(filename, "wb+", _SH_DENYWR);
#else
  file_ = create_temp_file ? tmpfile() : fopen(filename, "wb+");
#endif
  if (file_ == NULL)
    return false;
  return true;
}

void MkvWriterThenReader::Close() {
  if (file_ != NULL) {
    fclose(file_);
    file_ = NULL;
  }
}

int64 MkvWriterThenReader::Position() const {
  if (file_ == NULL)
    return 0;

#ifdef _MSC_VER
    return _ftelli64(file_);
#else
    return ftell(file_);
#endif
}

int32 MkvWriterThenReader::Position(int64 position) {
  if (file_ == NULL)
    return -1;

#ifdef _MSC_VER
    return _fseeki64(file_, position, SEEK_SET);
#else
    return fseek(file_, position, SEEK_SET);
#endif
}

bool MkvWriterThenReader::Seekable() const {
  return true;
}

void MkvWriterThenReader::ElementStartNotify(uint64, int64) {
}

int MkvWriterThenReader::Length(int64* total, int64* available) {
    return 0;
}

int MkvWriterThenReader::Read(int64 offset, int32 len, uint8* buffer) {
  if (file_ == NULL)
    return -1;

  if (offset < 0)
    return -1;

  if (len < 0)
    return -1;

  if (writes_allowed_)
    writes_allowed_ = false;

  if (len == 0)
    return 0;

  fflush(file_);

#ifdef _MSC_VER
  int64 pos = _ftelli64(file_);
#else
  int64 pos = ftell(file_);
#endif

#ifdef WIN32
  int status = _fseeki64(file_, offset, SEEK_SET);

  if (status)
    return -1;  //error
#else
  fseek(file_, offset, SEEK_SET);
#endif

  const size_t size = fread(buffer, 1, len, file_);

#ifdef WIN32
  status = _fseeki64(file_, pos, SEEK_SET);

  if (status)
    return -1;  //error
#else
  fseek(file_, pos, SEEK_SET);
#endif

  if (size < static_cast<size_t>(len))
    return -1;  //error

  return 0;  //success
}

}  // namespace mkvmuxer
