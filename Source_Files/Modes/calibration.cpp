#include "pch.h"

#include <iostream>
#include <Header_Files/Modes/calibration.h>
#include <Header_Files/gnuplot.h>

//PUBLIC FUNCTIONS
//Constructors
Calibration::Calibration(GL& graphics) : Mode(graphics)
{
	mode_name = "Calibration";
	mode_type = ModeType::CALIBRATION;

	clearAllText();
	clearAllImages();

	camera_location = { 0.0, 0.0, -0.5 };
	background_color = { 0.25, 0.25, 0.25 };
};

//Updating and Advancement Functions
void Calibration::update()
{
	if (collecting)
	{
		checkForNextStep();
		return; //disable key presses and mouse clicks while actively collecting data
	}
	processInput();

	//if (cal_mode == 0) calibration_q = p_sensor->getOpenGLQuaternion(); //In calibration select mode render chip normally
}
void Calibration::processInput()
{
	if (!p_graphics->GetCanPressKey()) return; //only process input if input processing is available

	if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		if (cal_mode == 0)
		{
			//quit back to main menu if currently on calibration main menu
			modeEnd(); //end the current mode
			p_graphics->setCurrentMode(ModeType::MAIN_MENU); //set the new mode
			p_graphics->resetKeyTimer();
		}
		else
		{
			//quit to calibration main menu if a test is currently going on
			cal_mode = 0;
			clearAllText();
			initializeText();
			p_graphics->SetClubMatrices({ 10.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 });
			p_graphics->resetKeyTimer();
		}
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_1) == GLFW_PRESS && cal_mode == 0)
	{
		cal_mode = 1; //set this to 1 which represents accelerometer calibration
		cal_stage = 0; //reset stage to 0 in case accelerometer isn't first test being performed

		//clear any previous acceleration cal. data
		ax.clear(); ay.clear(); az.clear();
		memset(acc_cal, 0, sizeof(acc_cal));
		time_data.clear();

		//clear existing text
		clearMessageType(MessageType::SUB_TITLE);
		clearMessageType(MessageType::BODY);
		clearMessageType(MessageType::FOOT_NOTE);

		//add new text
		addText(MessageType::SUB_TITLE, { "Accelerometer Calibration", 250, 520, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		addText(MessageType::BODY, { "A 6-point tumble calibration will be performed on the accelerometer. Press enter when ready to begin and then follow the instructions on screen.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
		addText(MessageType::FOOT_NOTE, { "Press Esc. to return to Calibration Menu", 520.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });

		//move sensor graphic so that text has more room
		p_graphics->SetClubMatrices({ 5.0, 5.0, 5.0 }, { 0.35, -0.15, 0.0 });

		//disable key presses momentarily
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_2) == GLFW_PRESS && cal_mode == 0)
	{
		cal_mode = 2;
		cal_stage = 0;

		//clear any previous gyroscope cal. data
		gx.clear(); gy.clear(); gz.clear();
		memset(gyr_cal, 0, sizeof(gyr_cal));
		time_data.clear();

		//clear existing text
		clearMessageType(MessageType::SUB_TITLE);
		clearMessageType(MessageType::BODY);
		clearMessageType(MessageType::FOOT_NOTE);

		//add new text
		addText(MessageType::SUB_TITLE, { "Gyroscope Calibration", 270, 520, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		addText(MessageType::BODY, { "Two tests will be performed to calibrate the Gyro. First, the zero-offset biases will be calculated by holding sensor steady for 5 seconds. Press Enter.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
		addText(MessageType::FOOT_NOTE, { "Press Esc. to return to Calibration Menu", 520.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });

		//move sensor graphic so that text has more room
		p_graphics->SetClubMatrices({ 5.0, 5.0, 5.0 }, { 0.35, -0.15, 0.0 });

		//disable key presses momentarily
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_3) == GLFW_PRESS && cal_mode == 0)
	{
		cal_mode = 3;
		cal_stage = 0;

		//clear any previous magnetometer cal. data
		mx.clear(); my.clear(); mz.clear();
		memset(mag_max, 0, sizeof(mag_max));
		memset(mag_min, 0, sizeof(mag_min));
		time_data.clear();

		//clear existing text
		clearMessageType(MessageType::SUB_TITLE);
		clearMessageType(MessageType::BODY);
		clearMessageType(MessageType::FOOT_NOTE);

		//add new text
		addText(MessageType::SUB_TITLE, { "Magnetometer Calibration", 270, 520, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		addText(MessageType::BODY, { "To calibrate the magnetometer, both hard and soft iron deposits are accounted for. Take the sensor in your hand and rotate it along all three axes in figure-8 patterns for 10 seconds. Press enter when ready.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
		addText(MessageType::FOOT_NOTE, { "Press Esc. to return to Calibration Menu", 520.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });

		//move sensor graphic so that text has more room
		p_graphics->SetClubMatrices({ 5.0, 5.0, 5.0 }, { 0.35, -0.15, 0.0 });

		//disable key presses momentarily
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		if (cal_mode == 1) accTest();
		else if (cal_mode == 2) gyroTest();
		else if (cal_mode == 3) magTest();

		p_graphics->resetKeyTimer();
	}
}
void Calibration::modeStart()
{
	//initialize text to render
	initializeText();

	cal_mode = 0; //represents calibration select mode
	cal_stage = 0; //start off on cal stage 0 because cal menu comes first

	collecting = 0; //initialize data collection to false

	//initialize images to render
	Model chip;
	chip.loadModel("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Models/Chip/chip.obj");
	model_map[ModelType::CHIP].push_back(chip);

	//set up other basic veriables specific to each mode type
	p_graphics->SetClubMatrices({ 10.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 });
	getCurrentCalibrationNumbers();
}
void Calibration::modeEnd()
{
	//clear all text data out to free up space
	clearAllText();

	//clear data from model map to free up space
	clearAllImages();

	if (changes_made)
	{
		setCalibrationNumbers(); //only update Calibration.txt if any numbers were actually changed
		p_graphics->updateCalibrationNumbers(); //have the sensor read the new numbers from Calibration.txt to update its own cal data
	}
}

//PRIVATE FUNCTIONS
//Setup Functions
void Calibration::initializeText()
{
	addText(MessageType::TITLE, { "Calibration Mode", 200.0f, 550.0f, 1.0, glm::vec3(1.0f, 1.0f, 1.0f), p_graphics->getScreenWidth() });
	addText(MessageType::SUB_TITLE, { "Press: 1 = Accelerometer Cal., 2 = Gyroscope Cal., 3 = Magnetometer Cal.", 10.0f, 520.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), p_graphics->getScreenWidth() });
	addText(MessageType::FOOT_NOTE, { "Press Esc. to return to Main Menu", 560.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
}

//Data Gathering Functions
void Calibration::addRawData(DataType dt, int current_sample)
{
	if (dt == DataType::ACCELERATION)
	{
		ax.push_back(p_graphics->getRawData(dt, X)->at(current_sample));
		ay.push_back(p_graphics->getRawData(dt, Y)->at(current_sample));
		az.push_back(p_graphics->getRawData(dt, Z)->at(current_sample));
	}
	else if (dt == DataType::ROTATION)
	{
		gx.push_back(p_graphics->getRawData(dt, X)->at(current_sample));
		gy.push_back(p_graphics->getRawData(dt, Y)->at(current_sample));
		gz.push_back(p_graphics->getRawData(dt, Z)->at(current_sample));
	}
	else if (dt == DataType::MAGNETIC)
	{
		mx.push_back(p_graphics->getRawData(dt, X)->at(current_sample));
		my.push_back(p_graphics->getRawData(dt, Y)->at(current_sample));
		mz.push_back(p_graphics->getRawData(dt, Z)->at(current_sample));
	}
}

//Advancement Functions
void Calibration::checkForNextStep()
{
	if (cal_mode == 1) accNextStep();
	else if (cal_mode == 2) gyroNextStep();
	else if (cal_mode == 3) magNextStep();
}
void Calibration::accTest()
{
	if (cal_stage == 0)
	{
		clearMessageType(MessageType::BODY);
		addText(MessageType::BODY, { "Lay Sensor flat on table with +Z axis pointing up like shown in the picture to the right. Sensor will accumulate data for 5 seconds. Press enter to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

		mode_q = { 1, 0, 0, 0 };
		cal_stage++;
	}
	else if (cal_stage > 0 && cal_stage < 7)
	{
		clearMessageType(MessageType::BODY);
		addText(MessageType::BODY, { "Collecting Data...", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

		collecting = 1; //signifies that the sensor is collecting data

		if (cal_stage > 1) //no data has been collected yet when cal_stage = 1
		{
			acc_cal[0][cal_stage - 2] /= avg_count; //average out values obtained for each part of the test
			acc_cal[1][cal_stage - 2] /= avg_count; //average out values obtained for each part of the test
			acc_cal[2][cal_stage - 2] /= avg_count; //average out values obtained for each part of the test

			avg_count = 0; //reset the counter for calculating averages
		}
		cal_time = glfwGetTime();
		data_time = glfwGetTime();
		cal_stage++;
	}
	else
	{
		clearAllText();
		initializeText();
		p_graphics->SetClubMatrices({ 10.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 });

		cal_mode = 0; //go back to calibration main menu
	}
}
void Calibration::accNextStep()
{
	if (glfwGetTime() - cal_time >= 5)
	{
		collecting = 0; //Stop data collection momentarily
		clearMessageType(MessageType::BODY);

		if (cal_stage == 7)
		{
			addText(MessageType::BODY, { "All data collection is complete. Here are the new accelerometer calibration numbers", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

			acc_cal[0][cal_stage - 2] /= avg_count;
			acc_cal[1][cal_stage - 2] /= avg_count;
			acc_cal[2][cal_stage - 2] /= avg_count;

			updateCalibrationNumbers();

			addText(MessageType::BODY, { "Accelerometer Offset = [" + std::to_string(acc_off[0][0]) + ", " + std::to_string(acc_off[0][1]) + ", " + std::to_string(acc_off[0][2]) + "]", 10, 330, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::BODY, { "                                       [" + std::to_string(acc_off[1][0]) + ", " + std::to_string(acc_off[1][1]) + ", " + std::to_string(acc_off[1][2]) + "]", 10, 300, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::BODY, { "                                       [" + std::to_string(acc_off[2][0]) + ", " + std::to_string(acc_off[2][1]) + ", " + std::to_string(acc_off[2][2]) + "]", 10, 270, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::BODY, { "Accelerometer Gain = [" + std::to_string(acc_gain[0][0]) + ", " + std::to_string(acc_gain[0][1]) + ", " + std::to_string(acc_gain[0][2]) + "]", 10, 210, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::BODY, { "                                     [" + std::to_string(acc_gain[1][0]) + ", " + std::to_string(acc_gain[1][1]) + ", " + std::to_string(acc_gain[1][2]) + "]", 10, 180, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::BODY, { "                                     [" + std::to_string(acc_gain[2][0]) + ", " + std::to_string(acc_gain[2][1]) + ", " + std::to_string(acc_gain[2][2]) + "]", 10, 150, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::BODY, { "Press enter to go back to calibration main menu.", 10, 90, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		}
		else
		{
			//tumble order should be [+Z, -Y, +X, +Y, -X, -Z]
			if (cal_stage == 2)
			{
				addText(MessageType::BODY, { "Data collection complete. Rotate sensor so that the positive Y-axis is pointing down as shown in the image. Press enter when ready to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				mode_q = { .707, 0, 0, -.707 }; //changes orientation of chip for rendering purposes
			}
			else if (cal_stage == 3)
			{
				addText(MessageType::BODY, { "Data collection complete. Rotate sensor 90 degrees so that the positive X-axis is pointing up as shown in the image. Press enter when ready to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				mode_q = { .707, -.707, 0, 0 }; //changes orientation of chip for rendering purposes
			}
			else if (cal_stage == 4)
			{
				addText(MessageType::BODY, { "Data collection complete. Rotate sensor 90 degrees so that the positive Y-axis is pointing up as shown in the image. Press enter when ready to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				mode_q = { .707, 0, 0, .707 }; //changes orientation of chip for rendering purposes
			}
			else if (cal_stage == 5)
			{
				addText(MessageType::BODY, { "Data collection complete. Rotate sensor 90 degrees so that the positive X-axis is pointing down as shown in the image. Press enter when ready to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				mode_q = { .707, .707, 0, 0 }; //changes orientation of chip for rendering purposes
			}
			else
			{
				addText(MessageType::BODY, { "Data collection complete. Rotate sensor 90 degrees so that the positive Z-axis is pointing down as shown in the image. Press enter when ready to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				mode_q = { 0, 0, 0, 1 }; //changes orientation of chip for rendering purposes
			}
		}
	}
	else
	{
		//add current raw acceleration values to appropriate index of acc_cal array, also add readings to overall reading vector
		addRawData(DataType::ACCELERATION, p_graphics->getCurrentSample());
		acc_cal[0][cal_stage - 2] += ax.back();
		acc_cal[1][cal_stage - 2] += ay.back();
		acc_cal[2][cal_stage - 2] += az.back();
		avg_count++;

		if (time_data.size() == 0) time_data.push_back(glfwGetTime() - data_time);
		else time_data.push_back(glfwGetTime() - data_time + time_data.back());
		data_time = glfwGetTime();
	}
}
void Calibration::gyroTest()
{
	if (cal_stage == 0)
	{
		clearMessageType(MessageType::BODY);
		addText(MessageType::BODY, { "Collecting Data...", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

		collecting = 1; //signifies that the sensor is collecting data

		cal_time = glfwGetTime();
		data_time = glfwGetTime();

		cal_stage++;
	}
	else if (cal_stage == 1)
	{
		clearMessageType(MessageType::BODY);

		//clear out data from the previous gyro test
		gx.clear(); gy.clear(); gz.clear();
		time_data.clear();

		//Render new text
		addText(MessageType::BODY, { "Next, the Gyroscope gain values will be calculated. This is acheived by rotating the sensor by a set angle over a set time period. When ready, place the sensor flat on the table. As soon as you hit the Enter key, you will have 5 seconds to rotate the sensor counter-clockwise by 90 degrees. Press Enter to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
		cal_stage++;
	}
	else if (cal_stage == 2)
	{
		collecting = 1;

		cal_time = glfwGetTime();
		data_time = glfwGetTime();

		cal_stage++;
	}
	else if (cal_stage == 4 || cal_stage == 5)
	{
		clearMessageType(MessageType::BODY);

		if (cal_stage == 4) addText(MessageType::BODY, { "Rotate toward you for: 5.00 seconds", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
		else if (cal_stage == 5) addText(MessageType::BODY, { "Rotate left for: 5.00 seconds", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

		collecting = 1;

		cal_time = glfwGetTime();
		data_time = glfwGetTime();

		cal_stage++;
	}
	else if (cal_stage == 6)
	{
		clearMessageType(MessageType::BODY);
		clearMessageType(MessageType::SUB_TITLE);
		addText(MessageType::SUB_TITLE, { "Press: 1 = Accelerometer Cal., 2 = Gyroscope Cal., 3 = Magnetometer Cal.", 10.0f, 520.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), p_graphics->getScreenWidth() });

		p_graphics->SetClubMatrices({ 10.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 });

		cal_mode = 0;
	}
}
void Calibration::gyroNextStep()
{
	if (cal_stage == 1)
	{
		if (glfwGetTime() - cal_time >= 5)
		{
			collecting = 0;
			clearMessageType(MessageType::BODY);

			gyr_off[0] = 0; gyr_off[1] = 0; gyr_off[2] = 0;
			for (int i = 0; i < gx.size(); i++)
			{
				gyr_off[0] += gx[i];
				gyr_off[1] += gy[i];
				gyr_off[2] += gz[i];
			}
			gyr_off[0] /= gx.size(); gyr_off[1] /= gy.size(); gyr_off[2] /= gz.size();

			addText(MessageType::BODY, { "Data collection is complete. Here are the new gyroscope offset calibration numbers", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
			addText(MessageType::BODY, { "Gyroscope Offset = [" + std::to_string(gyr_off[0]) + ", " + std::to_string(gyr_off[1]) + ", " + std::to_string(gyr_off[2]) + "]", 10, 330, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::BODY, { "Press enter to go to the next calibration step.", 10, 270, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		}
		else
		{
			//add current gyroscope values to appropriate index of gyr_cal array, also add readings to overall reading vector
			addRawData(DataType::ROTATION, p_graphics->getCurrentSample());

			//record time stamps to potentially display graphs in the future
			time_data.push_back(p_graphics->getCurrentTime());
			data_time = glfwGetTime();
		}
	}
	else if (cal_stage == 3)
	{
		clearMessageType(MessageType::BODY);
		addText(MessageType::BODY, { "Rotate clockwise for: 5.00 seconds", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

		cal_stage++;
	}
	else if (cal_stage >= 4)
	{
		if (glfwGetTime() - cal_time >= 5)
		{
			collecting = 0;
			clearMessageType(MessageType::BODY);
			//Integrate data to see measured angle of rotation
			if (cal_stage == 4)
			{
				addText(MessageType::BODY, { "Data gathered succesfully. Now the X-axis gain must be calibrated. Put the sesnor back to its original position, then rotate it 90 degrees toward you so that it's standing straight up. Press enter.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				gyr_cal[2] = 0; //reset to 0 incase this is not the first calibration attempt
				for (int i = 1; i < gz.size(); i++) gyr_cal[2] += integrateData(gz[i], gz[i - 1], time_data[i] - time_data[i - 1]);
				time_data.clear();
			}
			else if (cal_stage == 5)
			{
				addText(MessageType::BODY, { "Data gathered succesfully. Now the Y-axis gain must be calibrated. Lay the sensor back on the table, then rotate it 90 degrees onto its left side. Press enter to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				gyr_cal[0] = 0; //reset to 0 incase this is not the first calibration attempt
				for (int i = 1; i < gx.size(); i++) gyr_cal[0] += integrateData(gx[i], gx[i - 1], time_data[i] - time_data[i - 1]);
				time_data.clear();
			}
			else if (cal_stage == 6)
			{
				gyr_cal[1] = 0; //reset to 0 incase this is not the first calibration attempt
				for (int i = 1; i < gy.size(); i++) gyr_cal[1] += integrateData(gy[i], gy[i - 1], time_data[i] - time_data[i - 1]);
				addText(MessageType::BODY, { "Data collection is complete. Here are the new gyroscope gain calibration numbers:", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				addText(MessageType::BODY, { "Gyroscope Gain = [" + std::to_string(gyr_gain[0]) + ", " + std::to_string(gyr_gain[1]) + ", " + std::to_string(gyr_gain[2]) + "]", 10, 330, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
				addText(MessageType::BODY, { "Press enter to go back to calibration menu.", 10, 270, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
				updateCalibrationNumbers();
			}

			gx.clear(); gy.clear(); gz.clear(); //clear out data from the previous gyro test
			time_data.clear();
		}
		else
		{
			clearMessageType(MessageType::BODY);
			double time_left = 5 - (glfwGetTime() - cal_time);

			if (cal_stage == 4) addText(MessageType::BODY, { "Rotate clockwise for: " + std::to_string(time_left) + " seconds", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
			else if (cal_stage == 5) addText(MessageType::BODY, { "Rotate toward you for: " + std::to_string(time_left) + " seconds", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
			else if (cal_stage == 6) addText(MessageType::BODY, { "Rotate left for: " + std::to_string(time_left) + " seconds", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

			//make sure to apply offset values obtained from first portion of gyroscope calibration
			//don't use addRawData() function here because it's necessary to subtract new offset values from raw data, which that function doesn't do
			int cs = p_graphics->getCurrentSample();
			gx.push_back(p_graphics->getRawData(DataType::ROTATION, X)->at(cs) - gyr_off[0]); gy.push_back(p_graphics->getRawData(DataType::ROTATION, Y)->at(cs) - gyr_off[1]); gz.push_back(p_graphics->getRawData(DataType::ROTATION, Z)->at(cs) - gyr_off[2]); //push_back raw sensor data

			//record time stamps so that data can be integrated
			time_data.push_back(p_graphics->getCurrentTime());
		}
	}
}
void Calibration::magTest()
{
	if (cal_stage == 0)
	{
		clearMessageType(MessageType::BODY);
		addText(MessageType::BODY, { "Collecting Data for 20 more seconds", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

		collecting = 1; //signifies that the sensor is collecting data

		cal_time = glfwGetTime();
		data_time = glfwGetTime();

		cal_stage++;

		//initialize mag_max and mag_min values to both be equal to the current mag reading
		//need to do this becuase there's a chance the maximum value should actually be less than 0 (-22 for example), or minimum value could be greater than 0 (+22)
		int cs = p_graphics->getCurrentSample();
		mag_max[0] = p_graphics->getRawData(DataType::MAGNETIC, X)->at(cs); mag_max[1] = p_graphics->getRawData(DataType::MAGNETIC, Y)->at(cs); mag_max[2] = p_graphics->getRawData(DataType::MAGNETIC, Z)->at(cs);
		mag_min[0] = mag_max[0]; mag_min[1] = mag_max[1]; mag_min[2] = mag_max[2];
	}
	else if (cal_stage == 1)
	{
		clearMessageType(MessageType::BODY);

		MagGraph();
		updateCalibrationNumbers();

		addText(MessageType::BODY, { "Another graph will pop-up with calibrated data. Press enter to see the new graph. Close when done.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

		cal_stage++;
	}
	else if (cal_stage == 2)
	{
		clearMessageType(MessageType::BODY);

		MagGraph();

		addText(MessageType::BODY, { "Data collection is complete. Here are the new magnetometer calibration numbers:", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });
		addText(MessageType::BODY, { "Magnetometer Offset = [" + std::to_string(mag_off[0]) + ", " + std::to_string(mag_off[1]) + ", " + std::to_string(mag_off[2]) + "]", 10, 330, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		addText(MessageType::BODY, { "Magnetometer Gain = [" + std::to_string(mag_gain[0]) + ", " + std::to_string(mag_gain[1]) + ", " + std::to_string(mag_gain[2]) + "]", 10, 300, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		addText(MessageType::BODY, { "Press enter to go back to calibration menu.", 10, 240, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

		cal_stage++;
	}
	else if (cal_stage == 3)
	{
		clearMessageType(MessageType::SUB_TITLE);
		clearMessageType(MessageType::BODY);

		addText(MessageType::SUB_TITLE, { "Press: 1 = Accelerometer Cal., 2 = Gyroscope Cal., 3 = Magnetometer Cal.", 10.0f, 520.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), p_graphics->getScreenWidth() });

		p_graphics->SetClubMatrices({ 10.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 });

		cal_mode = 0;
	}
}
void Calibration::magNextStep()
{
	if (cal_stage == 1)
	{
		if (glfwGetTime() - cal_time >= 20)
		{
			collecting = 0;
			clearMessageType(MessageType::BODY);

			addText(MessageType::BODY, { "Data collection is complete. To help visualize the data, press Enter to see a graph. There should be three distinct ovals, that overlap. If the ovals are offset from eachother, then calibration was necessary. Close the graph to continue.", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		}
		else
		{
			clearMessageType(MessageType::BODY);

			double time_left = 20 - (glfwGetTime() - cal_time);
			addText(MessageType::BODY, { "Collecting Data for " + std::to_string(time_left) + " more seconds", 10, 420, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() / (float)2.0 });

			//add current raw magnetometer values to magnetometer data vectors
			addRawData(DataType::MAGNETIC, p_graphics->getCurrentSample());

			//see if any new highs or lows are found
			if (mx.back() > mag_max[0]) mag_max[0] = mx.back();
			if (mx.back() < mag_min[0]) mag_min[0] = mx.back();

			if (my.back() > mag_max[1]) mag_max[1] = my.back();
			if (my.back() < mag_min[1]) mag_min[1] = my.back();

			if (mz.back() > mag_max[2]) mag_max[2] = mz.back();
			if (mz.back() < mag_min[2]) mag_min[2] = mz.back();

			//record time stamps to potentially display graphs in the future
			if (time_data.size() == 0) time_data.push_back(glfwGetTime() - data_time);
			else time_data.push_back(glfwGetTime() - data_time + time_data.back());
			data_time = glfwGetTime();
		}
	}
}
void Calibration::MagGraph()
{
	if (mx.size() > my.size() || mx.size() < my.size())
	{
		std::cout << "Data sets must be the same length to graph them." << std::endl;
		return;
	}
	if (mx.size() > mz.size() || mx.size() < mz.size())
	{
		std::cout << "Data sets must be the same length to graph them." << std::endl;
		return;
	}

	//write all data to file
	ofstream myFile;
	myFile.open("mag.dat");
	for (int i = 0; i < mx.size(); i++) myFile << mx[i] << "    " << my[i] << "    " << mz[i] << '\n';
	myFile.close();

	//plot order is x (uses y and z data), y (uses x and z data), z (uses x and y data)
	std::string function = "plot 'mag.dat' using 2:3, 'mag.dat' using 1:3, 'mag.dat' using 1:2";

	Gnuplot plot;
	plot(function);
}

//Calibration Data Functions
void Calibration::getCurrentCalibrationNumbers()
{
	//this function is necessary so that if only one test is carried out, the variables for the other sensors aren't reset to 0 but back to their current values

	int line_count = 0;
	std::fstream inFile;
	inFile.open("Resources/calibration.txt");
	char name[256];

	///TODO: put in some kind of error here if file wasn't opened properly

	while (!inFile.eof())
	{
		inFile.getline(name, 256);
		///TODO: this if statement is crappy, code up something better later
		if (line_count == 0)      acc_off[0][0] = std::stof(name);
		else if (line_count == 1) acc_off[0][1] = std::stof(name);
		else if (line_count == 2) acc_off[0][2] = std::stof(name);
		else if (line_count == 3) acc_off[1][0] = std::stof(name);
		else if (line_count == 4) acc_off[1][1] = std::stof(name);
		else if (line_count == 5) acc_off[1][2] = std::stof(name);
		else if (line_count == 6) acc_off[2][0] = std::stof(name);
		else if (line_count == 7) acc_off[2][1] = std::stof(name);
		else if (line_count == 8) acc_off[2][2] = std::stof(name);

		else if (line_count == 9) acc_gain[0][0] = std::stof(name);
		else if (line_count == 10) acc_gain[0][1] = std::stof(name);
		else if (line_count == 11) acc_gain[0][2] = std::stof(name);
		else if (line_count == 12) acc_gain[1][0] = std::stof(name);
		else if (line_count == 13) acc_gain[1][1] = std::stof(name);
		else if (line_count == 14) acc_gain[1][2] = std::stof(name);
		else if (line_count == 15) acc_gain[2][0] = std::stof(name);
		else if (line_count == 16) acc_gain[2][1] = std::stof(name);
		else if (line_count == 17) acc_gain[2][2] = std::stof(name);

		else if (line_count == 18) gyr_off[0] = std::stof(name);
		else if (line_count == 19) gyr_off[1] = std::stof(name);
		else if (line_count == 20) gyr_off[2] = std::stof(name);

		else if (line_count == 21) gyr_gain[0] = std::stof(name);
		else if (line_count == 22) gyr_gain[1] = std::stof(name);
		else if (line_count == 23) gyr_gain[2] = std::stof(name);

		else if (line_count == 24) mag_off[0] = std::stof(name);
		else if (line_count == 25) mag_off[1] = std::stof(name);
		else if (line_count == 26) mag_off[2] = std::stof(name);

		else if (line_count == 27) mag_gain[0] = std::stof(name);
		else if (line_count == 28) mag_gain[1] = std::stof(name);
		else if (line_count == 29) mag_gain[2] = std::stof(name);

		line_count++;
	}

	if (line_count < 30) std::cout << "Some calibration information wasn't updated." << std::endl;

	inFile.close();
}
void Calibration::updateCalibrationNumbers()
{
	changes_made = 1; //this will flag the code to update Calibration.txt
	if (cal_mode == 1) //update acc calibration numbers
	{
		//Order of tumble point text is [+Z, -Y, -X, +Y, +X, -Z]
		acc_off[0][0] = (acc_cal[0][2] + acc_cal[0][4]) / 2.0;
		acc_off[1][1] = (acc_cal[1][1] + acc_cal[1][3]) / 2.0;
		acc_off[2][2] = (acc_cal[2][0] + acc_cal[2][5]) / 2.0;

		//Calculate Gain Matrix
		acc_gain[0][0] = (acc_cal[0][4] - acc_cal[0][2]) / (2 * gravity);
		acc_gain[1][0] = (acc_cal[1][4] - acc_cal[1][2]) / (2 * gravity);
		acc_gain[2][0] = (acc_cal[2][4] - acc_cal[2][2]) / (2 * gravity);

		acc_gain[0][1] = (acc_cal[0][3] - acc_cal[0][1]) / (2 * gravity);
		acc_gain[1][1] = (acc_cal[1][3] - acc_cal[1][1]) / (2 * gravity);
		acc_gain[2][1] = (acc_cal[2][3] - acc_cal[2][1]) / (2 * gravity);

		acc_gain[0][2] = (acc_cal[0][0] - acc_cal[0][5]) / (2 * gravity);
		acc_gain[1][2] = (acc_cal[1][0] - acc_cal[1][5]) / (2 * gravity);
		acc_gain[2][2] = (acc_cal[2][0] - acc_cal[2][5]) / (2 * gravity);

		//Convert Gain Matrix to it's own inverse
		invertAccMatrix();
	}
	else if (cal_mode == 2)
	{
		gyr_gain[0] = 90.0 / gyr_cal[0];
		gyr_gain[1] = 90.0 / gyr_cal[1];
		gyr_gain[2] = 90.0 / gyr_cal[2];
	}
	else if (cal_mode == 3)
	{

		mag_off[0] = (mag_max[0] + mag_min[0]) / 2.0;
		mag_off[1] = (mag_max[1] + mag_min[1]) / 2.0;
		mag_off[2] = (mag_max[2] + mag_min[2]) / 2.0;

		mag_max[0] -= mag_off[0]; mag_max[1] -= mag_off[1]; mag_max[2] -= mag_off[2]; //update max readings for gain calculation

		//correct soft iron
		for (int i = 0; i < mx.size(); i++)
		{
			mx[i] -= mag_off[0];
			my[i] -= mag_off[1];
			mz[i] -= mag_off[2];
		}

		float mag_magnitude = 51.3058; //TODO - ultimately need to hard code a way to look up current location and get this info
		mag_gain[0] = mag_magnitude / mag_max[0];
		mag_gain[1] = mag_magnitude / mag_max[1];
		mag_gain[2] = mag_magnitude / mag_max[2];

		//correct hard iron so second mag graph looks correct
		for (int i = 0; i < mx.size(); i++)
		{
			mx[i] *= mag_gain[0];
			my[i] *= mag_gain[1];
			mz[i] *= mag_gain[2];
		}
	}
}
void Calibration::setCalibrationNumbers()
{
	std::ofstream myFile;
	myFile.open("Resources/calibration.txt");

	///TODO: put in some kind of error here if file wasn't opened properly

	myFile << acc_off[0][0] << '\n';
	myFile << acc_off[0][1] << '\n';
	myFile << acc_off[0][2] << '\n';
	myFile << acc_off[1][0] << '\n';
	myFile << acc_off[1][1] << '\n';
	myFile << acc_off[1][2] << '\n';
	myFile << acc_off[2][0] << '\n';
	myFile << acc_off[2][1] << '\n';
	myFile << acc_off[2][2] << '\n';

	myFile << acc_gain[0][0] << '\n';
	myFile << acc_gain[0][1] << '\n';
	myFile << acc_gain[0][2] << '\n';
	myFile << acc_gain[1][0] << '\n';
	myFile << acc_gain[1][1] << '\n';
	myFile << acc_gain[1][2] << '\n';
	myFile << acc_gain[2][0] << '\n';
	myFile << acc_gain[2][1] << '\n';
	myFile << acc_gain[2][2] << '\n';

	myFile << gyr_off[0] << '\n';
	myFile << gyr_off[1] << '\n';
	myFile << gyr_off[2] << '\n';

	myFile << gyr_gain[0] << '\n';
	myFile << gyr_gain[1] << '\n';
	myFile << gyr_gain[2] << '\n';

	myFile << mag_off[0] << '\n';
	myFile << mag_off[1] << '\n';
	myFile << mag_off[2] << '\n';

	myFile << mag_gain[0] << '\n';
	myFile << mag_gain[1] << '\n';
	myFile << mag_gain[2];

	myFile.close();
}

//Utilization Functions
void Calibration::invertAccMatrix()
{
	//inverts the calculated Acc Gain matrix
	float determinant = acc_gain[0][0] * (acc_gain[1][1] * acc_gain[2][2] - acc_gain[1][2] * acc_gain[2][1]) - acc_gain[0][1] * (acc_gain[1][0] * acc_gain[2][2] - acc_gain[1][2] * acc_gain[2][0]) + acc_gain[0][2] * (acc_gain[1][0] * acc_gain[2][1] - acc_gain[1][1] * acc_gain[2][0]);
	for (int i = 0; i < 3; i++)
	{
		//Transpose the original matrix
		for (int j = i + 1; j < 3; j++)
		{
			float temp = acc_gain[i][j];
			acc_gain[i][j] = acc_gain[j][i];
			acc_gain[j][i] = temp;
		}
	}

	//Get determinants of 2x2 minor matrices
	float new_nums[3][3];
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int row1, row2, col1, col2;
			if (i == 0)
			{
				row1 = 1; row2 = 2;
			}
			if (i == 1)
			{
				row1 = 0; row2 = 2;
			}
			if (i == 2)
			{
				row1 = 0; row2 = 1;
			}
			if (j == 0)
			{
				col1 = 1; col2 = 2;
			}
			if (j == 1)
			{
				col1 = 0; col2 = 2;
			}
			if (j == 2)
			{
				col1 = 0; col2 = 1;
			}
			float num = acc_gain[row1][col1] * acc_gain[row2][col2] - acc_gain[row1][col2] * acc_gain[row2][col1];
			if ((i + j) % 2 == 1) num *= -1;
			new_nums[i][j] = num;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			acc_gain[i][j] = new_nums[i][j] / determinant;
		}
	}
}
float Calibration::integrateData(float& p1, float& p2, float t)
{
	return t * ((p1 + p2) / 2);
}