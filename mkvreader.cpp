// Copyright (c) 2010 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "mkvreader.hpp"

#include <cassert>

namespace mkvparser
{

MkvReader::MkvReader() :
    m_file(NULL)
{
}

MkvReader::~MkvReader()
{
    Close();
}

int MkvReader::Open(const char* fileName)
{
    if (fileName == NULL)
        return -1;

    if (m_file)
        return -1;

#ifdef WIN32
    const errno_t e = fopen_s(&m_file, fileName, "rb");

    if (e)
        return -1;  //error
#else
    m_file = fopen(fileName, "rb");

    if (m_file == NULL)
        return -1;
#endif

#ifdef WIN32
    int status = _fseeki64(m_file, 0L, SEEK_END);

    if (status)
        return -1;  //error

    m_length = _ftelli64(m_file);
#else
    fseek(m_file, 0L, SEEK_END);
    m_length = ftell(m_file);
#endif
    assert(m_length >= 0);

#ifdef WIN32
    status = _fseeki64(m_file, 0L, SEEK_SET);

    if (status)
        return -1;  //error
#else
    fseek(m_file, 0L, SEEK_SET);
#endif

    return 0;
}

void MkvReader::Close()
{
    if (m_file != NULL)
    {
        fclose(m_file);
        m_file = NULL;
    }
}

int MkvReader::Length(long long* total, long long* available)
{
    if (m_file == NULL)
        return -1;

    if (total)
        *total = m_length;

    if (available)
        *available = m_length;

    return 0;
}

int MkvReader::Read(long long offset, long len, unsigned char* buffer)
{
    if (m_file == NULL)
        return -1;

    if (offset < 0)
        return -1;

    if (len < 0)
        return -1;

    if (len == 0)
        return 0;

    if (offset >= m_length)
        return -1;

#ifdef WIN32
    const int status = _fseeki64(m_file, offset, SEEK_SET);

    if (status)
        return -1;  //error
#else
    fseek(m_file, offset, SEEK_SET);
#endif

    const size_t size = fread(buffer, 1, len, m_file);

    if (size < size_t(len))
        return -1;  //error

    return 0;  //success
}

bool MkvReader::IsOpen() const
{
  return m_file;
}

}  //end namespace mkvparser
