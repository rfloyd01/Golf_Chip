#include "pch.h"

#include <iostream>
#include <Header_Files/print.h>

void print(int a)
{
	std::cout << a << std::endl;
}

void print(glm::quat q)
{
	std::cout << '{' << q.w << ", " << q.x << ", " << q.y << ", " << q.z << '}' << std::endl;
}
