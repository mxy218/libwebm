#ifndef SRC_CHAPTERS_PARSER_H_
#define SRC_CHAPTERS_PARSER_H_

#include "src/edition_entry_parser.h"
#include "src/master_parser.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#Chapters
// http://www.webmproject.org/docs/container/#Chapters
class ChaptersParser : public MasterParser {
 public:
  ChaptersParser()
      : MasterParser(MakeChild<EditionEntryParser>(Id::kEditionEntry)) {}
};

}  // namespace webm

#endif  // SRC_CHAPTERS_PARSER_H_
