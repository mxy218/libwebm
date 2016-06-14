#ifndef SRC_CHAPTER_ATOM_PARSER_H_
#define SRC_CHAPTER_ATOM_PARSER_H_

#include "src/byte_parser.h"
#include "src/chapter_display_parser.h"
#include "src/int_parser.h"
#include "src/master_value_parser.h"
#include "src/recursive_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#ChapterAtom
// http://www.webmproject.org/docs/container/#ChapterAtom
class ChapterAtomParser : public MasterValueParser<ChapterAtom> {
 public:
  ChapterAtomParser()
      : MasterValueParser<ChapterAtom>(
            MakeChild<UnsignedIntParser>(Id::kChapterUid, &ChapterAtom::uid),
            MakeChild<StringParser>(Id::kChapterStringUid,
                                    &ChapterAtom::string_uid),
            MakeChild<UnsignedIntParser>(Id::kChapterTimeStart,
                                         &ChapterAtom::time_start),
            MakeChild<UnsignedIntParser>(Id::kChapterTimeEnd,
                                         &ChapterAtom::time_end),
            MakeChild<ChapterDisplayParser>(Id::kChapterDisplay,
                                            &ChapterAtom::displays),
            MakeChild<RecursiveParser<ChapterAtomParser>>(
                Id::kChapterAtom, &ChapterAtom::atoms)) {}
};

}  // namespace webm

#endif  // SRC_CHAPTER_ATOM_PARSER_H_
