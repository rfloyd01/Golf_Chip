#include "pch.h"

#include <iostream>
#include <Header_Files/gnuplot.h>
#include <Header_Files/Modes/free_swing.h>

//PUBLIC FUNCTIONS
//Updating and Advancement Functions
void FreeSwing::update()
{
	processInput(); //process FreeSwing specific input first
	Mode::processInput(); //process generic input second

	if (display_data) liveUpdate(); //displays current sensor data on screen if the 'R' key has been pressed
	if (record_data) addGraphData();

	setClubRotation(p_graphics->getOpenGLQuaternion());
}
void FreeSwing::processInput()
{
	if (!p_graphics->GetCanPressKey()) return; //only process input if input processing is available

	if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_C) == GLFW_PRESS)
	{
		//show and hide current sensor readings
		if (display_data)
		{
			display_data = 0; //stop displaying data
			if (record_data)
			{
				record_data = 0;
				displayGraph(); //show whatever data has already been recorded
			}
			clearMessageType(MessageType::SENSOR_INFO);
			clearMessageType(MessageType::FOOT_NOTE);
			addText(MessageType::FOOT_NOTE, { "Press Esc. to return to Main Menu.", 560.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
			addText(MessageType::FOOT_NOTE, { "Press 'C' to see live data stream.", 575.0, 28.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
		}
		else
		{
			display_data = 1;
			editMessage(MessageType::FOOT_NOTE, 1, { "Press 'C' to hide live data stream.", 568.0, 28.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
			addText(MessageType::FOOT_NOTE, { "Press 'R' to record currently selected data set.", 480.0, 46.0, 0.33, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::SENSOR_INFO, { "", 10, 105, 0.5, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::SENSOR_INFO, { "", 10, 80, 0.5, {1.0, 0.0, 0.0}, p_graphics->getScreenWidth() });
			addText(MessageType::SENSOR_INFO, { "", 10, 55, 0.5, {0.0, 1.0, 0.0}, p_graphics->getScreenWidth() });
			addText(MessageType::SENSOR_INFO, { "", 10, 30, 0.5, {0.0, 0.0, 1.0}, p_graphics->getScreenWidth() });
			addText(MessageType::SENSOR_INFO, { "Press num. keys 1-7 to cycle through data types.", 10, 10, 0.33, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
		}
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_R) == GLFW_PRESS)
	{
		//pressing R is used for recording the currently selected data set, pressing R a second time or closing data display will cause recording to stop and then display a graph
		if (display_data)
		{
			if (!record_data)
			{
				record_data = 1;

				//clear out any previously recorded data
				data_set.clear(); time_set.clear();
				makeVec(data_set, 3);
				editMessage(MessageType::FOOT_NOTE, 2, { "Press 'R' to stop recording.", 610.0, 46.0, 0.33, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
			}
			else
			{
				record_data = 0;
				editMessage(MessageType::FOOT_NOTE, 2, { "Press 'R' to record currently selected data set.", 480.0, 46.0, 0.33, {1.0, 1.0, 1.0}, p_graphics->getScreenWidth() });
				displayGraph();
				p_graphics->resetTime();
			}
		}
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_1) == GLFW_PRESS)
	{
		//switches live data stream to acceleration values if currently being displayed
		current_data_type = DataType::ACCELERATION;
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_2) == GLFW_PRESS)
	{
		//switches live data stream to gyroscope values if currently being displayed
		current_data_type = DataType::ROTATION;
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_3) == GLFW_PRESS)
	{
		//switches live data stream to magnetometer values if currently being displayed
		current_data_type = DataType::MAGNETIC;
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_4) == GLFW_PRESS)
	{
		//switches live data stream to linear acceleration values if currently being displayed
		current_data_type = DataType::LINEAR_ACCELERATION;
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_5) == GLFW_PRESS)
	{
		//switches live data stream to velocity values if currently being displayed
		current_data_type = DataType::VELOCITY;
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_6) == GLFW_PRESS)
	{
		//switches live data stream to location values if currently being displayed
		current_data_type = DataType::LOCATION;
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_7) == GLFW_PRESS)
	{
		//switches live data stream to location values if currently being displayed
		current_data_type = DataType::EULER_ANGLES;
		p_graphics->resetKeyTimer();
	}
}
void FreeSwing::modeStart()
{
	//initialize text to render
	addText(MessageType::TITLE, { "Free Swing Mode", 200.0, 550.0, 1.0, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::SUB_TITLE, { "(Swing golf club to see movement on screen)", 150.0, 520.0, .5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::FOOT_NOTE, { "Press Esc. to return to Main Menu.", 560.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::FOOT_NOTE, { "Press 'C' to see live data stream.", 575.0, 28.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });

	//initialize images to render
	Model club;
	club.loadModel("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Models/Golf_Club/golf_club.obj");
	model_map[ModelType::CLUB].push_back(club);

	//set up other basic veriables specific to each mode type
	//p_graphics->SetClubMatrices({ 1.0, 1.0, 1.0 }, { 0.0, 0.0, 0.0 });
	setClubScale({ 1.0, 1.0, 1.0 });
	setClubLocation({ 0.0, 0.0, 0.0 });
}
void FreeSwing::modeEnd()
{
	//clear out all text to free up space
	clearAllText();

	//clear data from model map to free up space
	clearAllImages();

	//Reset all Bool variables to false
	display_data = 0;
	record_data = 0;
}

//PRIVATE FUNCTIONS
//Graph Related Functions
void FreeSwing::displayGraph()
{
	if (data_set[0].size() != time_set.size())
	{
		std::cout << "Data sets must be the same length to graph them." << std::endl;
		return;
	}

	//write all data to file
	ofstream myFile;
	myFile.open("data.dat");
	for (int i = 0; i < data_set[0].size(); i++) myFile << time_set[i] << "    " << data_set[0][i] << "    " << data_set[1][i] << "    " << data_set[2][i] << '\n';
	myFile.close();

	std::string function = "plot 'data.dat' using 1:2 title 'x' with lines, 'data.dat' using 1:3 title 'y' with lines, 'data.dat' using 1:4 title 'z' with lines";
	std::string graph_title; std::string y_label;

	if (current_data_type == DataType::ACCELERATION)
	{
		graph_title = "'Acceleration vs. Time'";
		y_label = "'Acceleration (m/s^2)'";
	}
	else if (current_data_type == DataType::ROTATION)
	{
		graph_title = "'Angular Velocity vs. Time'";
		y_label = "'Angular Velocity (deg/s)'";
	}
	else if (current_data_type == DataType::MAGNETIC)
	{
		graph_title = "'Magnetic Field vs. Time'";
		y_label = "'Mag. Field (uT)'";
	}
	else if (current_data_type == DataType::LINEAR_ACCELERATION)
	{
		graph_title = "'Linear Acceleration vs. Time'";
		y_label = "'Acceleration (m/s^2)'";
	}
	else if (current_data_type == DataType::VELOCITY)
	{
		graph_title = "'Velocity vs. Time'";
		y_label = "'Velocity (m/s)'";
	}
	else if (current_data_type == DataType::LOCATION)
	{
		graph_title = "'Location vs. Time'";
		y_label = "'Position (m)'";
	}
	else if (current_data_type == DataType::EULER_ANGLES)
	{
		graph_title = "'Orientation vs. Time'";
		y_label = "'Angle (deg)'";
	}

	Gnuplot graph;
	graph("set title " + graph_title);
	graph("set xlabel 'Time (s)'");
	graph("set ylabel " + y_label);
	graph(function);
}
void FreeSwing::addGraphData()
{
	//pointers to sensor data are set in liveUpdate() function
	//this is ok because graph can only be displayed if sensor data is being displayed
	int cs = p_graphics->getCurrentSample(); //consider making cs a class variable so it doesn't have to be set in every function that needs it
	data_set[0].push_back(p_data_x->at(cs));
	data_set[1].push_back(p_data_y->at(cs));
	data_set[2].push_back(p_data_z->at(cs));
	time_set.push_back(p_graphics->getCurrentTime());

	//easier to read degrees than radians so if current mode is euler angles, convert to degrees from radians
	if (current_data_type == DataType::EULER_ANGLES)
	{
		data_set[0][data_set[0].size() - 1] *= (180.0 / 3.14159);
		data_set[1][data_set[0].size() - 1] *= (180.0 / 3.14159);
		data_set[2][data_set[0].size() - 1] *= (180.0 / 3.14159);
	}
}

//Data Functions
void FreeSwing::liveUpdate()
{

	int cs = p_graphics->getCurrentSample();
	std::string st1, st2, st3;
	if (current_data_type == DataType::ACCELERATION)
	{
		st1 = "Ax =  m/s^2"; st2 = "Ay =  m/s^2"; st3 = "Az =  m/s^2";
		editMessageText(MessageType::SENSOR_INFO, 0, "Accelerometer Readings");
	}
	else if (current_data_type == DataType::ROTATION)
	{
		st1 = "Gx =  deg/s"; st2 = "Gy =  deg/s"; st3 = "Gz =  deg/s";
		editMessageText(MessageType::SENSOR_INFO, 0, "Gyroscope Readings");
	}
	else if (current_data_type == DataType::MAGNETIC)
	{
		st1 = "Mx =  uT"; st2 = "My =  uT"; st3 = "Mz =  uT";
		editMessageText(MessageType::SENSOR_INFO, 0, "Magnetometer Readings");
	}
	else if (current_data_type == DataType::LINEAR_ACCELERATION)
	{
		st1 = "Ax =  m/s^2"; st2 = "Ay =  m/s^2"; st3 = "Az =  m/s^2";
		editMessageText(MessageType::SENSOR_INFO, 0, "Linear Acceleration");
	}
	else if (current_data_type == DataType::VELOCITY)
	{
		st1 = "Vx =  m/s"; st2 = "Vy =  m/s"; st3 = "Vz =  m/s";
		editMessageText(MessageType::SENSOR_INFO, 0, "Velocity");
	}
	else if (current_data_type == DataType::LOCATION)
	{
		st1 = "X  =  m"; st2 = "Y  =  m"; st3 = "Z  =  m";
		editMessageText(MessageType::SENSOR_INFO, 0, "Current Location");
	}
	else if (current_data_type == DataType::EULER_ANGLES)
	{
		st1 = "Roll   =  deg"; st2 = "Pitch  =  deg"; st3 = "Yaw   =  deg";
		editMessageText(MessageType::SENSOR_INFO, 0, "Current Orientation");
	}

	p_data_x = p_graphics->getData(current_data_type, X);
	p_data_y = p_graphics->getData(current_data_type, Y);
	p_data_z = p_graphics->getData(current_data_type, Z);
	if (current_data_type == DataType::EULER_ANGLES) //convert from radians to degrees for ease of reading
	{
		st1.insert(9, std::to_string(p_data_x->at(cs) * 180.0 / 3.14159));
		st2.insert(9, std::to_string(p_data_y->at(cs) * 180.0 / 3.14159));
		st3.insert(8, std::to_string(p_data_z->at(cs) * 180.0 / 3.14159));
	}
	else
	{
		st1.insert(5, std::to_string(p_data_x->at(cs)));
		st2.insert(5, std::to_string(p_data_y->at(cs)));
		st3.insert(5, std::to_string(p_data_z->at(cs)));
	}
	editMessageText(MessageType::SENSOR_INFO, 1, st1);
	editMessageText(MessageType::SENSOR_INFO, 2, st2);
	editMessageText(MessageType::SENSOR_INFO, 3, st3);
}

//Utilization Functions
void FreeSwing::makeVec(std::vector<std::vector<float> >& vec, int size)
{
	std::vector<float> yo;
	for (int i = 0; i < size; i++) vec.push_back(yo);
}