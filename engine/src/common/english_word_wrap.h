// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef TSUKIHIME_ENGLISH_WORD_WRAP_H
#define TSUKIHIME_ENGLISH_WORD_WRAP_H

#include <cstddef>

namespace tsukihime {

struct WordWrap {
    std::size_t skip_spaces;
    bool new_line;
};

inline bool wordBoundary(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '@' || c == '\\';
}

// The engine positions ASCII in half-width cells. Only inspect ordinary ASCII
// text: click/page controls and multibyte text must stay with the script parser.
inline WordWrap englishWordWrap(const char *text, std::size_t offset,
                               int column, int width, int indent)
{
    WordWrap result = {0, false};
    if (!text || width <= 0 || indent < 0 || indent >= width || column < 0)
        return result;
    const char *next = text + offset;
    if (!wordBoundary(static_cast<unsigned char>(*next)) && offset != 0 &&
        !wordBoundary(static_cast<unsigned char>(text[offset - 1])))
        return result;

    std::size_t spaces = 0;
    while (next[spaces] == ' ' || next[spaces] == '\t') ++spaces;
    const char *word = next + spaces;
    const int capacity = width - indent;
    int length = 0;
    while (length <= capacity) {
        const unsigned char c = static_cast<unsigned char>(word[length]);
        if (c < '!' || c > '~' || c == '@' || c == '\\' || c == '_') break;
        ++length;
    }
    // Leave controls, Japanese text, explicit spacing and overlong words alone.
    // Overlong words still use the engine's character wrapping as a fallback.
    if (length == 0 || column <= indent) return result;
    if ((spaces != 0 && column >= width) ||
        (length <= capacity &&
         (column >= width || spaces + static_cast<std::size_t>(length) >
                              static_cast<std::size_t>(width - column)))) {
        result.skip_spaces = spaces;
        result.new_line = true;
    }
    return result;
}

} // namespace tsukihime

#endif
