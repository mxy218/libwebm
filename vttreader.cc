#include "vttreader.h"  // NOLINT

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
