#ifndef SRC_BLOCK_GROUP_PARSER_H_
#define SRC_BLOCK_GROUP_PARSER_H_

#include "src/block_additions_parser.h"
#include "src/block_parser.h"
#include "src/int_parser.h"
#include "src/master_value_parser.h"
#include "src/slices_parser.h"
#include "src/virtual_block_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#BlockGroup
// http://www.webmproject.org/docs/container/#BlockGroup
class BlockGroupParser : public MasterValueParser<BlockGroup> {
 public:
  BlockGroupParser()
      : MasterValueParser<BlockGroup>(
            MakeChild<BlockParser>(Id::kBlock, &BlockGroup::block),
            MakeChild<VirtualBlockParser>(Id::kBlockVirtual,
                                          &BlockGroup::virtual_block),
            MakeChild<BlockAdditionsParser>(Id::kBlockAdditions,
                                            &BlockGroup::additions),
            MakeChild<UnsignedIntParser>(Id::kBlockDuration,
                                         &BlockGroup::duration),
            MakeChild<SignedIntParser>(Id::kReferenceBlock,
                                       &BlockGroup::references),
            MakeChild<SignedIntParser>(Id::kDiscardPadding,
                                       &BlockGroup::discard_padding),
            MakeChild<SlicesParser>(Id::kSlices, &BlockGroup::slices)) {}

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override {
    *num_bytes_read = 0;

    if (!parse_started_event_completed()) {
      Action action;
      Status status = OnParseStarted(callback, &action);
      if (!status.completed_ok()) {
        return status;
      }

      set_parse_started_event_completed_with_action(action);
    }

    return MasterValueParser::Feed(callback, reader, num_bytes_read);
  }

 protected:
  Status OnParseStarted(Callback* callback, Action* action) override {
    return callback->OnBlockGroupBegin(metadata(Id::kBlockGroup), action);
  }

  Status OnParseCompleted(Callback* callback) override {
    return callback->OnBlockGroupEnd(metadata(Id::kBlockGroup), value());
  }
};

}  // namespace webm

#endif  // SRC_BLOCK_GROUP_PARSER_H_
