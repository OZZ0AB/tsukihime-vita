// SPDX-License-Identifier: GPL-2.0-or-later
#include "dialogue_size.h"
#include "english_word_wrap.h"
#include <cassert>
#include <cstdio>
#include <string>

static std::string words(const std::string &text)
{
    std::string result;
    bool separator = true;
    for (unsigned char c : text) {
        if (c == ' ' || c == '\n' || c == '\t') {
            separator = true;
        }
        else {
            if (separator && !result.empty()) result += ' ';
            result += c;
            separator = false;
        }
    }
    return result;
}

static void checkPages(int level, const std::string &input)
{
    const tsukihime::DialogueSize size = tsukihime::dialogueSize(level);
    int column = 0, row = 0, pages = 1;
    std::string drawn;
    for (std::size_t offset = 0; offset < input.size();) {
        const tsukihime::WordWrap wrap = tsukihime::englishWordWrap(
            input.c_str(), offset, column, size.columns * 2, 0);
        if (wrap.new_line) {
            column = 0;
            row += 2;
            offset += wrap.skip_spaces;
            drawn += ' ';
        }
        if (level > 0 && tsukihime::dialogueNeedsPage(
                column, row, size.columns, size.rows)) {
            // Page advance must not consume the pending character.
            const std::size_t pending = offset;
            column = row = 0;
            ++pages;
            assert(offset == pending);
            continue;
        }
        if (column >= size.columns * 2) {
            column = 0;
            row += 2;
        }
        // Original-size pages use the script's page controls. This stress
        // paragraph exercises automatic pagination only at the enlarged sizes.
        if (level > 0) {
            assert(row / 2 < size.rows);
            assert(18 + (column + 1) * size.pitch_x / 2 <= 630);
            assert(18 + row / 2 * size.pitch_y + size.font <= 459);
        }
        drawn += input[offset++];
        ++column;
    }
    assert(words(drawn) == words(input));
    if (level > 0) assert(pages > 1);
    std::printf("Size %d/4: %d px, %d ASCII columns, %d rows; %d pages; text preserved\n",
                level + 1, size.font, size.columns * 2, size.rows, pages);
}

int main()
{
    int level = 0;
    for (int expected = 1; expected <= 4; ++expected) {
        level = tsukihime::nextDialogueSize(level);
        assert(level == expected % 4);
    }
    std::string paragraph;
    for (int i = 0; i < 150; ++i)
        paragraph += "Readable text stays inside the window, with every word retained. ";
    paragraph += std::string(95, 'x'); // Longer than a complete line.
    for (int size = 0; size < 4; ++size) {
        const tsukihime::DialogueSize layout = tsukihime::dialogueSize(size);
        assert(layout.columns * layout.pitch_x <= 612);
        assert((layout.rows - 1) * layout.pitch_y + layout.font <= 441);
        if (size > 0) {
            assert(layout.font > tsukihime::dialogueSize(size - 1).font);
            assert(layout.font <= 23);
        }
        assert(!tsukihime::dialogueNeedsPage(0, 0, layout.columns, layout.rows));
        assert(tsukihime::dialogueNeedsPage(0, layout.rows * 2, layout.columns, layout.rows));
        assert(tsukihime::dialogueNeedsPage(layout.columns * 2,
                                          (layout.rows - 1) * 2,
                                          layout.columns, layout.rows));
        checkPages(size, paragraph);
    }
    std::puts("Four-step cycle, screen bounds, page capacity and text preservation: OK");
}
