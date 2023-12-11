#include "parser.h"

#include <stdint.h>

static uint8_t* c;
static uint32_t cursor = 0;

static TokenType LookAhead() {

}

static TokenType GetNextToken() {

}

static std::string GetString() {

}

static std::vector<Line> GetLines() {

}

MorphProjectData ParseProjectFile(ATP_File projectFile)
{
    MorphProjectData result;

    c = projectFile.data;
    cursor = 0;
    while (LookAhead() != TOKEN_EOF) {
        TokenType t = GetNextToken();
        if (t == TOKEN_SRC_IMAGE) {
            result.sourceImagePath = GetString();
        }
        else if (t == TOKEN_DST_IMAGE) {
            result.destImagePath = GetString();
        }
        else if (t == TOKEN_SRC_LINES) {
            result.sourceLines = GetLines();
        }
        else if (t == TOKEN_DST_LINES) {
            result.destLines = GetLines();
        }
    }

    return result;
}
