// Copyright (c) 2016 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#include "common/vp9_header_parser.h"

#include <string>

#include "common/hdr_util.h"
#include "gtest/gtest.h"
#include "mkvparser/mkvparser.h"
#include "mkvparser/mkvreader.h"
#include "testing/test_util.h"


namespace {

class Vp9HeaderParserTests : public ::testing::Test {
 public:
  Vp9HeaderParserTests() : is_reader_open_(false), segment_(NULL) {
  }

  virtual ~Vp9HeaderParserTests() {
    CloseReader();
    if (segment_ != NULL) {
      delete segment_;
      segment_ = NULL;
    }
  }

  void CloseReader() {
    if (is_reader_open_) {
      reader_.Close();
    }
    is_reader_open_ = false;
  }

  bool CreateAndLoadSegment(const std::string& filename,
                            int expected_doc_type_ver) {
    filename_ = test::GetTestFilePath(filename);
    if (reader_.Open(filename_.c_str())) {
      return false;
    }
    is_reader_open_ = true;
    pos_ = 0;
    mkvparser::EBMLHeader ebml_header;
    ebml_header.Parse(&reader_, pos_);
    EXPECT_EQ(1, ebml_header.m_version);
    EXPECT_EQ(1, ebml_header.m_readVersion);
    EXPECT_STREQ("webm", ebml_header.m_docType);
    EXPECT_EQ(expected_doc_type_ver, ebml_header.m_docTypeVersion);
    EXPECT_EQ(2, ebml_header.m_docTypeReadVersion);

    if (mkvparser::Segment::CreateInstance(&reader_, pos_, segment_)) {
      return false;
    }
    return !HasFailure() && segment_->Load() >= 0;
  }

  bool CreateAndLoadSegment(const std::string& filename) {
    return CreateAndLoadSegment(filename, 2);
  }

  void ProcessTheFrames() {
    unsigned char* data = NULL;
    long data_len = 0;
    const mkvparser::Tracks* const parser_tracks = segment_->GetTracks();
    ASSERT_TRUE(parser_tracks != NULL);
    const mkvparser::Cluster* cluster = segment_->GetFirst();
    ASSERT_TRUE(cluster);

    while ((cluster != NULL) && !cluster->EOS()) {
      const mkvparser::BlockEntry* block_entry;
      long status = cluster->GetFirst(block_entry);
      ASSERT_EQ(0, status);

      while ((block_entry != NULL) && !block_entry->EOS()) {
        const mkvparser::Block* const block = block_entry->GetBlock();
        ASSERT_TRUE(block != NULL);
        const long long trackNum = block->GetTrackNumber();
        const mkvparser::Track* const parser_track =
            parser_tracks->GetTrackByNumber(static_cast<unsigned long>(trackNum));
        ASSERT_TRUE(parser_track != NULL);
        const long long track_type = parser_track->GetType();

        if (track_type == mkvparser::Track::kVideo) {
          const int frame_count = block->GetFrameCount();

          for (int i = 0; i < frame_count; ++i) {
            const mkvparser::Block::Frame& frame = block->GetFrame(i);

            if (frame.len > data_len) {
              delete[] data;
              data = new unsigned char[frame.len];
              ASSERT_TRUE(data != NULL);
              data_len = frame.len;
            }
            ASSERT_FALSE(frame.Read(&reader_, data));
            parser_.SetFrame(data, data_len);
            parser_.ParseUncompressedHeader();
          }
        }

        status = cluster->GetNext(block_entry, block_entry);
        ASSERT_EQ(0, status);
      }

      cluster = segment_->GetNext(cluster);
    }
    delete[] data;
  }

 protected:
  mkvparser::MkvReader reader_;
  bool is_reader_open_;
  mkvparser::Segment* segment_;
  std::string filename_;
  long long pos_;
  vp9_parser::Vp9HeaderParser parser_;
};

TEST_F(Vp9HeaderParserTests, VideoOnlyFile) {
  ASSERT_TRUE(CreateAndLoadSegment("test_stereo_left_right.webm"));
  ProcessTheFrames();
  EXPECT_EQ(256, parser_.width());
  EXPECT_EQ(144, parser_.height());
  EXPECT_EQ(1, parser_.column_tiles());
  EXPECT_EQ(0, parser_.frame_parallel_mode());
}

TEST_F(Vp9HeaderParserTests, Muxed) {
  ASSERT_TRUE(CreateAndLoadSegment("bbb_480p_vp9_opus_1second.webm", 4));
  ProcessTheFrames();
  EXPECT_EQ(854, parser_.width());
  EXPECT_EQ(480, parser_.height());
  EXPECT_EQ(2, parser_.column_tiles());
  EXPECT_EQ(1, parser_.frame_parallel_mode());
}

}  // namespace

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
