#include "pch.h"

#include <iostream>
#include <Header_Files/gnuplot.h>
#include <Header_Files/Modes/training.h>
#include <Header_Files/quaternion_functions.h> //remove after getting new BLE 33 Sense

//PUBLIC FUNCTIONS
//Updating and Advancement Functions
void Training::update()
{
	processInput(); //process FreeSwing specific input first
}
void Training::processInput()
{
	if (!p_graphics->GetCanPressKey()) return; //only process input if input processing is available

	if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		if (training_state == 0)
		{
			modeEnd();
			p_graphics->setCurrentMode(ModeType::MAIN_MENU); //set the new mode
		}
		else
		{
			training_state = 0;
			clearAllText();
			clearAllImages();
			initializeText();
			background_color = { .4, 0.282, 0.2 };
		}
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_1) == GLFW_PRESS)
	{
		if (training_state > 0) return; //key should only function from main menu, if pressed from another part of program shouldn't effect key press timer

		training_state = 1;
		editMessage(MessageType::TITLE, 0, { "Swing Plane Training", 190.0, 550.0, 1.0, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		editMessage(MessageType::FOOT_NOTE, 0, { "Press Esc. to return to Training Menu.", 540.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		clearMessageType(MessageType::BODY);
		std::string intro_statement = "    Swing Plane Training - The goal of this training is to learn how to swing the golf club on the proper plane. An in-to-out swing path can"
			" promote a draw while an out-to-in swing path can promote a fade. To start the training, adress the golf ball and stand still for a second. The program will sense the appropriate"
			" swing plane by looking at the angle of the club at address. From this point, a green line and red line will be rendered on screen parallel to the club shaft. The goal is to"
			" swing the club without touching the green or red lines. If you're succesful 5 times in a row, the distance between the two lines will shrink. Get as many in a row as you can!";
		addText(MessageType::BODY, {intro_statement, 20.0, 400.0, 0.5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		background_color = { 0.82, 0.706, 0.039 };

		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		if (training_state == 1) planeNextStep();
	}
}
void Training::modeStart()
{
	//initialize text to render
	initializeText();

	//initialize images to render

	//set up other basic veriables specific to each mode type
	p_graphics->SetClubMatrices({ 1.0, 1.0, 1.0 }, { 0.0, 0.0, 0.0 });
}
void Training::modeEnd()
{
	//clear out all text to free up space
	clearAllText();

	//clear data from model map to free up space
	clearAllImages();

	//Reset all Bool variables to false
}

//PRIVATE FUNCTIONS
//Setup Functions
void Training::initializeText()
{
	addText(MessageType::TITLE, { "Training Mode", 230.0, 550.0, 1.0, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::SUB_TITLE, { "(Choose one of the below facets of the golf swing to improve)", 70.0, 520.0, .5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::FOOT_NOTE, { "Press Esc. to return to Main Menu.", 560.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });

	addText(MessageType::BODY, { "1. Swing Plane", 20.0, 380.0, 0.75, glm::vec3(0.976, 0.902, 0.475), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "2. Clubface Squareness", 20.0, 335.0, 0.75, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "3. Clubface Tilt", 20.0, 290.0, 0.75, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "4. Distance", 20.0, 245.0, 0.75, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
}

//Advancement Functions
void Training::planeNextStep()
{
	//this function is called when the Enter key needs to be pressed to advance to the next part of the program
	if (training_state == 1)
	{
		training_stage++;
		separate_rotation_matrix = 1; //Remove this after getting new BLE 33 Sense
		mode_q = GetRotationQuaternion({ .423, -.906, 0 }, { 0, 0, 1 });

		clearMessageType(MessageType::BODY);

		Model club, plane, plane2;
		club.loadModel("Resources/Models/Golf_Club/golf_club.obj");
		plane.loadModel("Resources/Models/Lines/red_line.obj");
		plane2.loadModel("Resources/Models/Lines/green_line.obj");

		plane.setLocation({ -.5, 0, 0 });
		plane2.setLocation({ .5, 0, 0 });

		//can also set scale of lines here if desired
		plane.setScale({ .5, 1.0, 0 });
		plane2.setScale({ .5, 1.0, 1.0 });

		model_map[ModelType::CLUB].push_back(club);
		model_map[ModelType::LINE_OBJECTS].push_back(plane); //this is just for a test, delete later
		model_map[ModelType::LINE_OBJECTS].push_back(plane2); //this is just for a test, delete later
	}
}
void Training::planeTraining()
{
	if (training_state == 2)
	{
		//wait in training_state 2 until the club is moved to a "legal" location
		//a "legal" location meaning that the club shaft is pointing at the ground with an angle between 10 and 60 degrees
		//this can be gound by looking at the z value of the accelerometer (minus the calculated linear acceleration in that direction

		if (eulerAnglesOk())
		{
			
			training_state++;
			start_time = glfwGetTime();
			start_angles[0] = current_angles[0];
			start_angles[1] = current_angles[1];
			start_angles[2] = current_angles[2];
		}
	}
	else if (training_state == 3)
	{
		if (!eulerAnglesOk()) training_stage--; //If club goes into bad position before training starts then go back a training stage
		bool restart_timer = 0;

		//current Euler angles were updated in line right above so no need to call again
		//compare current angles vs. start angles to make sure that nothing has gone further than allowable threshold
		if (current_angles[0] - start_angles[0] > start_angle_threshold || current_angles[0] - start_angles[0] < -start_angle_threshold) restart_timer = 1;
		if (current_angles[1] - start_angles[1] > start_angle_threshold || current_angles[1] - start_angles[1] < -start_angle_threshold) restart_timer = 1;
		if (current_angles[2] - start_angles[2] > start_angle_threshold || current_angles[2] - start_angles[2] < -start_angle_threshold) restart_timer = 1;

		if (restart_timer)
		{
			start_time = glfwGetTime();
			start_angles[0] = current_angles[0];
			start_angles[1] = current_angles[1];
			start_angles[2] = current_angles[2];
		}
		else
		{
			if (glfwGetTime() - start_time >= start_time_threshold) training_state++; //move onto the next training state once club has been stationary for long enough
		}
	}
	else if (training_state == 4)
	{
		//the purpose of this state is to set up the camera directly behind the user and render the green and red boundary lines

		//Add a line here to rotate the camera so that it's directly behind the user, this is just in case the user moves to a slightly different location between swings
		//Add code to render the red and green lines

		training_state++;
	}
	else if (training_state == 5)
	{
		//in this state a swing is carried out. If successful then increment the successful_swings counter by 1
		//if not successful then reset counter to 0
		//go back to training_state 2 regardless of whether or not the swing was successful
	}
}
bool Training::eulerAnglesOk()
{
	//pitch should be between the above angles for a reasonable setup
	getCurrentClubAngles();
	if (current_angles[0] > 10 && current_angles[0] < 60) return true;
	//TODO - should also include some limits for roll and yaw angles at some point

	return false;
}

//Data Gathering Functions
void Training::getCurrentClubAngles()
{
	std::cout << "getCurrentClubAngle() function doesn't do anything yet!" << std::endl;
}