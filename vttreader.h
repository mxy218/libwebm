#ifndef VTTREADER_H_
#define VTTREADER_H_

#include "webvttparser.h"
#include <cstdio>

class VttReader : public libwebvtt::Reader {
 public:
  VttReader();
  virtual ~VttReader();

  int Open(const char* filename);
  void Close();

  virtual int GetChar(char*);
  virtual int PutChar(char);

 private:
  FILE* file_;
  int buf_;

 private:
  VttReader(const VttReader&);
  VttReader& operator=(const VttReader&);

};

#endif
