#pragma once
#include <stdint.h>

// all assets for chess pieces downloaded from https://commons.wikimedia.org/wiki/File:Chess_Pieces_Sprite.svg

extern const uint8_t DATA_BLACK_PAWN[];
extern const uint8_t DATA_BLACK_ROOK[];
extern const uint8_t DATA_BLACK_KNIGHT[];
extern const uint8_t DATA_BLACK_BISHOP[];
extern const uint8_t DATA_BLACK_QUEEN[];
extern const uint8_t DATA_BLACK_KING[];

extern const uint8_t DATA_WHITE_PAWN[];
extern const uint8_t DATA_WHITE_ROOK[];
extern const uint8_t DATA_WHITE_KNIGHT[];
extern const uint8_t DATA_WHITE_BISHOP[];
extern const uint8_t DATA_WHITE_QUEEN[];
extern const uint8_t DATA_WHITE_KING[];

extern const uint8_t DATA_MOVE_TILE[];

extern const uint8_t DATA_HOME_BUTTON[];
extern const uint8_t DATA_BACK_BUTTON[];
extern const uint8_t DATA_PVE[];
extern const uint8_t DATA_PVP[];

// sound created using bosca ceoil

extern const uint8_t DATA_PLACE_PIECE_SOUND[];
extern const uint8_t DATA_CHECK_SOUND[];
extern const uint8_t DATA_CHECKMATE_SOUND[];

#if defined(CHS_INCLUDE_SHADER_STR)
static const std::string DATA_SHADER_STR =
"$type vertex\n"
"#version 450\n"
"layout(location = 0) in vec2 aposition;\n"
"layout(location = 1) in vec2 auv;\n"
"layout(location = 2) in vec3 acolor;\n"
"layout(location = 3) in int atexindex;\n"
"layout(location = 0) out vec2 ouv;\n"
"layout(location = 1) out vec3 ocolor;\n"
"layout(location = 2) flat out int otexindex;\n"
"layout(set = 0, binding = 0) uniform PROJ\n"
"{\n"
"mat4 proj;\n"
"}p;\n"
"void main()\n"
"{\n"
"gl_Position = p.proj * vec4(aposition, 0.0, 1.0);\n"
"ouv = auv;\n"
"ocolor = acolor;\n"
"otexindex = atexindex;\n"
"}\n"
"$type fragment\n"
"#version 450\n"
"layout(location = 0) out vec4 color;\n"
"layout(location = 0) in vec2 ouv;\n"
"layout(location = 1) in vec3 ocolor;\n"
"layout(location = 2) flat in int otexindex;\n"
"layout(set = 0, binding = 1) uniform sampler2D textures[13];\n"
"void main()\n"
"{\n"
"if (otexindex > -1)\n"
"{\n"
"color = texture(textures[otexindex], ouv);\n"
"color.rgb *= ocolor;\n"
"}\n"
"else\n"
"color = vec4(ocolor, 1.0);\n"
"if (color.w <= 0.3)\n"
"discard;\n"
"}\0"
;
#endif