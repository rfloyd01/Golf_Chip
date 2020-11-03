#include "pch.h"

#include <Header_Files/text.h>

Text::Text(std::string tex, float ex, float why, float sc, glm::vec3 col, float ep)
{
	text = tex;
	x = ex;
	x_end = ep;
	y = why;
	scale = sc;
	color = col;
}

MessageType mtFromInt(int m)
{
	if (m == 0) return MessageType::TITLE;
	else if (m == 1) return MessageType::SUB_TITLE;
	else if (m == 2) return MessageType::BODY;
	else if (m == 3) return MessageType::SENSOR_INFO;
	else if (m == 4) return MessageType::FOOT_NOTE;
}