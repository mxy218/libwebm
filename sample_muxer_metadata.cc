#include <string>

#include "sample_muxer_metadata.h"
#include "vttreader.h"

using std::string;

SampleMuxerMetadata::SampleMuxerMetadata() : segment_(NULL) {
}

void SampleMuxerMetadata::Init(mkvmuxer::Segment* s) {
  segment_ = s;
}

bool SampleMuxerMetadata::Load(const char* file, Kind kind) {
  mkvmuxer::uint64 track_num;

  if (!AddTrack(kind, &track_num))
    return false;

  typedef cues_map_t::iterator iter_t;
  typedef std::pair<iter_t, bool> result_t;

  const cues_map_t::value_type x(track_num, cues_t());
  const result_t result = cues_map_.insert(x);

  if (!result.second)  // weird
    return false;

  if (!Parse(file, kind, result.first))
    return false;

  return true;
}

bool SampleMuxerMetadata::Write(mkvmuxer::int64 time_ns) {
  typedef merge_cues_t::iterator iter_t;

  iter_t i = merge_cues_.begin();
  const iter_t j = merge_cues_.end();

  while (i != j) {
    const merge_cues_t::value_type& v = *i;

    if (time_ns >= 0 && v > time_ns)
      return true;  // nothing else to do just yet

    if (!v.Write(segment_)) {
      printf("\nCould not add metadata.\n");
      return false;  // error
    }

    merge_cues_.erase(i++);
  }

  return true;
}

bool SampleMuxerMetadata::AddTrack(
    Kind kind,
    mkvmuxer::uint64* track_num) {
  *track_num = 0;

  // Track number value 0 means "let muxer choose track number"
  mkvmuxer::Track* const track = segment_->AddTrack(0);

  if (track == NULL)  // error
    return false;

  // Return the track number value chosen by the muxer
  *track_num = track->number();

  const int types[] = {
    0x11,   // subtitles
    0x11,   // captions
    0x21,   // descriptions
    0x21    // metadata
  };

  const char* const codec_ids[] = {
    "D_WEBVTT/SUBTITLES",
    "D_WEBVTT/CAPTIONS",
    "D_WEBVTT/DESCRIPTIONS",
    "D_WEBVTT/METADATA"
  };

  track->set_type(types[kind]);
  track->set_codec_id(codec_ids[kind]);

  //TODO(matthewjheaney): set name and language

  return true;
}

bool SampleMuxerMetadata::Parse(
    const char* file,
    Kind,
    cues_map_t::iterator cues_map) {
  libwebvtt::VttReader r;
  int e = r.Open(file);

  if (e) {
    printf("Unable to open WebVTT file: \"%s\"\n", file);
    return false;
  }

  libwebvtt::Parser p(&r);

  e = p.Init();

  if (e < 0) {  // error
    printf("Error parsing WebVTT file: \"%s\"\n", file);
    return false;
  }

  cues_t& cues = cues_map->second;

  MergeCue merge_cue;
  merge_cue.cues_map = cues_map;

  libwebvtt::Time t;
  t.hours = -1;

  for (;;) {
    typedef cues_t::iterator iter_t;

    const iter_t cue_iter = cues.insert(cues.end(), cue_t());
    cue_t& c = *cue_iter;

    e = p.Parse(&c);

    if (e < 0) {  // error
      printf("Error parsing WebVTT file: \"%s\"\n", file);
      return false;
    }

    if (e > 0) {  // EOF
      cues.erase(cue_iter);
      return true;
    }

    if (c.start_time >= t)
      t = c.start_time;
    else {
      printf("bad WebVTT cue timestamp (out-of-order)\n");
      return false;
    }

    if (c.stop_time < c.start_time) {
      printf("bad WebVTT cue timestamp (stop < start)\n");
      return false;
    }

    merge_cue.cue = cue_iter;
    merge_cues_.insert(merge_cue);
  }
}

void SampleMuxerMetadata::MakeFrame(const cue_t& c, std::string* pf) {
  pf->clear();
  WriteCueIdentifier(c.identifier, pf);
  WriteCueSettings(c.settings, pf);
  WriteCuePayload(c.payload, pf);
}

void SampleMuxerMetadata::WriteCueIdentifier(
    const std::string& identifier,
    std::string* pf) {
  pf->append(identifier);
  pf->push_back('\x0A');  // LF
}

void SampleMuxerMetadata::WriteCueSettings(
    const cue_t::settings_t& settings,
    std::string* pf) {
  if (settings.empty()) {
    pf->push_back('\x0A');  // LF
    return;
  }

  typedef cue_t::settings_t::const_iterator iter_t;

  iter_t i = settings.begin();
  const iter_t j = settings.end();

  for (;;) {
    const libwebvtt::Setting& setting = *i++;

    pf->append(setting.name);
    pf->push_back(':');
    pf->append(setting.value);

    if (i == j)
      break;

    pf->push_back(' ');  // separate settings with whitespace
  }

  pf->push_back('\x0A');  // LF
}

void SampleMuxerMetadata::WriteCuePayload(
    const cue_t::payload_t& payload,
    std::string* pf) {
  typedef cue_t::payload_t::const_iterator iter_t;

  iter_t i = payload.begin();
  const iter_t j = payload.end();

  while (i != j) {
    const string& line = *i++;
    pf->append(line);
    pf->push_back('\x0A');  // LF
  }
}

bool SampleMuxerMetadata::MergeCue::Write(mkvmuxer::Segment* segment) const {
  // Cue start time expressed in milliseconds
  const mkvmuxer::int64 start_ms = cue->start_time.presentation();

  // Cue start time expressed in nanoseconds (MKV time)
  const mkvmuxer::int64 start_ns = start_ms * 1000000;

  // Cue stop time expressed in milliseconds
  const mkvmuxer::int64 stop_ms = cue->stop_time.presentation();

  // Cue stop time expressed in nanonseconds
  const mkvmuxer::int64 stop_ns = stop_ms * 1000000;

  // Metadata blocks always specify the block duration.
  const mkvmuxer::int64 duration_ns = stop_ns - start_ns;

  string frame;
  MakeFrame(*cue, &frame);

  typedef const mkvmuxer::uint8* data_t;
  const data_t buf = reinterpret_cast<data_t>(frame.data());

  const mkvmuxer::uint64 len = frame.length();

  const mkvmuxer::uint64 track_num = cues_map->first;

  return segment->AddMetadata(buf, len, track_num, start_ns, duration_ns);
}
