#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>

GLFWwindow* Initialize(float glfwVersion, int width, int height) //the version of GLFW would be entered like 3.3
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)glfwVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)(10 * glfwVersion) % 10);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(width, height, "Golf Chip", NULL, NULL);

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

	return window;
}
