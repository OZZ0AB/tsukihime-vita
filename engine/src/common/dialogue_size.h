// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef TSUKIHIME_DIALOGUE_SIZE_H
#define TSUKIHIME_DIALOGUE_SIZE_H

namespace tsukihime {

struct DialogueSize {
    int font, pitch_x, pitch_y, columns, rows;
};

// Mirror moon's 640x480 dialogue window starts at (18,18). Keep its 612x441
// text area, reserving a bottom row for the click cursor at enlarged sizes.
inline DialogueSize dialogueSize(int level)
{
    if (level < 0 || level > 3) level = 0;
    const int font = 17 + level * 2;
    DialogueSize result = {font, font, font + 4, 36, 21};
    if (level != 0) {
        result.columns = 612 / font;
        result.rows = (441 - font) / result.pitch_y;
    }
    return result;
}

inline int nextDialogueSize(int level)
{
    return (level >= 0 && level < 3) ? level + 1 : 0;
}

inline bool dialogueNeedsPage(int column, int row, int columns, int rows)
{
    if (columns <= 0 || rows <= 0) return false;
    return row / 2 + (column >= columns * 2 ? 1 : 0) >= rows;
}

} // namespace tsukihime
#endif
