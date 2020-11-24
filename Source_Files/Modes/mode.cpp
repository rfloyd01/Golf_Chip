#include "pch.h"

#include <iostream>
#include <Header_Files/Modes/mode.h>

//PUBLIC FUNCTIONS
//Constructors
Mode::Mode(GL& graphics)
{
	p_graphics = &graphics;
	clearAllText();
}

//Updating and Advancement Functions
void Mode::update()
{
	processInput();
}
void Mode::processInput()
{
	if (!p_graphics->GetCanPressKey()) return; //only process input if input processing is available

	if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		p_graphics->getCurrentMode()->modeEnd(); //end the current mode
		p_graphics->setCurrentMode(ModeType::MAIN_MENU); //set the new mode
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		//pressing Enter will return the club or chip to the center of the screen if currently being rendered, useful for when image starts to drift over time
		p_graphics->setRotationQuaternion({ 1, 0, 0, 0 });
		p_graphics->setMagField(); //resets the magnetic field of the sensor
		p_graphics->getCurrentMode()->setClubLocation({ 0.0, 0.0, 0.0 }); //moves club back to center while maintaining current scale
		p_graphics->resetKeyTimer();
	}
}
void Mode::modeStart()
{
	//initialize text to render
	//initialize images to render

	//set up other basic veriables specific to each mode type
}
void Mode::modeEnd()
{
	//clear data from message map to free up space
	//clear data from model map to free up space

	//clear out any other data to free up space (such as calibration data in calibration mode or quaternion data in replay mode)
	//reset boolean variables or other variables that keep track of where in mode you are
}

//Get Functions
ModeType Mode::getModeType()
{
	return mode_type;
}
glm::vec3 Mode::getCameraLocation()
{
	return camera_location;
}
std::map<MessageType, std::vector<std::vector<Text> > >* Mode::getRenderText()
{
	return &message_map;
}
std::map<ModelType, std::vector<Model> >* Mode::getRenderModels()
{
	return &model_map;
}
glm::vec3 Mode::getBackgroundColor()
{
	return background_color;
}

//Set Functions
void Mode::setClubRotation(glm::quat q)
{
	//sets the rotation matrix for all models in the CLUB and CHIP categories to the quaternion 'q'
	for (int i = 0; i < model_map[ModelType::CLUB].size(); i++)
	{
		model_map[ModelType::CLUB][i].setRotation(q);
	}

	for (int i = 0; i < model_map[ModelType::CHIP].size(); i++)
	{
		model_map[ModelType::CHIP][i].setRotation(q);
	}
}
void Mode::setClubLocation(glm::vec3 l)
{
	//sets the translation matrix for all models in the CLUB and CHIP categories to the vector 'l'
	for (int i = 0; i < model_map[ModelType::CLUB].size(); i++)
	{
		model_map[ModelType::CLUB][i].setLocation(l);
	}

	for (int i = 0; i < model_map[ModelType::CHIP].size(); i++)
	{
		model_map[ModelType::CHIP][i].setLocation(l);
	}
}
void Mode::setClubScale(glm::vec3 s)
{
	//sets the scale matrix for all models in the CLUB and CHIP categories to the vector 's'
	for (int i = 0; i < model_map[ModelType::CLUB].size(); i++)
	{
		model_map[ModelType::CLUB][i].setScale(s);
	}

	for (int i = 0; i < model_map[ModelType::CHIP].size(); i++)
	{
		model_map[ModelType::CHIP][i].setScale(s);
	}
}

//PRIVATE FUNCTIONS
//Text Based Functions
void Mode::clearAllText()
{
	//clears whatever is currently stored in the messages map and re-creates empty containers for all the message types
	message_map.clear();
	std::vector<std::vector<Text> > blank_text;
	int heyhey = number_of_message_types;
	for (int i = 0; i < heyhey; i++) message_map[mtFromInt(i)] = blank_text;
}
void Mode::clearMessageType(MessageType mt)
{
	//completely clears out the specified message type
	message_map[mt].clear();
}
void Mode::addText(MessageType mt, Text new_text)
{
	//adds a new message to the end of the current message vector
	std::vector<Text> new_message;
	new_message.push_back(new_text);
	message_map[mt].push_back(new_message);

	//check to see if message needs to be broken up into sub-messaages
	createSubMessages(mt, message_map[mt].size() - 1);
}
void Mode::editMessage(MessageType mt, int index, Text edited_message)
{
	message_map[mt][index].clear();
	message_map[mt][index].push_back(edited_message);
	createSubMessages(mt, index); //create new sub-messages if necessary
}
void Mode::editMessageText(MessageType mt, int index, std::string new_text)
{
	Text nt = { new_text, message_map[mt][index][0].x,  message_map[mt][index][0].y, message_map[mt][index][0].scale, message_map[mt][index][0].color, message_map[mt][index][0].x_end };
	message_map[mt][index].clear();
	message_map[mt][index].push_back(nt);
	createSubMessages(mt, index); //create new sub-messages if necessary
}
void Mode::editMessageLocation(MessageType mt, int index, float new_x, float new_y)
{
	Text new_text = { message_map[mt][index][0].text, new_x,  new_y, message_map[mt][index][0].scale, message_map[mt][index][0].color, message_map[mt][index][0].x_end };
	message_map[mt][index].clear();
	message_map[mt][index].push_back(new_text);
	createSubMessages(mt, index); //create new sub-messages if necessary
}
void Mode::createSubMessages(MessageType mt, int index)
{
	//First, run through entire message and figure out where the top left and bottom right corners of text will be.
	//If either of these appear fall outside of the OpenGL screen proceed to break up the message
	std::string::const_iterator c, new_word; //new_word tracks the starting location of a new word and get's reset if there's a space
	Character ch;

	float temp_x;
	float max_y = 0, min_y = p_graphics->getScreenHeight(); //set max_y at min screen height and min_y at current screen height so they will get updated no matter what
	float scale = message_map[mt][index][0].scale;
	for (int m = 0; m < message_map[mt][index].size(); m++)
	{
		temp_x = message_map[mt][index][m].x;
		for (c = message_map[mt][index][m].text.begin(); c != message_map[mt][index][m].text.end(); c++)
		{
			//*c will return the character at current location in string
			ch = *(p_graphics->getCharacterInfo(*c)); //reference character render data stored in the p_graphics class
			if (*c == ' ') new_word = c + 1; //if a space is encountered it means a new word is about to start, makr this spot in case message needs to be wrapped to next line here

			float xpos = temp_x + ch.Bearing.x * scale; //indicates where next letter starts
			float ypos = message_map[mt][index][m].y - (ch.Size.y - ch.Bearing.y) * scale; //ypos denotes the bottom most part of the glyph

			//if xpos is greater than x_end cutoff limit, cut off current message at last space and create new message from that point, append it to messages[index]
			if (xpos + (ch.Advance >> 6) * scale >= message_map[mt][index][m].x_end) //xpos + (ch.Advance >> 6) * scale = end location of current letter
			{
				std::string next_text = "";
				for (auto cc = new_word; cc != message_map[mt][index][m].text.end(); cc++)
				{
					next_text += *cc;
				}
				message_map[mt][index][m].text.erase(new_word - 1, message_map[mt][index][m].text.end()); //subtract 1 from new_word to get rid of the space there

				//Add new Text to current index of messages. Next line starts the height of the capital letter 'L' * 1.5 downwards, as that's the tallest letter I thought of at the time
				Character ell = *(p_graphics->getCharacterInfo('L'));
				Text new_message = { next_text, message_map[mt][index][m].x, message_map[mt][index][m].y - (ell.Size.y * (float)1.5 * scale), scale, message_map[mt][index][m].color, message_map[mt][index][m].x_end }; //keep all the same attributes as original message, except for text and starting y position
				message_map[mt][index].push_back(new_message);

				break;
			}

			float h = ch.Size.y * scale;

			if (ypos < min_y) min_y = ypos;
			if ((ypos + h) > max_y) max_y = (ypos + h);

			temp_x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
	}

	//if max_y is higher than viewing window, need to shift everything down
	if (max_y > (float)p_graphics->getScreenHeight())
	{
		//message starts too high up, lower all sub-messages by the amount of pixels between top of message and top of viewing window
		float shift = max_y + p_graphics->getScreenHeight() + 5; //give a 5 pixel buffer
		for (int i = 0; i < message_map[mt][index].size(); i++) message_map[mt][index][i].y -= shift;
		min_y -= shift;

		if (min_y < 0)
		{
			//make sure that lowering messages didn't lower the bottom past the edge of the viewing window, if so need to recreate original message and start over with smaller font size
			std::string original_string = "";
			for (int i = 0; i < message_map[mt][index].size(); i++) original_string += (message_map[mt][index][i].text + " ");

			Text original_message = { original_string, message_map[mt][index][0].x, message_map[mt][index][0].y + shift, scale * (float)0.9, message_map[mt][index][0].color, message_map[mt][index][0].x_end }; //keep all the same attributes as original message, but reduce font size by 10%

			message_map[mt][index].clear();
			message_map[mt][index].push_back(original_message);
			createSubMessages(mt, index); //function recursively calls itself
		}
	}

	if (min_y < 0)
	{
		//end of message is too low, raise every sub message by the amount of pixels between bottom of screen and bottom of message
		float shift = 0 - min_y + 5; //give a 5 pixel buffer
		for (int i = 0; i < message_map[mt][index].size(); i++) message_map[mt][index][i].y += shift;
		max_y += shift;

		if (max_y > (float)p_graphics->getScreenHeight())
		{
			//make sure that raising messages didn't raise the top past the edge of the viewing window, if so need to recreate original message and start over with smaller font size
			std::string original_string = "";
			for (int i = 0; i < message_map[mt][index].size(); i++) original_string += (message_map[mt][index][i].text + " ");

			Text original_message = { original_string, message_map[mt][index][0].x, message_map[mt][index][0].y - shift, scale * (float)0.9, message_map[mt][index][0].color, message_map[mt][index][0].x_end }; //keep all the same attributes as original message, but reduce font size by 10%

			message_map[mt][index].clear();
			message_map[mt][index].push_back(original_message);
			createSubMessages(mt, index); //function recursively calls itself
		}
	}
}

//Model Based Functions
void Mode::clearAllImages()
{
	//TODO: doesn't seem to be freeing up data like I thought it would, look into making a deconstructor for Model and Mesh classes
	model_map.clear();
	std::vector<Model> blank_model;
	int heyhey = number_of_model_types;
	for (int i = 0; i < heyhey; i++) model_map[modeltypeFromInt(i)] = blank_model;
}