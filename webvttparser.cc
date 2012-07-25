// Copyright (c) 2012 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "webvttparser.h"
#include <climits>

using std::string;

namespace libwebvtt {

Reader::Reader() {
}

Reader::~Reader() {
}

Parser::Parser(Reader* r)
  : reader_(r) {
}

int Parser::Init() {
  // Explanation of UTF-8 BOM:
  // http://en.wikipedia.org/wiki/Byte_order_mark

  char c;
  int e = reader_->GetChar(&c);

  if (e < 0)  // error
    return e;

  if (e > 0)  // EOF
    return -1;

  if (c == '\xEF') {  // UTF-8 BOM
    e = reader_->GetChar(&c);

    if (e < 0)  // error
      return e;

    if (e > 0)  // EOF
      return -1;

    if (c != '\xBB')
      return -1;

    e = reader_->GetChar(&c);

    if (e < 0)  // error
      return e;

    if (e > 0)  // EOF
      return -1;

    if (c != '\xBF')
      return -1;
  } else {
    e = reader_->PutChar(c);

    if (e)
      return e;
  }

  // Parse "WEBVTT"

  const char idstr[] = "WEBVTT";
  const char* p = idstr;

  while (*p) {
    e = reader_->GetChar(&c);

    if (e < 0)  // error
      return e;

    if (e > 0)  // EOF
      return -1;

    if (c != *p)
      return -1;

    ++p;
  }

  //  Parse optional characters that follow "WEBVTT"

  e = reader_->GetChar(&c);

  if (e < 0)  // error
    return e;

  if (e > 0)  // EOF
    return 0;  // success

  if (c == ' ' || c == '\x09') {  // SPACE or TAB
    for (;;) {
      e = reader_->GetChar(&c);

      if (e < 0)  // error
        return e;

      if (e > 0)  // EOF
        return 0;

      if (c == '\x0A' || c == '\x0D') {  // LF or CR
        e = reader_->PutChar(c);

        if (e)
          return e;

        break;
      }
    }
  }
  else if (c == '\x0A' || c == '\x0D') {  // LF or CR
    e = reader_->PutChar(c);

    if (e)
      return e;
  }
  else
    return -1;

  // Parse line terminator of first line

  e = ParseLineTerminator();

  if (e < 0)  // error
    return e;

  if (e > 0)  // EOF
    return 0;  // success

  // The WebVTT spec requires that the "WEBVTT" line
  // be followed by an empty line (to separate it from
  // first cue).

  e = ParseLineTerminator();

  if (e < 0)  // error
    return e;

  if (e > 0)  // EOF
    return 0;

  return 0;  // success
}

int Parser::ParseLineTerminator() {
  // The WebVTT spec states that lines may be
  // terminated in any of these three ways:
  //  LF
  //  CR
  //  CR LR

  char c;
  int e = reader_->GetChar(&c);

  if (e)
    return e;

  if (c == '\x0A')  // LF
    return 0;  // success

  if (c != '\x0D')  // not a CR
    return -1;  // failure

  e = reader_->GetChar(&c);

  if (e)
    return e;

  if (c == '\x0A')  // LF
    return 0;  // success

  e = reader_->PutChar(c);

  if (e)
    return e;

  return 0;  // success
}

int Parser::Parse(Cue* cue) {
  if (cue == NULL)
    return -1;

  // Parse first non-blank line

  string line;
  int e;

  for (;;) {
    e = ParseLine(&line);

    if (e)
      return e;

    if (!line.empty())
      break;
  }

  // A WebVTT cue comprises an optional cue identifier line followed
  // by a (non-optional) timings line.  You determine whether you have
  // a timings line by scanning for the arrow token, the lexeme of which
  // may not appear in the cue identifier line.

  string::size_type off = line.find("-->");

  if (off != string::npos)  // timings line
    cue->identifier.clear();
  else {
    cue->identifier.swap(line);

    e = ParseLine(&line);

    if (e)
      return e;

    off = line.find("-->");

    if (off == string::npos)  // not a timings line
      return -1;
  }

  e = ParseTimingsLine(line,
                       off,
                       &cue->start_time,
                       &cue->stop_time,
                       &cue->settings);

  if (e)
    return e;

  // The cue payload comprises all the non-empty
  // lines that follow the timings line.

  Cue::payload_t& p = cue->payload;
  p.clear();

  for (;;) {
    e = ParseLine(&line);

    if (e < 0)  // error
      return e;

    if (line.empty())
      break;

    p.push_back(line);
  }

  if (p.empty())
    return -1;

  return 0;  // success
}

int Parser::ParseLine(string* line) {
  line->clear();

  for (;;) {
    char c;
    int e = reader_->GetChar(&c);

    if (e < 0)  // error
      return e;

    if (e > 0)  // EOF
      return (line->empty()) ? 1 : 0;

    if (c != '\x0A' && c != '\x0D') {  // not (LF or CR)
      line->push_back(c);
      continue;
    }

    // Detected EOL

    e = reader_->PutChar(c);

    if (e)
      return e;

    e = ParseLineTerminator();

    if (e < 0)  // error
      return e;

    return 0;  // success
  }
}

int Parser::ParseTimingsLine(
  string& line,
  string::size_type arrow_pos,
  Time* start_time,
  Time* stop_time,
  Cue::settings_t* settings) {

  // Place a NUL character at the start of the arrow token, in
  // order to demarcate the start time from remainder of line.

  line[arrow_pos] = '\x00';
  string::size_type idx = 0;

  int e = ParseTime(line, idx, start_time);

  if (e)
    return e;

  // Detect any junk that follows the start time,
  // but precedes the arrow symbol.

  while (char c = line[idx]) {
    if (c != ' ' && c != '\x09')  // SPACE, TAB
      return -1;
    ++idx;
  }

  // Place a NUL character at the end of the line,
  // so the scanner has a place to stop, and begin
  // the scan just beyond the arrow token.

  line.push_back('\x00');
  idx = arrow_pos + 3;

  e = ParseTime(line, idx, stop_time);

  if (e)
    return e;

  e = ParseSettings(line, idx, settings);

  if (e)
    return e;

  return 0;  // success
}

int Parser::ParseTime(
  const string& line,
  string::size_type& idx,
  Time* time) {

  // WebVTT timestamp syntax comes in three flavors:
  //  SS[.sss]
  //  MM:SS[.sss]
  //  HH:MM:SS[.sss]

  Time& t = *time;

  // Consume any whitespace that precedes the timestamp.

  while (char c = line[idx]) {
    if (c != ' ' && c != '\x09')  // SPACE, TAB
      break;
    ++idx;
  }

  // Parse a generic number value.  We don't know which component
  // of the time we have yet, until we do more parsing.

  if (!isdigit(line[idx]))
    return -1;

  long long val = 0;

  while (isdigit(line[idx])) {
    val *= 10;
    val += int(line[idx] - '0');

    if (val > INT_MAX)
      return -1;

    ++idx;
  }

  // The presence of a colon character indicates that we have
  // an [HH:]MM:SS style syntax.

  if (line[idx] == ':') {
    // We have either HH:MM:SS or MM:SS

    // The value we just parsed is either the hours or minutes.
    // It must be followed by another number value (that is
    // either minutes or seconds).

    const int first_val = static_cast<int>(val);

    ++idx;  // consume colon

    // Parse second value

    if (!isdigit(line[idx]))
      return -1;

    val = 0;

    while (isdigit(line[idx])) {
      val *= 10;
      val += int(line[idx] - '0');

      if (val >= 60)
        return -1;

      ++idx;
    }

    if (line[idx] == ':') {
      // We have HH:MM:SS

      t.hours = first_val;
      t.minutes = static_cast<int>(val);

      ++idx;  // consume MM:SS colon

      // We have parsed the hours and minutes.
      // We must now parse the seconds.

      if (!isdigit(line[idx]))
        return -1;

      val = 0;

      while (isdigit(line[idx])) {
        val *= 10;
        val += int(line[idx] - '0');

        if (val >= 60)  // because we have HH:MM:SS
          return -1;

        ++idx;
      }

      t.seconds = static_cast<int>(val);

    } else {
      // We have MM:SS

      // The implication here is that the hour value was omitted
      // from the timestamp (because it was 0).

      if (first_val >= 60)  // minutes
        return -1;

      t.hours = 0;
      t.minutes = first_val;
      t.seconds = static_cast<int>(val);
    }

  } else {
    // We have SS (only)

    // The time is expressed as total number of seconds,
    // so the seconds value has no upper bound.

    t.seconds = static_cast<int>(val);

    // Convert SS to HH:MM:SS

    t.minutes = t.seconds / 60;
    t.seconds -= t.minutes * 60;

    t.hours = t.minutes / 60;
    t.minutes -= t.hours * 60;
  }

  // We have parsed the hours, minutes, and seconds.
  // We must now parse the milliseconds.

  if (line[idx] != '.')  // no milliseconds
    t.milliseconds = 0;
  else {
    ++idx;  // consume FULL STOP

    if (!isdigit(line[idx]))
      return -1;

    val = 0;

    while (isdigit(line[idx])) {
      val *= 10;
      val += int(line[idx] - '0');

      if (val >= 1000)
        return -1;

      ++idx;
    }

    if (val < 10)
      t.milliseconds = val * 100;
    else if (val < 100)
      t.milliseconds = val * 10;
    else
      t.milliseconds = val;
  }

  // We have parsed the time proper.  We must check for any
  // junk that immediately follows the time specifier.

  const char c = line[idx];

  if (c != '\x00' && c != ' ' && c != '\x09')  // NUL, SPACE, TAB
    return -1;

  return 0;  // success
}

int Parser::ParseSettings(
  const string& line,
  string::size_type idx,
  Cue::settings_t* settings) {

  // Scanning starts at position idx, and stops when
  // we consume a NUL character.

  settings->clear();

  for (;;) {
    // Parse the whitespace that precedes the NAME:VALUE pair.

    for (;;) {
      const char c = line[idx];

      if (c == '\x00')
        return 0;  // success

      if (c != ' ' && c != '\x09')  // SPACE, TAB
        break;

      ++idx;  // consume whitespace
    }

    // There is something on the line for us to scan.

    settings->push_back(Setting());
    Setting& s = settings->back();

    // Parse the NAME part of the settings pair.

    for (;;) {
      const char c = line[idx];

      if (c == ':')  // we have reached end of NAME part
        break;

      if (c == '\x00' || c == ' ' || c == '\x09')  // NUL, SPACE, TAB
        return -1;

      s.name.push_back(c);

      ++idx;
    }

    if (s.name.empty())
      return -1;

    ++idx;  // consume colon

    // Parse the VALUE part of the settings pair.

    for (;;) {
      const char c = line[idx];

      if (c == '\x00' || c == ' ' || c == '\x09')  // NUL, SPACE, TAB
        break;

      if (c == ':')  // suspicious when part of VALUE
        return -1;   // TODO(matthewjheaney): verify this behavior

      s.value.push_back(c);

      ++idx;
    }

    if (s.value.empty())
      return -1;
  }
}

bool Time::operator==(const Time& rhs) const {
  if (hours != rhs.hours)
    return false;

  if (minutes != rhs.minutes)
    return false;

  if (seconds != rhs.seconds)
    return false;

  return (milliseconds == rhs.milliseconds);
}

bool Time::operator<(const Time& rhs) const {
  if (hours < rhs.hours)
    return true;

  if (hours > rhs.hours)
    return false;

  if (minutes < rhs.minutes)
    return true;

  if (minutes > rhs.minutes)
    return false;

  if (seconds < rhs.seconds)
    return true;

  if (seconds > rhs.seconds)
    return false;

  return (milliseconds < rhs.milliseconds);
}

bool Time::operator>(const Time& rhs) const {
  return rhs.operator<(*this);
}

bool Time::operator<=(const Time& rhs) const {
  return !this->operator>(rhs);
}

bool Time::operator>=(const Time& rhs) const {
  return !this->operator<(rhs);
}

presentation_t Time::presentation() const {
  const presentation_t h = 1000LL * 3600LL * presentation_t(hours);
  const presentation_t m = 1000LL * 60LL * presentation_t(minutes);
  const presentation_t s = 1000LL * presentation_t(seconds);
  const presentation_t result = h + m + s + milliseconds;
  return result;
}

Time& Time::presentation(presentation_t d) {
  if (d < 0) {  // error
    hours = 0;
    minutes = 0;
    seconds = 0;
    milliseconds = 0;

    return *this;
  }

  seconds = d / 1000;
  milliseconds = d - 1000 * seconds;

  minutes = seconds / 60;
  seconds -= 60 * minutes;

  hours = minutes / 60;
  minutes -= 60 * hours;

  return *this;
}

Time& Time::operator+=(presentation_t rhs) {
  const presentation_t d = this->presentation();
  const presentation_t dd = d + rhs;
  this->presentation(dd);
  return *this;
}

Time Time::operator+(presentation_t d) const {
  Time t(*this);
  t += d;
  return t;
}

Time& Time::operator-=(presentation_t d) {
  return this->operator+=(-d);
}

presentation_t Time::operator-(const Time& t) const {
  const presentation_t rhs = t.presentation();
  const presentation_t lhs = this->presentation();
  const presentation_t result = lhs - rhs;
  return result;
}

}  // namespace libwebvtt
