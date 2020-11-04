#pragma once

#include <Header_Files/Modes/mode.h>

//Classes, structs and enums that are defined in other headers
class GL;

class MainMenu : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	MainMenu(GL& graphics) : Mode(graphics)
	{
		mode_name = "Main Menu";
		mode_type = ModeType::MAIN_MENU;
	};

	//Updating and Advancement Functions
	void update();
	void processInput();
	void modeStart();
	void modeEnd();
};