#ifndef SRC_CHAPTER_DISPLAY_PARSER_H_
#define SRC_CHAPTER_DISPLAY_PARSER_H_

#include "src/byte_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#ChapterDisplay
// http://www.webmproject.org/docs/container/#ChapterDisplay
class ChapterDisplayParser : public MasterValueParser<ChapterDisplay> {
 public:
  ChapterDisplayParser()
      : MasterValueParser<ChapterDisplay>(
            MakeChild<StringParser>(Id::kChapString, &ChapterDisplay::string),
            MakeChild<StringParser>(Id::kChapLanguage,
                                    &ChapterDisplay::languages),
            MakeChild<StringParser>(Id::kChapCountry,
                                    &ChapterDisplay::countries)) {}
};

}  // namespace webm

#endif  // SRC_CHAPTER_DISPLAY_PARSER_H_
