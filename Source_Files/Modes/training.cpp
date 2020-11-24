#include "pch.h"

#include <iostream>
#include <cmath>
#include <string>
#include <Header_Files/gnuplot.h>
#include <Header_Files/Modes/training.h>
#include <Header_Files/quaternion_functions.h> //remove after getting new BLE 33 Sense

//PUBLIC FUNCTIONS
//Updating and Advancement Functions
void Training::update()
{
	processInput(); //process FreeSwing specific input first

	setClubRotation(p_graphics->getOpenGLQuaternion());
	if (training_state == 1) planeTraining();
	else if (training_state == 3) tiltTraining();
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
			training_stage = 0;
			clearAllText();
			clearAllImages();
			initializeText();
			background_color = { .4, 0.282, 0.2 };
			ready_to_swing = false; //will need to re-ready club for a swing after exiting a test
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
		addText(MessageType::BODY, {intro_statement, 20.0, 400.0, 0.5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() - (float) 10.0});
		background_color = { 0.82, 0.706, 0.039 };

		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_3) == GLFW_PRESS)
	{
		if (training_state > 0) return; //key should only function from main menu, if pressed from another part of program shouldn't effect key press timer

		training_state = 3;
		editMessage(MessageType::TITLE, 0, { "Clubface Tilt Training", 190.0, 550.0, 1.0, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		editMessage(MessageType::FOOT_NOTE, 0, { "Press Esc. to return to Training Menu.", 540.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		clearMessageType(MessageType::BODY);
		std::string intro_statement = "    Clubface Tilt Training - The goal of this training is to learn how to hit the ball with the clubface flat relative to the ground. If the hands"
			" are higher or lower than their starting position when the ball is struck then the clubface will be tilted so that either the toe or heel is down. This leads to thin shots and"
			" can even cause the ball to start off in the wrong direction given a perfectly on plane swing. Start off this training by addressing the golf ball. The program will automatically"
			" calculate the tilt of the golf club. From this point take a swing and the program will calculate the difference in club angle at impact. The goal is to have the end club tilt be"
			" within 5 degrees of the starting club tilt. After 5 successful swings the tilt window gets smaller, get as low as you can!";
		addText(MessageType::BODY, { intro_statement, 20.0, 400.0, 0.5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() - (float) 10.0});
		background_color = { 0.835, 0.518, 0.235 };

		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		if (training_state == 1) planeNextStep();
		else if (training_state == 3) tiltNextStep();
	}
}
void Training::modeStart()
{
	//initialize text to render
	initializeText();

	//initialize images to render

	//set up other basic veriables specific to each mode type
	setClubScale({ 1.0, 1.0, 1.0 });
	setClubLocation({ 0.0, 0.0, 0.0 });
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
	addText(MessageType::BODY, { "3. Clubface Tilt", 20.0, 290.0, 0.75, glm::vec3(0.835, 0.518, 0.235), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "4. Distance", 20.0, 245.0, 0.75, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
}

//Advancement Functions
void Training::readySwing()
{
	if (training_stage == 1)
	{
		//wait in training_state 1 until the club is moved to a "legal" location
		//a "legal" location meaning that the club shaft is pointing at the ground with an angle between 10 and 60 degrees
		//this can be gound by looking at the z value of the accelerometer (minus the calculated linear acceleration in that direction
		if (eulerAnglesOk())
		{

			training_stage++;
			start_time = glfwGetTime();
			start_angles[0] = current_angles[0];
			start_angles[1] = current_angles[1];
			start_angles[2] = current_angles[2];
		}
	}
	else if (training_stage == 2)
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
			if (glfwGetTime() - start_time >= start_time_threshold)
			{
				training_stage++; //move onto the next training state once club has been stationary for long enough
				ready_to_swing = true; //set the ready to swing variable to true
			}
		}
	}
}
void Training::planeNextStep()
{
	//this function is called when the Enter key needs to be pressed to advance to the next part of the program
	if (training_stage == 0)
	{
		training_stage++;

		clearMessageType(MessageType::BODY);

		//load golf club model onto screen
		Model club;
		club.loadModel("Resources/Models/Golf_Club/golf_club.obj");
		model_map[ModelType::CLUB].push_back(club);
	}
}
void Training::planeTraining()
{
	if (!ready_to_swing) readySwing();

	if (training_stage == 3)
	{

		//TODO: the purpose of this state is to set up the camera directly behind the user and render the green and red boundary lines

		//Create red and green line objects
		Model plane, plane2;
		plane.loadModel("Resources/Models/Lines/red_line.obj");
		plane2.loadModel("Resources/Models/Lines/green_line.obj");

		//make the rotation of the lines equal to the of the golf club
		glm::quat rot = { cosf((3.14159 / 2.0 + current_angles[1]) / 2.0), 0, 0, sinf((3.14159 / 2.0 + current_angles[1]) / 2.0) }; //add 90 to pitch as lines are rendered vertically by default and not horizontally

		//glm::quat rot = { 1, 0, 0, 0 };
		plane.setRotation(rot);
		plane2.setRotation(rot);

		//set the red line to the left of the golf club and the green line to the right of the club
		plane.setLocation({ -.5, 0, 0 });
		plane2.setLocation({ .5, 0, 0 });

		//can also set scale of lines here if desired
		plane.setScale({ .5, 1.0, 0 });
		plane2.setScale({ .5, 1.0, 1.0 });

		model_map[ModelType::LINE_OBJECTS].push_back(plane); //this is just for a test, delete later
		model_map[ModelType::LINE_OBJECTS].push_back(plane2); //this is just for a test, delete later

		addText(MessageType::BODY, { "Successful Swings: 0", 20.0, 100.0, 0.75, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		successful_swings = 0; //reset the successful_swings variable to 0

		training_stage++;
	}
	else if (training_stage == 4)
	{
		//in this state a swing is carried out. If successful then increment the successful_swings counter by 1
		//if not successful then reset counter to 0
		//go back to training_state 2 regardless of whether or not the swing was successful

		//check for impact of club and lines using the AABB collision detection method
		//if (detectCollision(model_map[ModelType::CLUB][0], model_map[ModelType::LINE_OBJECTS][0])) std::cout << "COLLISION!" << std::endl;
		//else if (detectCollision(model_map[ModelType::CLUB][0], model_map[ModelType::LINE_OBJECTS][1])) std::cout << "COLLISION!" << std::endl;
	}
}
void Training::tiltNextStep()
{
	if (training_stage == 0)
	{
		training_stage++;

		clearMessageType(MessageType::BODY);

		//load golf club model onto screen
		Model club;
		club.loadModel("Resources/Models/Golf_Club/golf_club.obj");
		model_map[ModelType::CLUB].push_back(club);
	}
}
void Training::tiltTraining()
{
	if (!ready_to_swing) readySwing();

	if (training_stage == 3)
	{

		//TODO: the purpose of this state is to set up the camera directly behind the user and render the green and red boundary lines
		clearMessageType(MessageType::BODY); //clear out any body text
		addText(MessageType::BODY, { "Start Pitch Angle: " + std::to_string(start_angles[1] * 180.0 / pi), 20.0, 280.0, 0.5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		successful_swings = 0; //reset the successful_swings variable to 0

		training_stage++;
	}
	else if (training_stage == 4)
	{
		//in this state a swing is carried out. If successful then increment the successful_swings counter by 1
		//if not successful then reset counter to 0
		//go back to training_state 2 regardless of whether or not the swing was successful

		//check to see if yaw angle has changed by at least 10 degrees, if so then it means the swing has begun so move to next stage
		int cs = p_graphics->getBLEDevice()->getCurrentSample();
		current_angles[2] = p_graphics->getBLEDevice()->getData(DataType::EULER_ANGLES, Z)->at(cs);

		//make sure that angle data didn't wrap around (i.e. go from +180 to -180) before calculating current angle difference
		//max reading of the gyroscope is 2000 DPS and refresh rate of the monitor is 60 Hz (.0167 seconds) so the max delta in angle
		//should be about 2000 * .0167 = 33.33 degrees. To be safe scrutinize anything larger than 75 degrees of difference. 75 deg = 1.309 radians

		float yaw_delta = current_angles[2] - start_angles[2];
		if (yaw_delta >= 1.309) yaw_delta -= 6.28318; //subtract 360 degrees to get actual difference
		else if (yaw_delta <= -1.309) yaw_delta += 6.28318; //add 360 degrees to get actual difference

		if (yaw_delta < 0) yaw_delta *= -1; //convert to only positive reading for easier comparison
		if (yaw_delta >= .175) //move to next stage if delta is more than 10 degrees
		{
			training_stage++;
			addText(MessageType::BODY, { "SWING IN PROGRESS", 20.0, 190.0, 0.5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		}
	}
	else if (training_stage == 5)
	{
		int cs = p_graphics->getBLEDevice()->getCurrentSample();
		float last_pitch = current_angles[1], last_yaw = current_angles[2];
		current_angles[1] = p_graphics->getBLEDevice()->getData(DataType::EULER_ANGLES, Y)->at(cs);
		current_angles[2] = p_graphics->getBLEDevice()->getData(DataType::EULER_ANGLES, Z)->at(cs);

		//check to see if current yaw angle is within 1 degree of starting position, if so the swing is complete
		bool swing_complete = false;
		float yaw_delta = current_angles[2] - last_yaw; //again need to check for wrapping of values from +180 to - 180, this value shouldn't really be more than 33 degrees
		if (yaw_delta < 0) yaw_delta *= -1; //convert to positive value
		if (yaw_delta > 1.309) //if greater than 75 degrees the values wrapped
		{
			//if the last value was positive, make the current value positive, if the last value was negative, make the current value negative
			if (last_yaw > 0) current_angles[2] += 6.28318; //add 360 degrees to make positive
			else current_angles[2] -= 6.28318; //subtract 360 degrees to make negative
		}

		//if swinging fast it can be easy to miss the starting point window, look at current angle and last calculated angle to see if starting point has gone by
		//and then extrapolate angle at start point
		if (start_angles[2] <= last_yaw && start_angles[2] >= current_angles[2]) //need to if statements to make sure both positive and negative angles are accounted for
		{
			swing_complete = true;
		}
		else if (start_angles[2] >= last_yaw && start_angles[2] <= current_angles[2])
		{
			swing_complete = true;
		}
		if (swing_complete)
		{
			float final_pitch = (start_angles[2] - last_yaw) / (current_angles[2] - last_yaw) * (current_angles[1] - last_pitch) + last_pitch;
			std::cout << "Last Yaw = " << last_yaw * 180.0 / pi << ", Current Yaw = " << current_angles[2] * 180.0 / pi << std::endl;
			std::cout << "Last Pitch = " << last_pitch * 180.0 / pi << ", Current Pitch = " << current_angles[1] * 180.0 / pi << std::endl;
			editMessage(MessageType::BODY, 1, { "End Pitch Angle: " + std::to_string(final_pitch * 180.0 / pi), 20.0, 250.0, 0.5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
			training_stage = 2;
			ready_to_swing = false;
		}
	}
}
bool Training::eulerAnglesOk()
{
	//currently only looking at pitch angle, to reasonably resemble a golf club setup the pitch should be between -10 and -80 degrees
	//TODO - should also include some limits for roll and yaw angles at some point

	//using radians: -80 degrees = -1.396 radians and -10 degrees = -.175 radians
	getCurrentClubAngles();
	if (current_angles[1] > -1.396 && current_angles[1] < -.175) return true;
	return false;
}

//Data Gathering Functions
void Training::getCurrentClubAngles()
{
	//TODO: BLEchip now calculates Euler Angles so shouldn't need any math in this function, cleanup later
	//This function look at the current rotation quaternion of sensor and gets Euler Angles. Formulas found here: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	glm::quat q = p_graphics->getRotationQuaternion();

	current_angles[0] = atan2f(2 * (q.w * q.x + q.y * q.z), 1 - 2 * (q.x * q.x + q.y * q.y));
	current_angles[1] = asinf(2 * (q.w * q.y - q.x * q.z));
	current_angles[2] = atan2f(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.y * q.y + q.z * q.z));

	//std::cout << "roll = " << current_angles[0] * 180 / 3.14159 << " pitch = " << current_angles[1] * 180 / 3.14159 << " yaw = " << current_angles[2] * 180 / 3.14159 << std::endl;
}

//Physics Functions
//these will be moved to their own header at some point
bool Training::detectCollision(Model a, Model b)
{
	std::vector<glm::vec3> a_box = a.getBoundingBox();
	std::vector<glm::vec3> b_box = b.getBoundingBox();

	return (a_box[0][0] <= b_box[1][0] && a_box[1][0] >= b_box[0][0]) && (a_box[0][1] <= b_box[1][1] && a_box[1][1] >= b_box[0][1]) && (a_box[0][2] <= b_box[1][2] && a_box[1][2] >= b_box[0][2]);
}