#include "pch.h"

#include <iostream>
#include <Header_Files/Modes/main_menu.h>

//PUBLIC FUNCTIONS
//Updating and Advancement Functions
void MainMenu::update()
{
	processInput();
}
void MainMenu::processInput()
{
	if (!p_graphics->GetCanPressKey()) return; //only process input if input processing is available

	if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		std::cout << "Now Exiting Program, thanks for using!" << std::endl;
		p_graphics->setWindowShouldClose();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_E) == GLFW_PRESS)
	{
		std::cout << "E key was pressed from the Main Menu mode!" << std::endl;
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_1) == GLFW_PRESS)
	{
		modeEnd(); //end the current mode
		p_graphics->setCurrentMode(ModeType::FREE);
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_3) == GLFW_PRESS)
	{
		modeEnd(); //end the current mode
		p_graphics->setCurrentMode(ModeType::TRAINING);
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_4) == GLFW_PRESS)
	{
		modeEnd(); //end the current mode
		p_graphics->setCurrentMode(ModeType::CALIBRATION);
		p_graphics->resetKeyTimer();
	}
}
void MainMenu::modeStart()
{
	//initialize text to render
	addText(MessageType::TITLE, { "Golf Chip v1.0", 140.0, 510.0, 1.5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::SUB_TITLE, { "(Press one of the keys listed below to select a mode)", 100.0, 465.0, .5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });

	addText(MessageType::BODY, { "1. Free Swing Mode", 20.0, 380.0, 0.75, glm::vec3( 0.392, 0.592, 0.592 ), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "2. Swing Analysis Mode", 20.0, 335.0, 0.75, glm::vec3(0.58, 0.929, 0.588), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "3. Training Mode", 20.0, 290.0, 0.75, glm::vec3(.71, 0.541, 0.416), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "4. Calibration Mode", 20.0, 245.0, 0.75, glm::vec3(0.5, 0.5, 0.5), p_graphics->getScreenWidth() });

	addText(MessageType::FOOT_NOTE, { "Press Esc. to exit the program", 580.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
}
void MainMenu::modeEnd()
{
	clearAllText();
}