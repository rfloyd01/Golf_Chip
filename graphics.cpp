#include "pch.h"

#include <cstring>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "graphics.h"
#include "initialize.h"
#include "vertices.h"
#include "stb_image.h"
#include "gnuplot.h"

GL::GL()
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
}

void GL::Initialize() //version should be entered like this: 3.3
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

void GL::DeleteBuffers()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

bool GL::ShouldClose()
{
	return glfwWindowShouldClose(window);
}

void GL::BindTexture(unsigned int tex)
{
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, textures[tex]);
}

void GL::Render()
{
	///TODO: need to form this into a MasterRender function at some point
	glClearColor(background_colors[background_color][0], background_colors[background_color][1], background_colors[background_color][2], background_colors[background_color][3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	BindTexture(0);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	if (!can_press_key) //unlock keyboard after key press here as this function gets called during every iteration of the main program loop
		if (glfwGetTime() - key_timer >= key_time) can_press_key = true;
}

void GL::RenderClub(glm::quat q)
{
	glClearColor(background_colors[background_color][0], background_colors[background_color][1], background_colors[background_color][2], background_colors[background_color][3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	BindTexture(0);

	clubShader.use();

	glm::mat4 RotationMatrix = glm::mat4_cast(q);

	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);

	model = glm::translate(model, club_translate);
	model = glm::scale(model, club_scale);
	model *= RotationMatrix;

	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	// pass them to the shaders (3 different ways)
	glUniformMatrix4fv(club_model_location, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(club_view_location, 1, GL_FALSE, &view[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	if (!can_press_key) //unlock keyboard after key press here as this function gets called during every iteration of the main program loop
		if (glfwGetTime() - key_timer >= key_time) can_press_key = true;
}

void GL::RenderText()
{
	// activate corresponding render state	
	textShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TVAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (int i = 0; i < messages.size(); i++)
	{
		glUniform3f(glGetUniformLocation(textShader.ID, "textColor"), messages[i].color.x, messages[i].color.y, messages[i].color.z);
		float temp_x = messages[i].x;
		for (c = messages[i].text.begin(); c != messages[i].text.end(); c++)
		{
			Character ch = Characters[*c];

			float xpos = temp_x + ch.Bearing.x * messages[i].scale;
			float ypos = messages[i].y - (ch.Size.y - ch.Bearing.y) * messages[i].scale;

			float w = ch.Size.x * messages[i].scale;
			float h = ch.Size.y * messages[i].scale;
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
			temp_x += (ch.Advance >> 6) * messages[i].scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GL::Terminate()
{
	glfwTerminate();
}

void GL::Swap()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void GL::RecordData()
{
	if (record_data)
	{
		record_data = false;
		std::cout << "Recording has just ended" << std::endl;
	}
	else
	{
		record_data = true;
		std::cout << "Recording has just begun." << std::endl;
	}
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

void GL::SetRenderBackground(int num)
{
	background_color = num;
}

void GL::SetClubMatrices(glm::vec3 s, glm::vec3 t)
{
	club_scale = s;
	club_translate = t;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void GL::setCal(Calibration* cal)
{
	p_cal = cal;
}

void GL::ClearAllText()
{
	messages.clear();
}

void GL::ClearText(int index)
{
	messages.erase(messages.begin() + index);
}

void GL::InsertText(int index, Text yo)
{
	messages.insert(messages.begin() + index, yo);
}

void GL::EditText(int index, std::string new_text)
{
	messages[index].text = new_text;
}

void GL::EditText(int index, std::string new_text, float x, float y)
{
	messages[index].text = new_text;
	messages[index].x = x;
	messages[index].y = y;
}

void GL::AddText(Text yoo)
{
	messages.push_back(yoo);
}

GLFWwindow* GL::GetWindow()
{
	return window;
}

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

void GL::processInput()
{
	if (can_press_key)
	{
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
				ClearText(0); ClearText(0); ClearText(0); ClearText(0); ClearText(0);
			}
			else
			{
				display_readings = 1;
				InsertText(0, { "Press 'R' to record currently selected data set", 470.0f, 28.0f, 0.33f, {1.0, 1.0, 1.0} });
				InsertText(0, { "", 10, 25, 0.67, {0.0, 0.0, 1.0} });
				InsertText(0, { "", 10, 60, 0.67, {0.0, 1.0, 0.0} });
				InsertText(0, { "", 10, 95, 0.67, {1.0, 0.0, 0.0} });
				InsertText(0, { "", 10, 130, 0.67, {1.0, 1.0, 1.0} });
			}
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		{
			//switches live data stream to acceleration values if currently being displayed
			current_display = 0;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		{
			//switches live data stream to gyroscope values if currently being displayed
			current_display = 1;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		{
			//switches live data stream to magnetometer values if currently being displayed
			current_display = 2;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		{
			//switches live data stream to linear acceleration values if currently being displayed
			current_display = 3;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		{
			//switches live data stream to position values if currently being displayed
			current_display = 4;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		{
			//switches live data stream to position values if currently being displayed
			current_display = 5;
			can_press_key = false;
			key_timer = glfwGetTime();
		}
		else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			//pressing Enter while not in calibration mode will return the club to the center of the screen
			reset_location = 1;
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

					record_clock = glfwGetTime();

					EditText(4, "Press 'R' to stop recording", 600.0f, 28.0f);
				}
				else
				{
					record_data = 0;
					EditText(4, "Press 'R' to record currently selected data set", 470.0f, 28.0f);
					//DisplayGraph();
					display_graph = 1;
				}
			}
			can_press_key = false;
			key_timer = glfwGetTime();
		}
	}
}

void GL::LiveUpdate(float& ax, float& ay, float& az, float& gx, float& gy, float& gz, float& mx, float& my, float& mz, float& lax, float& lay, float& laz, float& vx, float& vy, float& vz, float& x, float& y, float& z)
{
	if (display_readings)
	{
		if (current_display == 0)
		{
			EditText(0, "Accelerometer Readings");
			EditText(1, "Ax = " + std::to_string(ax) + " m/s^2");
			EditText(2, "Ay = " + std::to_string(ay) + " m/s^2");
			EditText(3, "Az = " + std::to_string(az) + " m/s^2");
		}
		else if (current_display == 1)
		{
			EditText(0, "Gyroscope Readings");
			EditText(1, "Gx = " + std::to_string(gx) + " deg/s");
			EditText(2, "Gy = " + std::to_string(gy) + " deg/s");
			EditText(3, "Gz = " + std::to_string(gz) + " deg/s");
		}
		else if (current_display == 2)
		{
			EditText(0, "Magnetometer Readings");
			EditText(1, "Mx = " + std::to_string(mx) + " uT");
			EditText(2, "My = " + std::to_string(my) + " uT");
			EditText(3, "Mz = " + std::to_string(mz) + " uT");
		}
		else if (current_display == 3)
		{
			EditText(0, "Linear Acceleration");
			EditText(1, "Ax = " + std::to_string(lax) + " m/s^2");
			EditText(2, "Ay = " + std::to_string(lay) + " m/s^2");
			EditText(3, "Az = " + std::to_string(laz) + " m/s^2");
		}
		else if (current_display == 4)
		{
			EditText(0, "Velocity");
			EditText(1, "Vx = " + std::to_string(vx) + " m/s");
			EditText(2, "Vy = " + std::to_string(vy) + " m/s");
			EditText(3, "Vz = " + std::to_string(vz) + " m/s");
		}
		else if (current_display == 5)
		{
			EditText(0, "Current Location");
			EditText(1, "X = " + std::to_string(x) + " m");
			EditText(2, "Y = " + std::to_string(y) + " m");
			EditText(3, "Z = " + std::to_string(z) + " m");
		}
	}
}

void GL::AddData(float x, float y, float z)
{
	data_set[0].push_back(x);
	data_set[1].push_back(y);
	data_set[2].push_back(z);

	time_set.push_back(glfwGetTime() - record_clock);
}

void GL::MakeVec(std::vector<std::vector<float> >& vec, int size)
{
	std::vector<float> yo;
	for (int i = 0; i < size; i++) vec.push_back(yo);
}

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
