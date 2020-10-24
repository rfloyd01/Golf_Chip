#include "pch.h"

#include <chrono> //remove this when done with debugging
#include <iostream>
#include <cstring>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <Header_Files/graphics.h>
#include <Header_Files/vertices.h>
#include <Header_Files/stb_image.h>
#include <Header_Files/gnuplot.h>
#include <Header_Files/print.h>

//PUBLIC FUNCTIONS
//Constructors
GL::GL(BLEDevice* sensor)
{
	Initialize();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Enable necessary settings
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Set up shaders
	clubShader.SetAddress("Vertex.txt", "Fragment.txt");
	textShader.SetAddress("TextVertex.txt", "TextFragment.txt");
	modelShader.SetAddress("ModelVertex.txt", "ModelFragment.txt");

	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(800), 0.0f, static_cast<float>(600));
	textShader.use();
	glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	clubShader.use();
	clubShader.setInt("texture1", 0);

	projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
	clubShader.setMat4("projection", projection);

	//retrieve the matrix uniform locations
	club_model_location = glGetUniformLocation(clubShader.ID, "model");
	club_view_location = glGetUniformLocation(clubShader.ID, "view");

	SetBuffers();
	InitializeText();

	background_color = 0;

	background_colors.push_back({ 0.2f, 0.3f, 0.3f, 1.0f });
	background_colors.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

	club_translate = { 0.0, 0.0, 0.0 };
	club_scale = { 1.0, 1.0, 1.0 };

	record_data = false;
	can_press_key = true;
	key_time = .3; //sets the keyboard disable time after hitting one of the keyboard keys

	p_BLE = sensor;
	data_type = ACCELERATION; //initialize data_type to ACCELERATION

	//Initialize message map with blank Text vectors for all message types
	clearAllText(); //this function clears whatever is currently stored in messages map and creates empty containers for all message types

	club.loadModel("Resources/Models/Golf_Club/golf_club.obj"); //load graphic representation of golf club being rendered
}

//Setup Functions
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
void GL::setCal(Calibration* cal)
{
	//Make this a separate function instead of including it in constructor because a calibration doesn't necessarily need to be set up
	p_cal = cal;
}
GLFWwindow* GL::GetWindow()
{
	return window;
}

//Rendering Functions
void GL::Render(glm::quat q)
{
	//renderSensor(q);
	renderClub(q);
	RenderText();
	Swap();
}
void GL::SetRenderBackground(int num)
{
	//changes the color of the view screen background
	background_color = num;
}
void GL::SetClubMatrices(glm::vec3 s, glm::vec3 t)
{
	club_scale = s;
	club_translate = t;
}

//Text Based Functions
void GL::AddText(MessageType mt, Text new_text)
{
	//adds a new message to the end of the current message vector
	std::vector<Text> new_message;
	new_message.push_back(new_text);
	messages[mt].push_back(new_message);

	//check to see if message needs to be broken up into sub-messaages
	createSubMessages(mt, messages[mt].size() - 1);
}
void GL::clearAllText()
{
	//clears whatever is currently stored in the messages map and re-creates empty containers for all the message types
	messages.clear();
	std::vector<std::vector<Text> > blank_text;
	for (int i = 0; i < number_of_message_types; i++) messages[mtFromInt(i)] = blank_text;
}
void GL::clearMessageType(MessageType mt)
{
	//completely clears out the specified message type
	messages[mt].clear();
}
void GL::editText(MessageType mt, int index, std::string new_text)
{
	//this function will look at current message, erase it (in case it has been broken down into smaller sub-messages)
	//replace it with new_text, and then once again break it down into smaller sub-messaages if necessary
	//std::chrono::high_resolution_clock::time_point tt = std::chrono::high_resolution_clock::now();
	if (messages[mt].size() == 0)
	{
		std::cout << "No messages here to edit, returning from editText()" << std::endl;
		return;
	}
	Text new_message = { new_text, messages[mt][index][0].x, messages[mt][index][0].y, messages[mt][index][0].scale, messages[mt][index][0].color, messages[mt][index][0].x_end }; //keep all the same attributes as original message, just change text

	messages[mt][index].clear();
	messages[mt][index].push_back(new_message);

	createSubMessages(mt, index);
}
void GL::editText(MessageType mt, int index, std::string new_text, float x, float y)
{
	//this function will look at current message, erase it, then replace it with the new text and location
	//it will then break it down into smaller sub-messages if necessary
	if (messages[mt].size() == 0)
	{
		std::cout << "No messages here to edit, returning from editText()" << std::endl;
		return;
	}

	Text new_message = { new_text, x, y, messages[mt][index][0].scale, messages[mt][index][0].color, messages[mt][index][0].x_end }; //keep same scale and color as original message, but change text and location

	messages[mt][index].clear();
	messages[mt][index].push_back(new_message);

	createSubMessages(mt, index);
}

//Cleanup Functions
bool GL::ShouldClose()
{
	return glfwWindowShouldClose(window);
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
void GL::SetCanPressKey(bool val)
{
	can_press_key = val;
}
double GL::GetKeyTimer()
{
	return key_timer;
}
void GL::SetKeyTimer(double time)
{
	key_timer = time;
}

//Utilization Functions
void GL::Update()
{
	processInput();
	AddData();
	LiveUpdate();
}
void GL::processInput()
{
	if (can_press_key)
	{
		if (p_cal->getCalMode()) return; //calibration mode has it's own function for processing input, break here though because this function resets when a key can actually be pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			p_cal->CalibrationLoop();
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		{
			//show and hide current sensor readings
			if (display_readings)
			{
				display_readings = 0;
				if (record_data)
				{
					record_data = 0;
					DisplayGraph();
				}
				clearMessageType(MessageType::SENSOR_INFO);
				clearMessageType(MessageType::FOOT_NOTE);
			}
			else
			{
				display_readings = 1;
				AddText(MessageType::FOOT_NOTE, { "Press 'R' to record currently selected data set", 470.0f, 28.0f, 0.33f, {1.0, 1.0, 1.0}, (float)screen_width });
				AddText(MessageType::SENSOR_INFO, { "", 10, 130, 0.67, {1.0, 1.0, 1.0}, (float)screen_width });
				AddText(MessageType::SENSOR_INFO, { "", 10, 95, 0.67, {1.0, 0.0, 0.0}, (float)screen_width });
				AddText(MessageType::SENSOR_INFO, { "", 10, 60, 0.67, {0.0, 1.0, 0.0}, (float)screen_width });
				AddText(MessageType::SENSOR_INFO, { "", 10, 25, 0.67, {0.0, 0.0, 1.0}, (float)screen_width });
			}
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		{
			//switches live data stream to acceleration values if currently being displayed
			current_display = 0;
			data_type = ACCELERATION;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		{
			//switches live data stream to gyroscope values if currently being displayed
			current_display = 1;
			data_type = ROTATION;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		{
			//switches live data stream to magnetometer values if currently being displayed
			current_display = 2;
			data_type = MAGNETIC;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		{
			//switches live data stream to linear acceleration values if currently being displayed
			current_display = 3;
			data_type = LINEAR_ACCELERATION;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		{
			//switches live data stream to velocity values if currently being displayed
			current_display = 4;
			data_type = VELOCITY;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		{
			//switches live data stream to location values if currently being displayed
			current_display = 5;
			data_type = LOCATION;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			//pressing Enter while not in calibration mode will return the club to the center of the screen, useful for when image starts to drift over time
			p_BLE->resetPosition();
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			//pressing R is used for recording the currently selected data set, pressing R a second time or closing data display will cause recording to stop and then display a graph
			if (display_readings)
			{
				if (!record_data)
				{
					record_data = 1;

					//clear out any previously recorded data
					data_set.clear(); time_set.clear();
					MakeVec(data_set, 3);
					clearMessageType(MessageType::FOOT_NOTE);
					AddText(MessageType::FOOT_NOTE, { "Press 'R' to stop recording", 600.0f, 28.0f, 0.33f, {1.0, 1.0, 1.0}, (float)screen_width });
				}
				else
				{
					record_data = 0;
					clearMessageType(MessageType::FOOT_NOTE);
					AddText(MessageType::FOOT_NOTE, { "Press 'R' to record currently selected data set", 470.0f, 28.0f, 0.33f, {1.0, 1.0, 1.0}, (float)screen_width });
					DisplayGraph();
					p_BLE->resetTime();
				}
			}
			can_press_key = false;
			key_timer = glfwGetTime();
		}
	}
	else
	{
		//check to see if enough time has passed since the last key press, if so allow keys to be pressed again
		if (glfwGetTime() - key_timer >= key_time) can_press_key = true;
	}
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
void GL::SetBuffers()
{
	//First set up the VAO and VBO for rendering shapes
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // texture coord attribute
	glEnableVertexAttribArray(1);

	//Next set up the VAO and VBO for rendering text
	glGenVertexArrays(1, &TVAO);
	glGenBuffers(1, &TVBO);
	glBindVertexArray(TVAO);
	glBindBuffer(GL_ARRAY_BUFFER, TVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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
void GL::renderSensor(glm::quat q)
{
	glClearColor(background_colors[background_color][0], background_colors[background_color][1], background_colors[background_color][2], background_colors[background_color][3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	BindTexture(0);

	clubShader.use();

	glm::mat4 RotationMatrix = glm::mat4_cast(q); //create rotation matrix from current sensor rotation quaternion

	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);

	model = glm::translate(model, club_translate);
	model = glm::scale(model, club_scale);
	model *= RotationMatrix;

	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	// pass them to the shaders
	glUniformMatrix4fv(club_model_location, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(club_view_location, 1, GL_FALSE, &view[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, 36);
}
void GL::renderClub(glm::quat q)
{
	glClearColor(background_colors[background_color][0], background_colors[background_color][1], background_colors[background_color][2], background_colors[background_color][3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glBindVertexArray(VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//BindTexture(0); //may need to comment this out

	modelShader.use();

	// view/projection transformations
	glm::mat4 RotationMatrix = glm::mat4_cast(q); //create rotation matrix from current sensor rotation quaternion

	glm::mat4 projection = glm::perspective((float)glm::radians(90.0), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -1.5f));

	modelShader.setMat4("projection", projection);
	modelShader.setMat4("view", view);

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
	model *= RotationMatrix;
	modelShader.setMat4("model", model);
	club.Draw(modelShader);
}
void GL::RenderText()
{
	// activate corresponding render state	
	textShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TVAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (int m = 0; m < number_of_message_types; m++)
	{
		for (int i = 0; i < messages[mtFromInt(m)].size(); i++)
		{
			for (int j = 0; j < messages[mtFromInt(m)][i].size(); j++)
			{
				glUniform3f(glGetUniformLocation(textShader.ID, "textColor"), messages[mtFromInt(m)][i][j].color.x, messages[mtFromInt(m)][i][j].color.y, messages[mtFromInt(m)][i][j].color.z);
				float temp_x = messages[mtFromInt(m)][i][j].x;
				float scale = messages[mtFromInt(m)][i][j].scale;
				for (c = messages[mtFromInt(m)][i][j].text.begin(); c != messages[mtFromInt(m)][i][j].text.end(); c++)
				{
					Character ch = Characters[*c];

					float xpos = temp_x + ch.Bearing.x * scale;
					float ypos = messages[mtFromInt(m)][i][j].y - (ch.Size.y - ch.Bearing.y) * scale;

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
					glBindTexture(GL_TEXTURE_2D, ch.TextureID);
					// update content of VBO memory
					glBindBuffer(GL_ARRAY_BUFFER, TVBO);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tvertices), tvertices);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
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

//Text Based Functions
void GL::createSubMessages(MessageType mt, int index)
{
	//First, run through entire message and figure out where the top left and bottom right corners of text will be.
	//If either of these appear fall outside of the OpenGL screen proceed to break up the message
	std::string::const_iterator c, new_word; //new_word tracks the starting location of a new word and get's reset if there's a space
	Character ch;

	float temp_x;
	float max_y = 0, min_y = screen_height; //set max_y at min screen height and min_y at max screen height so they will get updated no matter what
	float scale = messages[mt][index][0].scale;
	for (int m = 0; m < messages[mt][index].size(); m++)
	{
		temp_x = messages[mt][index][m].x;
		for (c = messages[mt][index][m].text.begin(); c != messages[mt][index][m].text.end(); c++)
		{
			ch = Characters[*c];
			if (*c == ' ') new_word = c + 1; //if a space is encountered it means a new word is about to start, makr this spot in case message needs to be wrapped to next line here

			float xpos = temp_x + ch.Bearing.x * scale;
			float ypos = messages[mt][index][m].y - (ch.Size.y - ch.Bearing.y) * scale; //ypos denotes the bottom most part of the glyph

			//if xpos is greater than x_end cutoff limit, cut off current message at last space and create new message from that point, append it to messages[index]
			if (xpos >= messages[mt][index][m].x_end)
			{
				std::string next_text = "";
				for (auto cc = new_word; cc != messages[mt][index][m].text.end(); cc++)
				{
					next_text += *cc;
				}
				messages[mt][index][m].text.erase(new_word - 1, messages[mt][index][m].text.end()); //subtract 1 from new_word to get rid of the space there

				//Add new Text to current index of messages. Next line starts the height of the capital letter 'L' * 1.5 downwards, as that's the tallest letter I thought of at the time
				Text new_message = { next_text, messages[mt][index][m].x, messages[mt][index][m].y - (Characters['L'].Size.y * (float)1.5 * scale ), scale, messages[mt][index][m].color, messages[mt][index][m].x_end }; //keep all the same attributes as original message, except for text and starting y position
				messages[mt][index].push_back(new_message);

				break;
			}

			float h = ch.Size.y * scale;

			if (ypos < min_y) min_y = ypos;
			if ((ypos + h) > max_y) max_y = (ypos + h);

			temp_x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
	}

	//if max_y is higher than viewing window, need to shift everything down
	if (max_y > (float)screen_height)
	{
		//message starts too high up, lower all sub-messages by the amount of pixels between top of message and top of viewing window
		float shift = max_y + screen_height + 5; //give a 5 pixel buffer
		for (int i = 0; i < messages[mt][index].size(); i++) messages[mt][index][i].y -= shift;
		min_y -= shift;

		if (min_y < 0)
		{
			//make sure that lowering messages didn't lower the bottom past the edge of the viewing window, if so need to recreate original message and start over with smaller font size
			std::string original_string = "";
			for (int i = 0; i < messages[mt][index].size(); i++) original_string += (messages[mt][index][i].text + " ");

			Text original_message = { original_string, messages[mt][index][0].x, messages[mt][index][0].y + shift, scale * (float) 0.9, messages[mt][index][0].color, messages[mt][index][0].x_end }; //keep all the same attributes as original message, but reduce font size by 10%

			messages[mt][index].clear();
			messages[mt][index].push_back(original_message);
			createSubMessages(mt, index); //function recursively calls itself
		}
	}

	if (min_y < 0)
	{
		//end of message is too low, raise every sub message by the amount of pixels between bottom of screen and bottom of message
		float shift = 0 - min_y + 5; //give a 5 pixel buffer
		for (int i = 0; i < messages[mt][index].size(); i++) messages[mt][index][i].y += shift;
		max_y += shift;

		if (max_y > (float)screen_height)
		{
			//make sure that raising messages didn't raise the top past the edge of the viewing window, if so need to recreate original message and start over with smaller font size
			std::string original_string = "";
			for (int i = 0; i < messages[mt][index].size(); i++) original_string += (messages[mt][index][i].text + " ");

			Text original_message = { original_string, messages[mt][index][0].x, messages[mt][index][0].y - shift, scale * (float) 0.9, messages[mt][index][0].color, messages[mt][index][0].x_end }; //keep all the same attributes as original message, but reduce font size by 10%

			messages[mt][index].clear();
			messages[mt][index].push_back(original_message);
			createSubMessages(mt, index); //function recursively calls itself
		}
	}
}
void GL::LiveUpdate()
{
	if (display_readings)
	{
		int cs = p_BLE->getCurrentSample();
		std::string st1, st2, st3;
		if (current_display == 0)
		{
			st1 = "Ax =  m/s^2"; st2 = "Ay =  m/s^2"; st3 = "Az =  m/s^2";
			editText(MessageType::SENSOR_INFO, 0, "Accelerometer Readings");
		}
		else if (current_display == 1)
		{
			st1 = "Gx =  deg/s"; st2 = "Gy =  deg/s"; st3 = "Gz =  deg/s";
			editText(MessageType::SENSOR_INFO, 0, "Gyroscope Readings");
		}
		else if (current_display == 2)
		{
			st1 = "Mx =  uT"; st2 = "My =  uT"; st3 = "Mz =  uT";
			editText(MessageType::SENSOR_INFO, 0, "Magnetometer Readings");
		}
		else if (current_display == 3)
		{
			st1 = "Ax =  m/s^2"; st2 = "Ay =  m/s^2"; st3 = "Az =  m/s^2";
			editText(MessageType::SENSOR_INFO, 0, "Linear Acceleration");
		}
		else if (current_display == 4)
		{
			st1 = "Vx =  m/s"; st2 = "Vy =  m/s"; st3 = "Vz =  m/s";
			editText(MessageType::SENSOR_INFO, 0, "Velocity");
		}
		else if (current_display == 5)
		{
			st1 = "X  =  m"; st2 = "Y  =  m"; st3 = "Z  =  m";
			editText(MessageType::SENSOR_INFO, 0, "Current Location");
		}

		p_data_x = p_BLE->getData(data_type, X);
		p_data_y = p_BLE->getData(data_type, Y);
		p_data_z = p_BLE->getData(data_type, Z);
		st1.insert(5, std::to_string(p_data_x->at(cs)));
		st2.insert(5, std::to_string(p_data_y->at(cs)));
		st3.insert(5, std::to_string(p_data_z->at(cs)));
		editText(MessageType::SENSOR_INFO, 1, st1);
		editText(MessageType::SENSOR_INFO, 2, st2);
		editText(MessageType::SENSOR_INFO, 3, st3);
	}
}

//Utilization Functions
void GL::MakeVec(std::vector<std::vector<float> >& vec, int size)
{
	std::vector<float> yo;
	for (int i = 0; i < size; i++) vec.push_back(yo);
}

//Screen Size Functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
float GL::getScreenWidth()
{
	return (float)screen_width;
}

//Other Functions
MessageType mtFromInt(int m)
{
	if (m == 0) return MessageType::TITLE;
	else if (m == 1) return MessageType::SUB_TITLE;
	else if (m == 2) return MessageType::BODY;
	else if (m == 3) return MessageType::SENSOR_INFO;
	else if (m == 4) return MessageType::FOOT_NOTE;
}

Text::Text(std::string tex, float ex, float why, float sc, glm::vec3 col, float ep)
{
	text = tex; 
    x = ex;
	x_end = ep;
	y = why;
	scale = sc;
	color = col;
}
