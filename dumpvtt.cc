// Copyright (c) 2012 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include <cstdio>
#include "vttreader.h"  // NOLINT
#include "webvttparser.h"  // NOLINT

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("wrong number of arguments\n");
    return 0;
  }

  VttReader reader;

  const char* const filename = argv[1];
  int e = reader.Open(filename);

  if (e) {
    printf("open failed\n");
    return 1;
  }

  libwebvtt::Parser parser(&reader);

  e = parser.Init();

  if (e) {
    printf("parser init failed\n");
    return 1;
  }

  libwebvtt::Cue cue;

  for (;;) {
    e = parser.Parse(&cue);

    if (e < 0) {  // error
      printf("error parsing cue\n");
      return 1;
    }

    if (e > 0)  // EOF
      return 0;

    printf("cue identifier: \"%s\"\n", cue.identifier.c_str());

    const libwebvtt::Time& st = cue.start_time;
    printf("cue start time: \"HH=%i MM=%i SS=%i SSS=%i\"\n",
           st.hours,
           st.minutes,
           st.seconds,
           st.milliseconds);

    const libwebvtt::Time& sp = cue.stop_time;
    printf("cue stop time: \"HH=%i MM=%i SS=%i SSS=%i\"\n",
           sp.hours,
           sp.minutes,
           sp.seconds,
           sp.milliseconds);

    {
      typedef libwebvtt::Cue::settings_t::const_iterator iter_t;
      iter_t i = cue.settings.begin();
      const iter_t j = cue.settings.end();

      if (i == j)
        printf("cue setting: <no settings present>\n");
      else
        while (i != j) {
          const libwebvtt::Setting& setting = *i++;
          printf("cue setting: name=%s value=%s\n",
                 setting.name.c_str(),
                 setting.value.c_str());
        }
    }

    {
      typedef libwebvtt::Cue::payload_t::const_iterator iter_t;
      iter_t i = cue.payload.begin();
      const iter_t j = cue.payload.end();

      int idx = 1;
      while (i != j) {
        const std::string& payload = *i++;
        const char* const payload_str = payload.c_str();
        printf("cue payload[%i]: \"%s\"\n", idx, payload_str);
        ++idx;
      }
    }

    printf("\n");
  }
}
