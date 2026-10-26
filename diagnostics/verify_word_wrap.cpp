// SPDX-License-Identifier: GPL-2.0-or-later
#include "english_word_wrap.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

static std::string layout(const std::string &text, int width, bool words,
                          std::size_t *splits = NULL)
{
    std::string output;
    int column = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (words) {
            const tsukihime::WordWrap wrap = tsukihime::englishWordWrap(
                text.c_str(), i, column, width, 0);
            if (wrap.new_line) {
                output += '\n';
                column = 0;
                i += wrap.skip_spaces;
            }
        }
        const char c = text[i];
        if (c == '@') continue; // A wait does not occupy a character cell.
        if (c == '\\' || c == '\n') {
            output += '\n';
            column = 0;
            continue;
        }
        if (column >= width) {
            output += '\n';
            column = 0;
            if (splits && i > 0 && !tsukihime::wordBoundary(c) &&
                !tsukihime::wordBoundary(text[i - 1])) ++*splits;
        }
        output += c;
        ++column;
    }
    return output;
}

int main(int argc, char **argv)
{
    assert(layout("alpha beta gamma", 10, true) == "alpha beta\ngamma");
    assert(layout("one two three", 12, true) == "one two\nthree");
    assert(layout("one two", 7, true) == "one two");
    assert(layout("abcd  efgh", 8, true) == "abcd\nefgh");
    assert(layout("abcdefghijkl", 5, true) == "abcde\nfghij\nkl");
    assert(layout("abc\ndef ghi", 7, true) == "abc\ndef ghi");
    assert(layout("abcdef@ ghij", 8, true) == "abcdef\nghij");
    assert(layout("abcdef\\ghij", 8, true) == "abcdef\nghij");
    assert(layout("  abc", 8, true) == "  abc");
    assert(!tsukihime::englishWordWrap("word", 1, 7, 8, 0).new_line);
    assert(tsukihime::englishWordWrap("word", 0, 7, 8, 0).new_line);
    assert(!tsukihime::englishWordWrap(" word", 0, 2, 8, 2).new_line);
    assert(!tsukihime::englishWordWrap(" word", 0, 7, 8, 8).new_line);
    assert(!tsukihime::englishWordWrap(" @", 0, 7, 8, 0).new_line);
    assert(!tsukihime::englishWordWrap(" \x82\xa0", 0, 7, 8, 0).new_line);
    // UTF-8 / Shift-JIS bytes and controls are not counted as ASCII words.
    assert(!tsukihime::englishWordWrap("\xd0\x90", 0, 7, 8, 0).new_line);
    std::puts("Word wrapping, boundaries, waits, page breaks, indentation: OK");

    if (argc == 2) {
        std::ifstream file(argv[1], std::ios::binary);
        if (!file) return 1;
        std::string script((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        for (char &c : script) c = static_cast<unsigned char>(c) ^ 0x84;
        std::istringstream input(script);
        std::string line;
        std::size_t checked = 0, old_splits = 0, new_splits = 0;
        while (std::getline(input, line)) {
            const std::size_t first = line.find_first_not_of(" \t");
            if (first == std::string::npos || line[first] != '`') continue;
            std::string text = line.substr(first + 1);
            const std::size_t close = text.find('`');
            if (close != std::string::npos) text.resize(close);
            // This check models ordinary ASCII dialogue, not ruby or escapes.
            if (text.find('_') != std::string::npos) continue;
            bool ascii = true;
            for (unsigned char c : text) if (c > 126 || (c < 32 && c != '\r')) ascii = false;
            if (!ascii) continue;
            if (!text.empty() && text.back() == '\r') text.pop_back();
            // Exclude exceptionally long tokens: they intentionally fall back
            // to character wrapping, as covered by the synthetic test above.
            int run = 0, longest = 0;
            for (unsigned char c : text) {
                run = tsukihime::wordBoundary(c) ? 0 : run + 1;
                if (run > longest) longest = run;
            }
            if (longest > 72) continue;
            layout(text, 72, false, &old_splits);
            layout(text, 72, true, &new_splits);
            ++checked;
        }
        std::printf("English dialogue checked: %zu; mid-word breaks: %zu -> %zu\n",
                    checked, old_splits, new_splits);
        assert(checked > 1000 && old_splits > 0 && new_splits == 0);
    }
    return 0;
}
