#ifndef _PARSER_H_
#define _PARSER_H_

#include <vector>
#include <string>

#include "imgui.h"

#include "common.h"
#include "render_common.h"

struct MorphProjectData {
	std::string sourceImagePath;
	std::string destImagePath;
	std::vector<Line> sourceLines;
	std::vector<Line> destLines;
};

enum TokenType {
	TOKEN_SRC_IMAGE,
	TOKEN_DST_IMAGE,
	TOKEN_SRC_LINES,
	TOKEN_DST_LINES,
	TOKEN_NUMBER,
	TOKEN_EOF,
	TOKEN_UNKNOWN
};

struct Token {	
	TokenType type;
};

struct SrcImageToken : Token {
	std::string string;
};

struct DstImageToken : Token {
	std::string string;
};

MorphProjectData ParseProjectFile(ATP_File projectFile);

#endif