#ifndef SRC_SKIP_CALLBACK_H_
#define SRC_SKIP_CALLBACK_H_

#include "webm/callback.h"
#include "webm/dom_types.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

// An implementation of Callback that skips all elements. Every method that
// yields an action will yield Action::kSkip, and Reader::Skip will be called
// if the callback ever needs to process data from the byte stream.
class SkipCallback : public Callback {
 public:
  Status OnElementBegin(const ElementMetadata& metadata,
                        Action* action) override {
    *action = Action::kSkip;
    return Status(Status::kOkCompleted);
  }

  Status OnSegmentBegin(const ElementMetadata& metadata,
                        Action* action) override {
    *action = Action::kSkip;
    return Status(Status::kOkCompleted);
  }

  Status OnClusterBegin(const ElementMetadata& metadata, const Cluster& cluster,
                        Action* action) override {
    *action = Action::kSkip;
    return Status(Status::kOkCompleted);
  }

  Status OnSimpleBlockBegin(const ElementMetadata& metadata,
                            const SimpleBlock& simple_block,
                            Action* action) override {
    *action = Action::kSkip;
    return Status(Status::kOkCompleted);
  }

  Status OnBlockGroupBegin(const ElementMetadata& metadata,
                           Action* action) override {
    *action = Action::kSkip;
    return Status(Status::kOkCompleted);
  }

  Status OnBlockBegin(const ElementMetadata& metadata, const Block& block,
                      Action* action) override {
    *action = Action::kSkip;
    return Status(Status::kOkCompleted);
  }
};

}  // namespace webm

#endif  // SRC_SKIP_CALLBACK_H_
