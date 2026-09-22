#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Dump strings with escaping understood by both comment scanning and restore.
static inline void cliWriteQuotedString(const char *value, void (*writeChar)(uint8_t))
{
    writeChar('"');
    for (; *value; value++) {
        if (*value == '"' || *value == '\\') {
            writeChar('\\');
        }
        writeChar((uint8_t)*value);
    }
    writeChar('"');
}

static inline size_t cliUncommentedLength(const char *line, size_t length)
{
    bool quoted = false;
    for (size_t i = 0; i < length; i++) {
        if (quoted && line[i] == '\\' && i + 1 < length &&
            (line[i + 1] == '"' || line[i + 1] == '\\')) {
            i++;
        } else if (line[i] == '"') {
            quoted = !quoted;
        } else if (!quoted && line[i] == '#') {
            return i;
        }
    }
    return length;
}

static inline char *cliUnquoteString(char *value)
{
    const size_t length = strlen(value);
    if (length < 2 || value[0] != '"' || value[length - 1] != '"') {
        return value;
    }
    char *dest = value;
    for (size_t i = 1; i < length - 1; i++) {
        if (value[i] == '\\' && i + 1 < length - 1 &&
            (value[i + 1] == '"' || value[i + 1] == '\\')) {
            i++;
        }
        *dest++ = value[i];
    }
    *dest = 0;
    return value;
}
