// Copyright (c) 2012 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef WEBVTTPARSER_H_  // NOLINT
#define WEBVTTPARSER_H_

#include <string>
#include <list>

namespace libwebvtt {

class Reader {
 public:
  // Fetch a character from the stream. Return
  // negative if error, positive if end-of-stream,
  // and 0 if a character is available.
  //
  virtual int GetChar(char* c) = 0;

  // Unfetch a character; only a one-character put-back
  // buffer is required. Return non-zero if error.
  //
  virtual int PutChar(char c) = 0;

 protected:
  Reader();
  virtual ~Reader();
};

// As measured in thousandths of a second,
// e.g. a duration of 1 equals 0.001 seconds,
// and a duration of 1000 equals 1 second.
typedef long long presentation_t;  // NOLINT

struct Time {
  int hours;
  int minutes;
  int seconds;
  int milliseconds;

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
  explicit Parser(Reader* r);
  Reader* const reader_;

  // Pre-parse enough of the stream to determine whether
  // this is really a WEBVTT file. Returns 0 on success.
  int Init();

  // Parse the next WebVTT cue from the stream. Returns 0 if
  // an entire cue was parsed, negative if error, and positive
  // at end-of-stream.
  int Parse(Cue* cue);

 private:
  // Consume the characters that indicate end-of-line.
  int ParseLineTerminator();

  // Consume a line of text from the stream.
  int ParseLine(std::string* line);

  // Parse the distinguished "cue timings" line, which includes
  // the start and stop times and settings.  The line argument
  // contains the complete line of text, which the function is
  // free to modify as it sees fit, to facilitate scanning.  The
  // arrow_pos argument is the offset of the arrow token ("-->"),
  // which indicates that this is the timings line.
  //
  static int ParseTimingsLine(std::string& line,  // NOLINT
                              std::string::size_type arrow_pos,
                              Time* start_time,
                              Time* stop_time,
                              Cue::settings_t* settings);

  // Parse a single time specifier (from the timings line), starting
  // at the given offset; lexical scanning stops when a NUL character
  // is detected. The function modifies the offset by the amount of
  // characters consumed.
  //
  static int ParseTime(const std::string& line,
                       std::string::size_type& off,
                       Time* time);

  // Parse the cue settings from the timings line, starting at the
  // given offset.
  //
  static int ParseSettings(const std::string& line,
                           std::string::size_type off,
                           Cue::settings_t* settings);

 private:
  Parser(const Parser&);
  Parser& operator=(const Parser&);
};

}  // namespace libwebvtt

#endif  // WEBVTTPARSER_H_  // NOLINT
