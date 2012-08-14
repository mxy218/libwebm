// Copyright (c) 2012 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef SAMPLE_MUXER_METADATA_H_  // NOLINT
#define SAMPLE_MUXER_METADATA_H_

#include <list>
#include <map>
#include <set>

#include "mkvmuxer.hpp"
#include "webvttparser.h"

class SampleMuxerMetadata {
 public:
  SampleMuxerMetadata();

  // Bind this metadata object to the muxer instance.
  void Init(mkvmuxer::Segment* segment);

  enum Kind {
    kSubtitles,
    kCaptions,
    kDescriptions,
    kMetadata
  };

  // Parse the WebVTT file |filename| having the indicated |kind|, and
  // create a corresponding track in the segment.  Returns false on
  // error.
  bool Load(const char* filename, Kind kind);

  // Write any WebVTT cues whose time is less or equal to |time_ns| as
  // a metadata block in its corresponding track.  If |time_ns| is
  // negative, write all remaining cues. Returns false on error.
  bool Write(mkvmuxer::int64 time_ns);

 private:
  typedef libwebvtt::Cue cue_t;
  typedef std::list<cue_t> cues_t;
  typedef std::map<mkvmuxer::uint64, cues_t> cues_map_t;

  // Used to sort cues as they are loaded.
  struct MergeCue {
    bool operator>(mkvmuxer::int64 time_ns) const {
      const cue_t& c = *cue;

      // Cue start time expressed in milliseconds
      const mkvmuxer::int64 start_ms = c.start_time.presentation();

      // Cue start time expressed in nanoseconds (MKV time)
      const mkvmuxer::int64 start_ns = start_ms * 1000000;

      return (start_ns > time_ns);
    }
      
    // Write this cue as a metablock to |segment|.  Returns false on
    // error.
    bool Write(mkvmuxer::Segment* segment) const;

    cues_map_t::iterator cues_map;
    cues_t::iterator cue;
  };

  // Add a metadata track to the segment having the indicated |kind|,
  // returning the |track_num| that has been chosen for this track.
  // Returns false on error.
  bool AddTrack(Kind kind, mkvmuxer::uint64* track_num);

  // Parse the WebVTT |file| having the indicated |kind|, returning
  // the parsed list of |cues|.  Returns false on error.
  bool Parse(const char* file, Kind kind, cues_map_t::iterator);

  // Converts a WebVTT cue to a Matroska metadata block.
  static void MakeFrame(const cue_t& cue, std::string* frame);

  // Populate the cue identifier part of the metadata block.
  static void WriteCueIdentifier(
    const std::string& identifier,
    std::string* frame);

  // Populate the cue settings part of the metadata block.
  static void WriteCueSettings(
    const cue_t::settings_t& settings,
    std::string* frame);

  // Populate the payload part of the metadata block.
  static void WriteCuePayload(
    const cue_t::payload_t& payload,
    std::string* frame);

  mkvmuxer::Segment* segment_;

  // Map of parsed WebVTT cues, indexed by track number.
  cues_map_t cues_map_;

  struct CompareCues {
    bool operator()(const MergeCue& lhs, const MergeCue& rhs) const {
      if (lhs.cue->start_time < rhs.cue->start_time)
        return true;

      if (lhs.cue->start_time > rhs.cue->start_time)
        return false;

      return (lhs.cues_map->first < rhs.cues_map->first);
    }
  };

  // Set of cues ordered by time and then by track number.
  typedef std::multiset<MergeCue, CompareCues> merge_cues_t;
  merge_cues_t merge_cues_;

  // Disable copy ctor and copy assign.
  SampleMuxerMetadata(const SampleMuxerMetadata&);
  SampleMuxerMetadata& operator=(const SampleMuxerMetadata&);
};

#endif  // SAMPLE_MUXER_METADATA_H_  // NOLINT
