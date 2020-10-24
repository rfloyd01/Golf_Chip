#pragma once

#include <map>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Header_Files/glm.h>
#include <Header_Files/shader.h>
#include <Header_Files/calibration.h>
#include <Header_Files/BluetoothLE.h>
#include <Header_Files/model.h>

class Shader;
class Calibration;
class BLEDevice;
class Model;

//Structs
struct Character
{
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int Advance;
};

struct Text
{
	Text(std::string tex, float ex, float why, float sc, glm::vec3 col, float ep);// { text = tex;  x = ex; y = why; scale = sc; color = col; }
	std::string text;
	float x;
	float y;
	float scale;
	float x_end; //denotes the point on screen at witch text should wrap to the next line

	glm::vec3 color;
};

//Enums
enum DataType; //this enum is taken from the BLEDevice page

enum class MessageType
{
	//this enum class is used to group different messages displayed on screen
	TITLE = 0,
	SUB_TITLE = 1,
	BODY = 2,
	SENSOR_INFO = 3,
	FOOT_NOTE = 4
};

MessageType mtFromInt(int m);

class GL
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	GL(BLEDevice* sensor);

	//Setup Functions
	void LoadTexture(const char* name);
	void setCal(Calibration* cal);
	GLFWwindow* GetWindow();

	//Rendering Functions
	//void Render();
	void Render(glm::quat q); //same as the above Render function but allows for manual setting of sensor rotation quaternion
	void SetRenderBackground(int num);
	void SetClubMatrices(glm::vec3 s, glm::vec3 t);

	//Text Based Functions
	void AddText(MessageType mt, Text new_message);
	void clearAllText();
	void clearMessageType(MessageType mt);
	void editText(MessageType mt, int index, std::string new_text);
	void editText(MessageType mt, int index, std::string new_text, float x, float y);

	//Cleanup Functions
	bool ShouldClose();
	void Terminate();
	//put deconstructor here eventually?

	//Key Press Functions
	bool GetCanPressKey();
	void SetCanPressKey(bool val);
	double GetKeyTimer();
	void SetKeyTimer(double time);

	//Utilization Functions
	void Update();
	void processInput();

	//Graph Related Functions
	void DisplayGraph();
	void RecordData();
	void AddData();

	//Screen Size Functions
	float getScreenWidth();

	//PUBLIC VARIABLES
	BLEDevice* p_BLE;

	bool display_readings = 0, record_data = 0; //these variables keep track of when to show sensor data and record data for creating graphs
	int current_display = 0; //keeps track of what sensor, if any, to display on screen
private:
	//PRIVATE FUNCTIONS
	//Setup Functions
	void Initialize();
	void InitializeText();

	//Buffer and Texture Update Functions
	void SetBuffers();
	void DeleteBuffers();
	void BindTexture(unsigned int tex);
	void Swap();

	//Rendering Functions
	void renderSensor(glm::quat q); //renders an image of sensor gathering data
	void renderClub(glm::quat q); //renders an image of a golf club where the sensor is assumed to be embedded near the top of the grip
	void RenderText();

	//Text Based Functions
	void createSubMessages(MessageType mt, int index);
	void LiveUpdate();

	//Utilization Functions
	void MakeVec(std::vector<std::vector<float> >& vec, int size);

	//PRIVATE VARIABLES
	//pointers to sensor data
	std::vector<float>* p_data_x;
	std::vector<float>* p_data_y;
	std::vector<float>* p_data_z;

	//Data type variable
	DataType data_type; //keeps track of the current data type (acceleration, magnetic, etc.) that is currently being displayed and graphed

	//OpenGL Variables
	float version = 3.3; //the version of OpenGL to be used
	GLFWwindow* window;
	unsigned int VBO, VAO, TVBO, TVAO;
	Shader clubShader, textShader, modelShader;
	Model club;

	//Rendering Variables
	glm::vec3 club_translate, club_scale;
	unsigned int club_model_location, club_view_location;
	std::vector<unsigned int> textures;

	//Viewscreen Variables
	int screen_width = 800, screen_height = 600, background_color = 0;
	std::vector<glm::vec4> background_colors; //a vector that keeps different colors stored in it, can quickly change color of view screen with these colors via background_color variable

	//Key Press Variables
	bool can_press_key; //can_press_key will disable keyboard presses momentarily after a key is pressed, to make sure the same key isn't pressed multiple times in one go
	double key_timer, key_time; //key timer keeps track of time elapsed since last time a key was pressed, key_time sets the limit on time allowable between key presses

	//Text Variables
	std::map<char, Character> Characters; //a map that holds rendering info on the first 128 ASCII characters, info includes shape, dimensions, etc.
	std::map<MessageType, std::vector<std::vector<Text> > > messages; //a vector that holds vectors which contain individual messages to be rendered, broken up into sub-messages so that everything fits on screen
	int number_of_message_types = 5;

	//Graph Variables
	std::vector<std::vector<float> > data_set; //records data to graph, graph y azis
	std::vector<float> time_set; //records time to graph, graph x axis

	//Pointer to Calibration
	Calibration* p_cal;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
