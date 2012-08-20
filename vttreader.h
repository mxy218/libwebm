#ifndef VTTREADER_H_  // NOLINT
#define VTTREADER_H_

#include <cstdio>
#include "webvttparser.h"  // NOLINT

class VttReader : public libwebvtt::Reader {
 public:
  VttReader();
  virtual ~VttReader();

  int Open(const char* filename);
  void Close();

  virtual int GetChar(char* c);

 private:
  FILE* file_;

 private:
  VttReader(const VttReader&);
  VttReader& operator=(const VttReader&);
};

#endif  // VTTREADER_H_  // NOLINT
