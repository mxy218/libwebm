// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <sys/stat.h>

// libwebm parser includes
#include "mkvreader.hpp"
#include "mkvparser.hpp"

// libwebm muxer includes
#include "mkvmuxer.hpp"
#include "mkvwriter.hpp"
#include "mkvmuxerutil.hpp"

// webvtt includes
#include "webvttparser.h"
#include "vttreader.h"

using mkvmuxer::uint64;
using mkvmuxer::uint8;
using std::string;

namespace {

void Usage() {
  printf("Usage: sample_muxer -i input -o output [options]\n");
  printf("\n");
  printf("Main options:\n");
  printf("  -h | -?                     show help\n");
  printf("  -video <int>                >0 outputs video\n");
  printf("  -audio <int>                >0 outputs audio\n");
  printf("  -live <int>                 >0 puts the muxer into live mode\n");
  printf("                              0 puts the muxer into file mode\n");
  printf("  -output_cues <int>          >0 outputs cues element\n");
  printf("  -cues_on_video_track <int>  >0 outputs cues on video track\n");
  printf("  -cues_on_audio_track <int>  >0 outputs cues on audio track\n");
  printf("  -max_cluster_duration <double> in seconds\n");
  printf("  -max_cluster_size <int>     in bytes\n");
  printf("  -switch_tracks <int>        >0 switches tracks in output\n");
  printf("  -audio_track_number <int>   >0 Changes the audio track number\n");
  printf("  -video_track_number <int>   >0 Changes the video track number\n");
  printf("  -chunking <string>          Chunk output\n");
  printf("\n");
  printf("Video options:\n");
  printf("  -display_width <int>        Display width in pixels\n");
  printf("  -display_height <int>       Display height in pixels\n");
  printf("  -stereo_mode <int>          3D video mode\n");
  printf("\n");
  printf("Cues options:\n");
  printf("  -output_cues_block_number <int> >0 outputs cue block number\n");
}

class Metadata {
 public:
  Metadata();
  void Init(mkvmuxer::Segment*);
  bool LoadSubtitles(const char* subtitles_filename);
  bool AddTracks();
  bool Write(long long time_ns);

 private:
  mkvmuxer::Segment* segment_;

  typedef libwebvtt::Cue cue_t;
  typedef std::list<cue_t> cues_t;

  cues_t subtitles_;
  uint64 subtitles_num_;

  bool Load(const char* filename, const char* name, cues_t*);
  bool AddTrack(const char* name, const cues_t&, int type, uint64* num);

  int DoWrite(long long time_ns);
  int DoWrite(long long time_ns, cues_t&, uint64 track_num);

  void MakeFrame(const cue_t&, std::string* frame);

 private:
  Metadata(const Metadata&);
  Metadata& operator=(const Metadata&);

};

} //end namespace

int main(int argc, char* argv[]) {
  char* input = NULL;
  char* output = NULL;

  // Segment variables
  bool output_video = true;
  bool output_audio = true;
  bool live_mode = false;
  bool output_cues = true;
  bool cues_on_video_track = true;
  bool cues_on_audio_track = false;
  uint64 max_cluster_duration = 0;
  uint64 max_cluster_size = 0;
  bool switch_tracks = false;
  int audio_track_number = 0; // 0 tells muxer to decide.
  int video_track_number = 0; // 0 tells muxer to decide.
  bool chunking = false;
  const char* chunk_name = NULL;

  bool output_cues_block_number = true;

  uint64 display_width = 0;
  uint64 display_height = 0;
  uint64 stereo_mode = 0;

  const char* webvtt_subtitles_filename = NULL;

  const int argc_check = argc - 1;
  for (int i = 1; i < argc; ++i) {
    char* end;

    if (!strcmp("-h", argv[i]) || !strcmp("-?", argv[i])) {
      Usage();
      return EXIT_SUCCESS;
    } else if (!strcmp("-i", argv[i]) && i < argc_check) {
      input = argv[++i];
    } else if (!strcmp("-o", argv[i]) && i < argc_check) {
      output = argv[++i];
    } else if (!strcmp("-video", argv[i]) && i < argc_check) {
      output_video = strtol(argv[++i], &end, 10) == 0 ? false : true;
    } else if (!strcmp("-audio", argv[i]) && i < argc_check) {
      output_audio = strtol(argv[++i], &end, 10) == 0 ? false : true;
    } else if (!strcmp("-live", argv[i]) && i < argc_check) {
      live_mode = strtol(argv[++i], &end, 10) == 0 ? false : true;
    } else if (!strcmp("-output_cues", argv[i]) && i < argc_check) {
      output_cues = strtol(argv[++i], &end, 10) == 0 ? false : true;
    } else if (!strcmp("-cues_on_video_track", argv[i]) && i < argc_check) {
      cues_on_video_track = strtol(argv[++i], &end, 10) == 0 ? false : true;
      if (cues_on_video_track)
        cues_on_audio_track = false;
    } else if (!strcmp("-cues_on_audio_track", argv[i]) && i < argc_check) {
      cues_on_audio_track = strtol(argv[++i], &end, 10) == 0 ? false : true;
      if (cues_on_audio_track)
        cues_on_video_track = false;
    } else if (!strcmp("-max_cluster_duration", argv[i]) && i < argc_check) {
      const double seconds = strtod(argv[++i], &end);
      max_cluster_duration =
          static_cast<uint64>(seconds * 1000000000.0);
    } else if (!strcmp("-max_cluster_size", argv[i]) && i < argc_check) {
      max_cluster_size = strtol(argv[++i], &end, 10);
    } else if (!strcmp("-switch_tracks", argv[i]) && i < argc_check) {
      switch_tracks = strtol(argv[++i], &end, 10) == 0 ? false : true;
    } else if (!strcmp("-audio_track_number", argv[i]) && i < argc_check) {
      audio_track_number = strtol(argv[++i], &end, 10);
    } else if (!strcmp("-video_track_number", argv[i]) && i < argc_check) {
      video_track_number = strtol(argv[++i], &end, 10);
    } else if (!strcmp("-chunking", argv[i]) && i < argc_check) {
      chunking = true;
      chunk_name = argv[++i];
    } else if (!strcmp("-display_width", argv[i]) && i < argc_check) {
      display_width = strtol(argv[++i], &end, 10);
    } else if (!strcmp("-display_height", argv[i]) && i < argc_check) {
      display_height = strtol(argv[++i], &end, 10);
    } else if (!strcmp("-stereo_mode", argv[i]) && i < argc_check) {
      stereo_mode = strtol(argv[++i], &end, 10);
    } else if (!strcmp("-output_cues_block_number", argv[i]) &&
               i < argc_check) {
      output_cues_block_number =
          strtol(argv[++i], &end, 10) == 0 ? false : true;
    }
    else if (!strcmp("-webvtt-subtitles", argv[i])) {
      ++i;  // consume arg name

      if (i > argc_check) {
	printf("missing value for -webvtt-subtitles\n");
	return EXIT_FAILURE;
      }

      webvtt_subtitles_filename = argv[i];
      struct ::stat buf;

      if (::stat(webvtt_subtitles_filename, &buf) != 0) {
        printf("bad value for -webvtt-subtitles: \"%s\"\n",
               webvtt_subtitles_filename);
        return EXIT_FAILURE;
      }
    }
  }

  if (input == NULL || output == NULL) {
    Usage();
    return EXIT_FAILURE;
  }

  Metadata metadata;

  if (!metadata.LoadSubtitles(webvtt_subtitles_filename))
    return EXIT_FAILURE;

  //TODO(matthewjheaney): load other metadata files

  // Get parser header info
  mkvparser::MkvReader reader;

  if (reader.Open(input)) {
    printf("\n Filename is invalid or error while opening.\n");
    return EXIT_FAILURE;
  }

  long long pos = 0;
  mkvparser::EBMLHeader ebml_header;
  ebml_header.Parse(&reader, pos);

  mkvparser::Segment* parser_segment;
  long long ret = mkvparser::Segment::CreateInstance(&reader,
                                                     pos,
                                                     parser_segment);
  if (ret) {
    printf("\n Segment::CreateInstance() failed.");
    return EXIT_FAILURE;
  }

  ret = parser_segment->Load();
  if (ret < 0) {
    printf("\n Segment::Load() failed.");
    return EXIT_FAILURE;
  }

  const mkvparser::SegmentInfo* const segment_info = parser_segment->GetInfo();
  const long long timeCodeScale = segment_info->GetTimeCodeScale();

  // Set muxer header info
  mkvmuxer::MkvWriter writer;

  if (!writer.Open(output)) {
    printf("\n Filename is invalid or error while opening.\n");
    return EXIT_FAILURE;
  }

  // Set Segment element attributes
  mkvmuxer::Segment muxer_segment;

  if (!muxer_segment.Init(&writer)) {
    printf("\n Could not initialize muxer segment!\n");
    return EXIT_FAILURE;
  }

  metadata.Init(&muxer_segment);

  if (live_mode)
    muxer_segment.set_mode(mkvmuxer::Segment::kLive);
  else
    muxer_segment.set_mode(mkvmuxer::Segment::kFile);

  if (chunking)
    muxer_segment.SetChunking(true, chunk_name);

  if (max_cluster_duration > 0)
    muxer_segment.set_max_cluster_duration(max_cluster_duration);
  if (max_cluster_size > 0)
    muxer_segment.set_max_cluster_size(max_cluster_size);
  muxer_segment.OutputCues(output_cues);

  // Set SegmentInfo element attributes
  mkvmuxer::SegmentInfo* const info = muxer_segment.GetSegmentInfo();
  info->set_timecode_scale(timeCodeScale);
  info->set_writing_app("sample_muxer");

  // Set Tracks element attributes
  const mkvparser::Tracks* const parser_tracks = parser_segment->GetTracks();
  unsigned long i = 0;
  uint64 vid_track = 0; // no track added
  uint64 aud_track = 0; // no track added

  using mkvparser::Track;

  while (i != parser_tracks->GetTracksCount()) {
    int track_num = i++;
    if (switch_tracks)
      track_num = i % parser_tracks->GetTracksCount();

    const mkvparser::Track* const parser_track =
        parser_tracks->GetTrackByIndex(track_num);

    if (parser_track == NULL)
      continue;

    // TODO(fgalligan): Add support for language to parser.
    const char* const track_name = parser_track->GetNameAsUTF8();

    const long long track_type = parser_track->GetType();

    if (track_type == Track::kVideo && output_video) {
      // Get the video track from the parser
      const mkvparser::VideoTrack* const pVideoTrack =
          static_cast<const mkvparser::VideoTrack*>(parser_track);
      const long long width =  pVideoTrack->GetWidth();
      const long long height = pVideoTrack->GetHeight();

      // Add the video track to the muxer
      vid_track = muxer_segment.AddVideoTrack(static_cast<int>(width),
                                              static_cast<int>(height),
                                              video_track_number);
      if (!vid_track) {
        printf("\n Could not add video track.\n");
        return EXIT_FAILURE;
      }

      mkvmuxer::VideoTrack* const video =
          static_cast<mkvmuxer::VideoTrack*>(
              muxer_segment.GetTrackByNumber(vid_track));
      if (!video) {
        printf("\n Could not get video track.\n");
        return EXIT_FAILURE;
      }

      if (track_name)
        video->set_name(track_name);

      if (display_width > 0)
        video->set_display_width(display_width);
      if (display_height > 0)
        video->set_display_height(display_height);
      if (stereo_mode > 0)
        video->SetStereoMode(stereo_mode);

      const double rate = pVideoTrack->GetFrameRate();
      if (rate > 0.0) {
        video->set_frame_rate(rate);
      }
    } else if (track_type == Track::kAudio && output_audio) {
      // Get the audio track from the parser
      const mkvparser::AudioTrack* const pAudioTrack =
          static_cast<const mkvparser::AudioTrack*>(parser_track);
      const long long channels =  pAudioTrack->GetChannels();
      const double sample_rate = pAudioTrack->GetSamplingRate();

      // Add the audio track to the muxer
      aud_track = muxer_segment.AddAudioTrack(static_cast<int>(sample_rate),
                                              static_cast<int>(channels),
                                              audio_track_number);
      if (!aud_track) {
        printf("\n Could not add audio track.\n");
        return EXIT_FAILURE;
      }

      mkvmuxer::AudioTrack* const audio =
          static_cast<mkvmuxer::AudioTrack*>(
              muxer_segment.GetTrackByNumber(aud_track));
      if (!audio) {
        printf("\n Could not get audio track.\n");
        return EXIT_FAILURE;
      }

      if (track_name)
        audio->set_name(track_name);

      size_t private_size;
      const unsigned char* const private_data =
          pAudioTrack->GetCodecPrivate(private_size);
      if (private_size > 0) {
        if (!audio->SetCodecPrivate(private_data, private_size)) {
          printf("\n Could not add audio private data.\n");
          return EXIT_FAILURE;
        }
      }

      const long long bit_depth = pAudioTrack->GetBitDepth();
      if (bit_depth > 0)
        audio->set_bit_depth(bit_depth);
    }
  }

  if (!metadata.AddTracks())
    return EXIT_FAILURE;

  // Set Cues element attributes
  mkvmuxer::Cues* const cues = muxer_segment.GetCues();
  cues->set_output_block_number(output_cues_block_number);
  if (cues_on_video_track && vid_track)
    muxer_segment.CuesTrack(vid_track);
  if (cues_on_audio_track && aud_track)
    muxer_segment.CuesTrack(aud_track);

  // Write clusters
  unsigned char* data = NULL;
  int data_len = 0;

  const mkvparser::Cluster* cluster = parser_segment->GetFirst();

  while ((cluster != NULL) && !cluster->EOS()) {
    const mkvparser::BlockEntry* block_entry;

    long status = cluster->GetFirst(block_entry);

    if (status)
    {
        printf("\n Could not get first block of cluster.\n");
        return EXIT_FAILURE;
    }

    while ((block_entry != NULL) && !block_entry->EOS()) {
      const mkvparser::Block* const block = block_entry->GetBlock();
      const long long trackNum = block->GetTrackNumber();
      const mkvparser::Track* const parser_track =
          parser_tracks->GetTrackByNumber(
              static_cast<unsigned long>(trackNum));
      const long long track_type = parser_track->GetType();
      const long long time_ns = block->GetTime(cluster);

      if (!metadata.Write(time_ns))
        return EXIT_FAILURE;

      if ((track_type == Track::kAudio && output_audio) ||
          (track_type == Track::kVideo && output_video)) {
        const int frame_count = block->GetFrameCount();
        const bool is_key = block->IsKey();

        for (int i = 0; i < frame_count; ++i) {
          const mkvparser::Block::Frame& frame = block->GetFrame(i);

          if (frame.len > data_len) {
            delete [] data;
            data = new unsigned char[frame.len];
            if (!data)
              return EXIT_FAILURE;
            data_len = frame.len;
          }

          if (frame.Read(&reader, data))
            return EXIT_FAILURE;

          uint64 track_num = vid_track;
          if (track_type == Track::kAudio)
            track_num = aud_track;

          if (!muxer_segment.AddFrame(data,
                                      frame.len,
                                      track_num,
                                      time_ns,
                                      is_key)) {
            printf("\n Could not add frame.\n");
            return EXIT_FAILURE;
          }
        }
      }

      status = cluster->GetNext(block_entry, block_entry);

      if (status)
      {
          printf("\n Could not get next block of cluster.\n");
          return EXIT_FAILURE;
      }
    }

    cluster = parser_segment->GetNext(cluster);
  }

  if (!metadata.Write(-1))
    return EXIT_FAILURE;

  muxer_segment.Finalize();

  delete [] data;
  delete parser_segment;

  writer.Close();
  reader.Close();

  return EXIT_SUCCESS;
}

namespace {

Metadata::Metadata() : segment_(NULL) {
}

void Metadata::Init(mkvmuxer::Segment* s) {
  segment_ = s;
}

bool Metadata::LoadSubtitles(const char* f) {
  return Load(f, "subtitles", &subtitles_);
}

bool Metadata::Load(const char* file,
                    const char* name,
                    cues_t* cues) {
  if (file == NULL)
    return true;

  VttReader r;

  int e = r.Open(file);

  if (e) {
    printf("Unable to open %s file: \"%s\"\n", name, file);
    return false;
  }

  libwebvtt::Parser p(&r);

  e = p.Init();

  if (e < 0) {  // error
    printf("Error parsing %s file\n", name);
    return false;
  }

  libwebvtt::Time t;
  t.hours = -1;

  for (;;) {
    cues->push_back(cue_t());
    cue_t& c = cues->back();

    e = p.Parse(&c);

    if (e < 0) {  // error
      printf("Error parsing %s file\n", name);
      return false;
    }

    if (e > 0) {  // EOF
      cues->pop_back();
      return true;
    }

    if (c.start_time >= t)
      t = c.start_time;
    else {
      printf("bad %s cue timestamp (out-of-order)\n", name);
      return false;
    }

    if (c.stop_time < c.start_time) {
      printf("bad %s cue timestamp (stop < start)\n", name);
      return false;
    }
  }
}

bool Metadata::AddTracks() {
  if (!AddTrack("subtitles", subtitles_, 0x11, &subtitles_num_))
    return false;

  return true;
}

bool Metadata::AddTrack(const char* name,
                        const cues_t& cues,
                        int track_type,
                        uint64* num) {
  *num = 0;

  if (cues.empty())
    return true;

  *num = segment_->AddTrack(track_type);

  //TODO(matthewjheaney): pass in name and language
  //GetTrackByNumber, then set_name and set_language

  if (*num)
    return true;

  printf("\n Could not add %s track.\n", name);
  return false;
}

bool Metadata::Write(long long time_ns) {
  for (;;) {
    const int result = DoWrite(time_ns);

    if (result < 0)  // error
      return false;

    if (result > 0)  // nothing more to do
      return true;
  }
}

int Metadata::DoWrite(long long time_ns) {
  bool done = true;

  int result = DoWrite(time_ns, subtitles_, subtitles_num_);

  if (result < 0)  // error
    return result;

  if (result == 0)  // did something
    done = false;

  //TODO(matthewjheaney): DoWrite for each metadata kind

  return done ? 1 : 0;
}

int Metadata::DoWrite(long long time_ns,
                      cues_t& cues,
                      uint64 track_num) {
  if (cues.empty())
    return 1;  // nothing to do

  const cue_t& c = cues.front();
  const libwebvtt::presentation_t st_ms = c.start_time.presentation();
  const long long st_ns = st_ms * 1000000LL;

  if (time_ns >= 0 && st_ns > time_ns)
    return 1;  // nothing to do (yet)

  const libwebvtt::presentation_t sp_ms = c.stop_time.presentation();
  const long long sp_ns = sp_ms * 1000000LL;

  const long long dur_ns = sp_ns - st_ns;

  if (dur_ns < 0) {
    printf("\n Metadata has bad duration.\n");
    return -1;  // error
  }

  string f;  // frame
  MakeFrame(c, &f);

  const uint8* const data = reinterpret_cast<const uint8*>(f.data());

  if (!segment_->AddMetadata(data, f.length(), track_num, st_ns, dur_ns)) {
    printf("\n Could not add metadata.\n");
    return -1;  // error
  }

  cues.pop_front();

  return 0;  // succeeded (did something)
}

void Metadata::MakeFrame(const cue_t& c, std::string* pf) {
  string& f = *pf;

  f.clear();

  f.append(c.identifier);
  f.push_back('\x0A');  // LF

  {
    const cue_t::settings_t& ss = c.settings;
    typedef cue_t::settings_t::const_iterator iter_t;

    iter_t i = ss.begin();
    const iter_t j = ss.end();

    while (i != j) {
      const libwebvtt::Setting& s = *i++;

      f.append(s.name);
      f.push_back(':');
      f.append(s.value);
    }

    f.push_back('\x0A');  // LF
  }

  {
    const cue_t::payload_t& pp = c.payload;
    typedef cue_t::payload_t::const_iterator iter_t;

    iter_t i = pp.begin();
    const iter_t j = pp.end();

    while (i != j) {
      const string& p = *i++;
      f.append(p);
      f.push_back('\x0A');  // LF
    }
  }
}

}  // end anonymous namespace
