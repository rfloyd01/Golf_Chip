#pragma once

#include <string>
#include <map>

#include <Header_Files/graphics.h>
#include <Header_Files/text.h>
#include <Header_Files/model.h>

//Classes, structs and enums defined in other headers
class GL;

struct Text;

enum class MessageType;

//Classes, structs and enums that are helpful for this class
enum class ModeType
{
	//This enum represents all of the different types of modes there are for the program
	//when adding a new mode it should also be added to this list

	BLANK = 0,
	MAIN_MENU = 1,
	FREE = 2,
	CALIBRATION = 3,
	TRAINING = 4
};

//Class definition
class Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	Mode(GL& graphics);

	//Updating and Advancement Functions
	virtual void update(); //virtual allows a sub-class to overwrite the base class' implementation of the function
	virtual void processInput();
	virtual void modeStart();
	virtual void modeEnd();
	
	//Get Functions
	//bool getSeparateRotation();
	//glm::quat getSeparateQuaternion();
	ModeType getModeType();
	glm::vec3 getCameraLocation();
	std::map<MessageType, std::vector<std::vector<Text> > >* getRenderText(); //returns a pointer to the message_map with all text to be rendered on screen
	std::map<ModelType, std::vector<Model> >* getRenderModels(); //returns a pointer to the model_map with all models to be rendered
	glm::vec3 getBackgroundColor();

	//Set Functions
	void setClubRotation(glm::quat q);
	void setClubLocation(glm::vec3 l);
	void setClubScale(glm::vec3 s);

protected:
	//PROTECTED FUNCTIONS
	//Text Based Functions
	void clearAllText();
	void clearMessageType(MessageType mt);
	void addText(MessageType mt, Text new_message);
	void editMessage(MessageType mt, int index, Text edited_message);
	void editMessageText(MessageType mt, int index, std::string new_text);
	void editMessageLocation(MessageType mt, int index, float new_x, float new_y);
	void createSubMessages(MessageType mt, int index); //this function is used when a new message is too long to fit on screen so the text needs to be wrapped

	//Model Based Functions
	void clearAllImages();

	//PROTECTED VARIABLES
	//Mode Identifiers
	std::string mode_name;
	ModeType mode_type = ModeType::BLANK;

	//Boolean Variables
	bool mode_active = 0;
	//bool separate_rotation_matrix = 0; //sometimes wish to render club in a fixed position rather than according to current sensor reading, this bool allows that

	//Rendering Variables
	glm::vec3 background_color = { 0.0, 0.0, 0.0 }; //generic background color is black
	glm::vec3 camera_location = { 0.0, 0.0, -1.5 }; //location in space of camera
	//glm::quat mode_q = { 1, 0, 0, 0 }; //used when it's necessary to render club or chip in position other than what sensor is currently reading
	std::map<MessageType, std::vector<std::vector<Text> > > message_map; //a map used to store all words to be rendered on screen, a map is used to make it easier when adding and deleting messages
	std::map<ModelType, std::vector<Model> > model_map; //a map used to store all images to be rendered on screen, a map is used to make it easier to keep track of where models are located

	//Class Pointers
	GL* p_graphics;
};