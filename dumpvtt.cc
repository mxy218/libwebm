#include <cstdio>
#include "vttreader.h"
#include "webvttparser.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("wrong number of arguments\n");
    return 0;
  }

  VttReader r;

  const char* const filename = argv[1];
  int e = r.Open(filename);

  if (e) {
    printf("open failed\n");
    return 1;
  }

  libwebvtt::Parser p(&r);

  e = p.Init();

  if (e) {
    printf("parser init failed\n");
    return 1;
  }

  libwebvtt::Cue c;

  for (;;) {
    e = p.Parse(&c);

    if (e < 0) {  // error
      printf("error parsing cue\n");
      return 1;
    }

    if (e > 0)  // EOF
      return 0;

    printf("cue identifier: \"%s\"\n", c.identifier.c_str());

    const libwebvtt::Time& st = c.start_time;
    printf("cue start time: \"HH=%i MM=%i SS=%i SSS=%i\"\n",
           st.hours,
           st.minutes,
           st.seconds,
           st.milliseconds);

    const libwebvtt::Time& sp = c.stop_time;
    printf("cue stop time: \"HH=%i MM=%i SS=%i SSS=%i\"\n",
           sp.hours,
           sp.minutes,
           sp.seconds,
           sp.milliseconds);

    bool b = c.start_time == c.stop_time;
    b = c.start_time < c.stop_time;
    b = c.start_time > c.stop_time;
    b = c.start_time <= c.stop_time;
    b = c.start_time >= c.stop_time;

    {
      typedef libwebvtt::Cue::settings_t::const_iterator iter_t;
      iter_t i = c.settings.begin();
      const iter_t j = c.settings.end();

      if (i == j)
        printf("cue setting: <no settings present>\n");
      else
        while (i != j) {
          const libwebvtt::Setting& s = *i++;
          printf("cue setting: name=%s value=%s\n",
                 s.name.c_str(),
                 s.value.c_str());
        }
    }

    {
      typedef libwebvtt::Cue::payload_t::const_iterator iter_t;
      iter_t i = c.payload.begin();
      const iter_t j = c.payload.end();

      int idx = 1;
      while (i != j) {
        const std::string& p = *i++;
        printf("cue payload[%i]: \"%s\"\n", idx, p.c_str());
        ++idx;
      }
    }

    printf("\n");
  }
}
