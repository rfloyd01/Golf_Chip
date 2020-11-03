#include "pch.h"

#include <chrono> //remove this when done with debugging
#include <iostream>
#include <cstring>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <Header_Files/graphics.h>
#include <Header_Files/stb_image.h>
#include <Header_Files/gnuplot.h>

//PUBLIC FUNCTIONS
//Constructors
GL::GL(BLEDevice* sensor)
{
	Initialize();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Enable necessary settings
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	//glfwSwapInterval(0); //If this is uncommented it will disable v-sync, allowing OpenGl to update faster than the monitor refresh rate (60 Hz in this case)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Set up shaders
	clubShader.SetAddress("Vertex.txt", "Fragment.txt");
	textShader.SetAddress("TextVertex.txt", "TextFragment.txt");
	modelShader.SetAddress("ModelVertex.txt", "ModelFragment.txt");
	lineShader.SetAddress("LineVertex.txt", "LineFragment.txt");

	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(800), 0.0f, static_cast<float>(600));
	textShader.use();
	glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	lineShader.use();
	glUniformMatrix4fv(glGetUniformLocation(lineShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	clubShader.use();
	clubShader.setInt("texture1", 0);

	projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
	clubShader.setMat4("projection", projection);

	//retrieve the matrix uniform locations
	club_model_location = glGetUniformLocation(clubShader.ID, "model");
	club_view_location = glGetUniformLocation(clubShader.ID, "view");

	setTextBuffers();
	InitializeText();

	club_translate = { 0.0, 0.0, 0.0 };
	club_scale = { 1.0, 1.0, 1.0 };

	record_data = false;
	can_press_key = true;
	key_time = .3; //sets the keyboard disable time after hitting one of the keyboard keys

	p_BLE = sensor;
	data_type = DataType::ACCELERATION; //initialize data_type to ACCELERATION
}

//Setup Functions
/*
void GL::LoadTexture(const char* name)
{
	unsigned int texture;

	char location[200] = "Resources/";
	strcat_s(location, name);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	unsigned char* data = stbi_load(location, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	textures.push_back(texture);
}
*/
GLFWwindow* GL::GetWindow()
{
	return window;
}

//Rendering Functions
void GL::masterRender()
{
	renderModels();
	renderText();
	Swap();
}
void GL::SetClubMatrices(glm::vec3 s, glm::vec3 t)
{
	club_scale = s;
	club_translate = t;
}
std::vector<glm::vec3> GL::getClubMatrices()
{
	return { club_scale, club_translate };
}

//Text Based Functions
Character* GL::getCharacterInfo(char c)
{
	return &Characters[c];
}

//Cleanup Functions
bool GL::ShouldClose()
{
	return glfwWindowShouldClose(window);
}
void GL::setWindowShouldClose()
{
	glfwSetWindowShouldClose(window, true);
}
void GL::Terminate()
{
	DeleteBuffers();
	glfwTerminate();
}

//Key Press Functions
bool GL::GetCanPressKey()
{
	return can_press_key;
}
double GL::GetKeyTime()
{
	return key_time;
}
void GL::resetKeyTimer()
{
	can_press_key = 0;
	key_timer = glfwGetTime();
}

//Utilization Functions
void GL::masterUpdate()
{

	setCanPressKey(); //allow use of the keyboard if necessary
	p_current_mode->update();
}

//Graph Related Functions
void GL::DisplayGraph()
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

	if (current_display == 0)
	{
		graph_title = "'Acceleration vs. Time'";
		y_label = "'Acceleration (m/s^2)'";
	}
	else if (current_display == 1)
	{
		graph_title = "'Angular Velocity vs. Time'";
		y_label = "'Angular Velocity (deg/s)'";
	}
	else if (current_display == 2)
	{
		graph_title = "'Magnetic Field vs. Time'";
		y_label = "'Mag. Field (uT)'";
	}
	else if (current_display == 3)
	{
		graph_title = "'Linear Acceleration vs. Time'";
		y_label = "'Acceleration (m/s^2)'";
	}
	else if (current_display == 4)
	{
		graph_title = "'Velocity vs. Time'";
		y_label = "'Velocity (m/s)'";
	}
	else if (current_display == 5)
	{
		graph_title = "'Location vs. Time'";
		y_label = "'Position (m)'";
	}

	Gnuplot graph;
	graph("set title " + graph_title);
	graph("set xlabel 'Time (s)'");
	graph("set ylabel " + y_label);
	graph(function);
}
void GL::RecordData()
{
	if (record_data)
	{
		record_data = false;
	}
	else
	{
		record_data = true;
	}
}
void GL::AddData()
{
	if (record_data == 0) return; //only record data if in the proper mode

	int cs = p_BLE->getCurrentSample();

	data_set[0].push_back(p_BLE->getData(data_type, X)->at(cs));
	data_set[1].push_back(p_BLE->getData(data_type, Y)->at(cs));
	data_set[2].push_back(p_BLE->getData(data_type, Z)->at(cs));

	time_set.push_back(p_BLE->getCurrentTime());
}

//Screen Size Functions
float GL::getScreenWidth()
{
	return (float)screen_width;
}
float GL::getScreenHeight()
{
	return (float)screen_height;
}

//Sensor Functions
std::vector<float>* GL::getData(DataType dt, Axis a)
{
	return p_BLE->getData(dt, a);
}
std::vector<float>* GL::getRawData(DataType dt, Axis a)
{
	return p_BLE->getRawData(dt, a);
}
int GL::getCurrentSample()
{
	return p_BLE->getCurrentSample();
}
float GL::getCurrentTime()
{
	return p_BLE->getCurrentTime();
}
void GL::resetTime()
{
	p_BLE->resetTime();
}
void GL::updateCalibrationNumbers()
{
	p_BLE->updateCalibrationNumbers();
}

//Mode Functions
void GL::addMode(Mode* m)
{
	mode_map[m->getModeType()] = m;
}
Mode* GL::getCurrentMode()
{
	return p_current_mode;
}
void GL::setCurrentMode(ModeType m)
{
	p_current_mode = mode_map[m];
	p_current_mode->modeStart();
}

//PRIVATE FUNCTIONS
//Setup Functions
void GL::Initialize()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)version);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)(10 * version) % 10);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = glfwCreateWindow(screen_width, screen_height, "BLE33 Nano Sense", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to careate GLFW window" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	glEnable(GL_DEPTH_TEST);
}
void GL::InitializeText()
{
	//Freetype font stuff
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, "Resources/Fonts/segoeui.ttf", 0, &face)) std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	FT_Set_Pixel_Sizes(face, 0, 48);
	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned char c = 0; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
			continue;
		}

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = { texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x };
		Characters.insert(std::pair<char, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

//Buffer and Texture Update Functions
void GL::setTextBuffers()
{
	//Next set up the VAO and VBO for rendering text
	glGenVertexArrays(1, &TVAO);
	glGenBuffers(1, &TVBO);
	glBindVertexArray(TVAO);
	glBindBuffer(GL_ARRAY_BUFFER, TVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	//each letter is stored in VBO before rendering which is why dynamic draw is used
	//size is 24 floats because each letter is made up of 2 triangles, each with 3 vertices (6 vertices * 2 spacial coordinates * 2 texture coordinates = 24 floats)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind
	glBindVertexArray(0); //unbind

	//Set up VAO and VBO for rendering lines
	glGenVertexArrays(1, &LVAO);
	glGenBuffers(1, &LVBO);
	glBindVertexArray(LVAO);
	glBindBuffer(GL_ARRAY_BUFFER, LVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 6, NULL, GL_DYNAMIC_DRAW);
	//each line is stored in VBO before rendering which is why dynamic draw is used
	//size is 12 floats because each line is made up of 2 vertices, each with 3 spatial coordinates and 3 color coordinates (2 * (3 + 3) = 12)
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)3);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind
	glBindVertexArray(0); //unbind
}
void GL::DeleteBuffers()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}
void GL::BindTexture(unsigned int tex)
{
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, textures[tex]);
}
void GL::Swap()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

//Rendering Functions
void GL::renderText()
{
	// activate corresponding render state	
	textShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TVAO);

	//get pointer to model specific text
	std::map<MessageType, std::vector<std::vector<Text> > >* p_messages = p_current_mode->getRenderText();

	// iterate through all characters
	std::string::const_iterator c;
	int heyhey = number_of_message_types;
	for (int m = 0; m < heyhey; m++)
	{
		for (int i = 0; i < (*p_messages)[mtFromInt(m)].size(); i++)
		{
			for (int j = 0; j < (*p_messages)[mtFromInt(m)][i].size(); j++)
			{
				glUniform3f(glGetUniformLocation(textShader.ID, "textColor"), (*p_messages)[mtFromInt(m)][i][j].color.x, (*p_messages)[mtFromInt(m)][i][j].color.y, (*p_messages)[mtFromInt(m)][i][j].color.z);
				float temp_x = (*p_messages)[mtFromInt(m)][i][j].x;
				float scale = (*p_messages)[mtFromInt(m)][i][j].scale;
				for (c = (*p_messages)[mtFromInt(m)][i][j].text.begin(); c != (*p_messages)[mtFromInt(m)][i][j].text.end(); c++)
				{
					Character ch = Characters[*c];

					float xpos = temp_x + ch.Bearing.x * scale;
					float ypos = (*p_messages)[mtFromInt(m)][i][j].y - (ch.Size.y - ch.Bearing.y) * scale;

					float w = ch.Size.x * scale;
					float h = ch.Size.y * scale;

					// update VBO for each character
					float tvertices[6][4] = {
						{ xpos,     ypos + h,   0.0f, 0.0f },
						{ xpos,     ypos,       0.0f, 1.0f },
						{ xpos + w, ypos,       1.0f, 1.0f },

						{ xpos,     ypos + h,   0.0f, 0.0f },
						{ xpos + w, ypos,       1.0f, 1.0f },
						{ xpos + w, ypos + h,   1.0f, 0.0f }
					};
					// render glyph texture over quad
					glBindTexture(GL_TEXTURE_2D, ch.TextureID); //TODO: does this need to be set every iteration of the loop, or only once at the beginning of loop?
					// update content of VBO memory
					glBindBuffer(GL_ARRAY_BUFFER, TVBO); //TODO: does this need to be bound every iteration of the loop, or only once at the beginning of loop?
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tvertices), tvertices);
					glBindBuffer(GL_ARRAY_BUFFER, 0); //TODO: does this need to be bound every iteration of the loop, or only once at the beginning of loop?
					// render quad
					glDrawArrays(GL_TRIANGLES, 0, 6);
					// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
					temp_x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
				}
			}
		}
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void GL::renderModels()
{
	glm::vec3 br = p_current_mode->getBackgroundColor();
	glClearColor(br[0], br[1], br[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	std::map<ModelType, std::vector<Model> >* models = p_current_mode->getRenderModels();
	std::map<ModelType, std::vector<Model> >::iterator iter;

	for (iter = (*models).begin(); iter != (*models).end(); iter++) //need to loop through all ModelTypes in the model_map
	{
		for (int i = 0; i < iter->second.size(); i++) //need to loop through all models of a specific ModelType
		{
			if (iter->first == ModelType::CLUB | iter->first == ModelType::CHIP) //render instructions specific to drawing the golf club
			{
				modelShader.use(); //ultimately will need to create a different shader specific to the golf club
				glm::quat q; 
				if (p_current_mode->getSeparateRotation()) q = p_current_mode->getSeparateQuaternion();
				else q = p_BLE->getOpenGLQuaternion();

				// view/projection transformations
				glm::mat4 RotationMatrix = glm::mat4_cast(q); //create rotation matrix from current sensor rotation quaternion

				glm::mat4 projection = glm::perspective((float)glm::radians(90.0), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
				glm::mat4 view = glm::mat4(1.0f);
				//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -1.5f));
				view = glm::translate(view, p_current_mode->getCameraLocation());

				modelShader.setMat4("projection", projection);
				modelShader.setMat4("view", view);

				// render the loaded model
				//order of operations for model transform is translate, rotate, then scale
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, club_translate);
				model *= RotationMatrix;
				model = glm::scale(model, club_scale);
				modelShader.setMat4("model", model);
				iter->second[i].Draw(modelShader);
			}
			else if (iter->first == ModelType::LINE_OBJECTS)
			{
				modelShader.use();
			    //glm::quat q = { (float)cos(glfwGetTime()/2.0), 0, 0, (float)sin(glfwGetTime()/2.0) };
				glm::quat q = { .966, 0, 0, .259 };

				// view/projection transformations
				glm::mat4 RotationMatrix = glm::mat4_cast(q); //create rotation matrix from current sensor rotation quaternion
				glm::mat4 projection = glm::perspective((float)glm::radians(90.0), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
				glm::mat4 view = glm::mat4(1.0f);
				view = glm::translate(view, p_current_mode->getCameraLocation());

				modelShader.setMat4("projection", projection);
				modelShader.setMat4("view", view);

				// render the loaded model
				//order of operations for model transform is translate, rotate, then scale
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, iter->second[i].getLocation());
				model *= RotationMatrix;
				model = glm::scale(model, iter->second[i].getScale());
				
				modelShader.setMat4("model", model);
				iter->second[i].Draw(modelShader);
			}
		}
	}
}

//Key Press Functions
void GL::setCanPressKey()
{
	if (!can_press_key) //see if key presses can be reset, if so then reset the bool variable which allows it
	{
		if (glfwGetTime() - key_timer >= key_time) can_press_key = true;
	}
}

//Screen Size Functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}