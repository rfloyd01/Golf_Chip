#pragma once

#include <map>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm.h"
#include "shader.h"
#include "calibration.h"

class Shader;
class Calibration;

struct Character
{
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int Advance;
};

struct Text
{
	Text(std::string tex, float ex, float why, float sc, glm::vec3 col) { text = tex;  x = ex; y = why; scale = sc; color = col; }
	std::string text;
	float x;
	float y;
	float scale;
	glm::vec3 color;
};

class GL
{
public:
	GL();
	void LoadTexture(const char* name);
	void Initialize();
	//void UseShader();
	void DeleteBuffers();
	void processInput();
	void BindTexture(unsigned int tex);
	void setCal(Calibration* cal);

	void Render();
	void RenderClub(glm::quat q);
	void SetRenderBackground(int num);
	void SetClubMatrices(glm::vec3 s, glm::vec3 t);

	//Functions for displaying text
	void AddText(Text yoo);
	void ClearAllText();
	void ClearText(int index);
	void InsertText(int index, Text yo);
	void RenderText();
	void EditText(int index, std::string new_text);
	void EditText(int index, std::string new_text, float x, float y);
	void LiveUpdate(float& ax, float& ay, float& az, float& gx, float& gy, float& gz, float& mx, float& my, float& mz, float& lax, float& lay, float& laz, float& vx, float& vy, float& vz, float& x, float& y, float& z);

	void Terminate();
	void RecordData();
	void AddData(float x, float y, float z);
	void InitializeText();
	void Swap();

	//Key Press functions
	bool GetCanPressKey();
	void SetCanPressKey(bool val);
	double GetKeyTimer();
	void SetKeyTimer(double time);

	bool ShouldClose();

	void DisplayGraph();

	GLFWwindow* GetWindow();

	bool should_render_text = 0, display_readings = 0, reset_location = 0, record_data = 0, display_graph = 0;
	int current_display = 0;
private:
	void SetBuffers();
	void MakeVec(std::vector<std::vector<float> >& vec, int size);

	unsigned int VBO, VAO, TVBO, TVAO;
	unsigned int club_model_location, club_view_location;
	std::vector<unsigned int> textures;
	int screen_width = 800, screen_height = 600, wtf = 0, background_color;
	float version = 3.3;
	bool can_press_key; //can_press_key will disable keyboard presses momentarily after a key is pressed, to make sure the same key isn't pressed multiple times in one go
	double key_timer, key_time;
	GLFWwindow* window;
	std::map<char, Character> Characters;
	Shader clubShader, textShader;
	std::vector<glm::vec4> background_colors;
	glm::vec3 club_translate, club_scale;
	std::vector<Text> messages;

	std::vector<std::vector<float> > data_set; //records data to graph
	std::vector<float> time_set;
	double record_clock;

	Calibration* p_cal;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
