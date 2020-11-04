#pragma once

#include <Header_Files/Modes/mode.h>
#include <Header_Files/BluetoothLE.h>

class GL;
class Text;

enum class DataType;

class FreeSwing : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	FreeSwing(GL& graphics) : Mode(graphics)
	{
		mode_name = "Free Swing";
		mode_type = ModeType::FREE;
		background_color = { 0.2f, 0.3f, 0.3f };

		clearAllText();
		clearAllImages();

		current_data_type = DataType::ACCELERATION; //start with acceleration as display variable
	};
	//Updating and Advancement Functions
	void update(); //virtual allows a sub-class to overwrite the base class' implementation of the function
	void processInput();
	void modeStart();
	void modeEnd();

private:
	//PRIVATE FUNCTIONS
	//Graph Related Functions
	void displayGraph();

	//Data Functions
	void liveUpdate();

	//Utilization Functions
	void makeVec(std::vector<std::vector<float> >& vec, int size);

	//PRIVATE VARIABLES
	//Bool Variables
	bool record_data = 0, display_data = 0; //keeps track of when to show live data on screen and when to record values to graph

	//Graph Variables
	std::vector<std::vector<float> > data_set; //records data to graph, graph y azis
	std::vector<float> time_set; //records time to graph, graph x axis

	//Data Variables
	DataType current_data_type; //keeps track of which data type to display and record

	//pointers to sensor data
	std::vector<float>* p_data_x;
	std::vector<float>* p_data_y;
	std::vector<float>* p_data_z;
};