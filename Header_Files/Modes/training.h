#pragma once

#include <Header_Files/Modes/mode.h>
#include <Header_Files/BluetoothLE.h>

#define pi 3.14159

class Training : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	Training(GL& graphics) : Mode(graphics)
	{
		mode_name = "Training";
		mode_type = ModeType::TRAINING;

		background_color = { .4, 0.282, 0.2 };

		clearAllText();
		clearAllImages();
	};
	//Updating and Advancement Functions
	void update(); //virtual allows a sub-class to overwrite the base class' implementation of the function
	void processInput();
	void modeStart();
	void modeEnd();

private:
	//PRIVATE FUNCTIONS
	//Setup Functions
	void initializeText();

	//Advancement Functions
	void readySwing();
	void planeNextStep();
	void planeTraining();
	void tiltNextStep();
	void tiltTraining();
	bool eulerAnglesOk();

	//Data Gathering Functions
	void getCurrentClubAngles(); //get the current euler angles of sensor on club

	//Physics Functions
	bool detectCollision(Model a, Model b);

	//PRIVATE VARIABLES
	//State Variables
	int training_state = 0; //0 = training menu, 1 = Swing Plane, 2 = Clubeface Squareness, 3 = Clubface Tilt, 4 = Clubhead Speed
	int training_stage = 0; //denotes at which stage in the training the user is currently in
	int successful_swings = 0; //keeps track of how many successful swings have occured in a training exercise
	bool ready_to_swing = 0; //lets the program know if the conditions have been met to start tracking a swing

	//Data Variables
	float current_angles[3]; //this variable holds the current Euler Angles of the chip on the golf club
	float start_angles[3]; //this variable records the club's Euler Angles at the start of a training session (order is pitch, roll then yaw)
	float start_angle_threshold = 5 * pi / 180.0; //the club can't move more degrees along any angle than this threshold before start_time_threshold has elapsed from start_time
	float start_time; //once the start_angle has been set, start_time variable is set to glfwGetTime().
	float start_time_threshold = 2.0; //after start_time is set, club must stay still within start_angle_threshold for start_time_threshold seconds to initiate training

	//Graph Variables
	std::vector<std::vector<float> > data_set; //records data to graph, graph y azis
	std::vector<float> time_set; //records time to graph, graph x axis

	//pointers to sensor data
	std::vector<float>* p_data_x;
	std::vector<float>* p_data_y;
	std::vector<float>* p_data_z;
};