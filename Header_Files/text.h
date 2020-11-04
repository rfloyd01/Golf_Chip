#pragma once

#include <string>
#include <Header_Files/glm.h>
#include <Header_Files/graphics.h>

//global definitions
#define number_of_message_types 5;

//structs and functions useful for the rendering of text
struct Character
{
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int Advance;
};

struct Text
{
	Text(std::string tex, float ex, float why, float sc, glm::vec3 col, float ep);
	std::string text;
	float x;
	float y;
	float scale;
	float x_end; //denotes the point on screen at witch text should wrap to the next line

	glm::vec3 color;
};

enum class MessageType
{
	//this enum class is used to group different messages displayed on screen
	TITLE = 0,
	SUB_TITLE = 1,
	BODY = 2,
	SENSOR_INFO = 3,
	FOOT_NOTE = 4
};

//Text Based Functions
MessageType mtFromInt(int m);