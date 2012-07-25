// Copyright (c) 2012 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef WEBVTTPARSER_H_
#define WEBVTTPARSER_H_

#include <string>
#include <list>

namespace libwebvtt {

class Reader {
 public:
  virtual int GetChar(char*) = 0;
  virtual int PutChar(char) = 0;
 protected:
  Reader();
  virtual ~Reader();
};

// As measured in thousandths of a second,
// e.g. a duration of 1 equals 0.001 seconds,
// and a duration of 1000 equals 1 seconds.
typedef long long presentation_t;

struct Time {
  int hours;
  int minutes;
  int seconds;
  int thousandths;

  bool operator==(const Time& rhs) const;
  bool operator<(const Time& rhs) const;
  bool operator>(const Time& rhs) const;
  bool operator<=(const Time& rhs) const;
  bool operator>=(const Time& rhs) const;

  presentation_t presentation() const;
  Time& presentation(presentation_t);

  Time& operator+=(presentation_t);
  Time operator+(presentation_t) const;

  Time& operator-=(presentation_t);
  presentation_t operator-(const Time&) const;
};

struct Setting {
  std::string name;
  std::string value;
};

struct Cue {
  std::string identifier;

  Time start_time;
  Time stop_time;

  typedef std::list<Setting> settings_t;
  settings_t settings;

  typedef std::list<std::string> payload_t;
  payload_t payload;
};

class Parser {
 public:
  explicit Parser(Reader*);
  Reader* const reader_;
  int Init();
  int Parse(Cue*);

 private:
  int ParseLineTerminator();
  int ParseLine(std::string*);
  int ParseTimingsLine(std::string&, std::string::size_type, Cue*);
  int ParseTime(const std::string&, std::string::size_type&, Time&);
  int ParseSettings(const std::string&, std::string::size_type, Cue*);

 private:
  Parser(const Parser&);
  Parser& operator=(const Parser&);
};

}  // namespace libwebvtt

#endif  // WEBVTTPARSER_H_
