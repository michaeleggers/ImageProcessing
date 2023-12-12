// NOTE: This parser is pretty much *NOT* how you would a write a parser
// for eg. a programming language. Normally you probably would want TOKENs
// to be abstract entities. This is not what is being done here. But, given
// the time-constraints, this is probably *good enough* for now. 
// Still. We should probably work on this as it is not very robust.
// If the input has errors (eg. a number is missing to define a line completely)
// the program most likely crashes.
// Maybe over christmas we have more time to work on this. But who knows...

#include "parser.h"

#include <stdint.h>

static uint8_t* c;
static uint32_t inputLength;
static uint32_t cursor = 0;

static uint32_t AdvanceToNextNonWhiteSpace(uint8_t* c) {
    uint32_t advanced = 0;
    
    return advanced;
}

static uint32_t Advance(uint32_t count) {
    uint32_t i = 0;
    while (cursor + i < inputLength) {        
        if (i == count) break;
        c++; cursor++; i++;
    }

    return count - i;
}

static void AdvanceToNextNonWhitespace() {
    while (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n') {
        c++; cursor++;
    }
}

static std::string GetString() {
    AdvanceToNextNonWhitespace();
    std::string result;
    while (*c != ' ' && *c != '\t' && *c != '\r' && *c != '\n') {
        result += *c;
        c++; cursor++;
    }

    return result;
}

static bool IsNumber() {
    AdvanceToNextNonWhitespace();

    if (*c >= '0' && *c <= '9') return true;

    return false;
}

static Token GetNextToken() {
    
    Token result;


    // The following are what in a programming language are
    // *keywords*. Usually one would store them in a hash-map
    // because string compares are slow. For us, it might be just
    // fine, though.
   
    std::string parsed = GetString();
    if (!parsed.compare("src_img_path")) {
        result.type = TOKEN_SRC_IMAGE;
    }
    else if (!parsed.compare("dst_img_path")) {
        result.type = TOKEN_DST_IMAGE;
    }
    else if (!parsed.compare("src")) {
        result.type = TOKEN_SRC_LINES;
    }
    else if (!parsed.compare("dst")) {
        result.type = TOKEN_DST_LINES;
    }

    return result;
}

static Line GetLine() {
    std::string lineNo = GetString();
    std::string xPosA = GetString();
    std::string yPosA = GetString();
    std::string xPosB = GetString();
    std::string yPosB = GetString();
    std::string absAx = GetString();
    std::string absAy = GetString();
    std::string absBx = GetString();
    std::string absBy = GetString();
    std::string editorScaleX = GetString();
    std::string editorScaleY = GetString();

    Line line;
    line.a = { glm::vec3(std::stof(xPosA), std::stof(yPosA), 0.0) };
    line.a = { glm::vec3(std::stof(xPosB), std::stof(yPosB), 0.0) };
    line.absA.x = std::stof(absAx);
    line.absA.y = std::stof(absAy);
    line.absB.x = std::stof(absBx);
    line.absB.y = std::stof(absBy);
    line.buttonSize.x = std::stof(editorScaleX);
    line.buttonSize.y = std::stof(editorScaleY);

    return line;
}

static std::vector<Line> GetLines() {
    
    std::vector<Line> result;
    while (IsNumber()) {
        Line line = GetLine();
        result.push_back(line);
    }


    return result;
}

MorphProjectData ParseProjectFile(ATP_File projectFile)
{
    MorphProjectData result;

    c = projectFile.data;
    inputLength = projectFile.size;

    cursor = 0;
    while (cursor < inputLength) {
        Token t = GetNextToken();

        if (t.type == TOKEN_EOF) break;

        if (t.type == TOKEN_SRC_IMAGE) {
            result.sourceImagePath = GetString();
        }
        else if (t.type == TOKEN_DST_IMAGE) {
            result.destImagePath = GetString();
        }
        else if (t.type == TOKEN_SRC_LINES) {
            result.sourceLines = GetLines();
        }
        else if (t.type == TOKEN_DST_LINES) {
            result.destLines = GetLines();
        }
    }

    return result;
}
