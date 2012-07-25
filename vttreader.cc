#include "vttreader.h"

VttReader::VttReader()
  : file_(NULL) {
}

VttReader::~VttReader() {
  Close();
}

int VttReader::Open(const char* filename) {
  if (filename == NULL)
    return -1;

  if (file_)
    return -1;

  file_ = fopen(filename, "rb");

  if (file_ == NULL)
    return -1;

  buf_ = EOF;

  return 0;  // success
}

void VttReader::Close() {
  if (file_) {
    fclose(file_);
    file_ = NULL;
  }
}

int VttReader::GetChar(char* c) {
  if (c == NULL)
    return -1;

  if (file_ == NULL)
    return -1;

  if (buf_ != EOF) {
    *c = static_cast<char>(buf_);
    buf_ = EOF;
    return 0;  // success
  }

  const int result = fgetc(file_);

  if (result != EOF) {
    *c = static_cast<char>(result);
    return 0;  // success
  }

  if (ferror(file_))
    return -1;  // error

  if (feof(file_))
    return 1;  // EOF

  return -1;  // weird
}

int VttReader::PutChar(char c) {
  if (file_ == NULL)
    return -1;

  if (buf_ != EOF)
    return -1;

  buf_ = c;
  return 0;  // success
}
