#pragma once

float length = 1.0 / 2.0, width = 0.6 / 2.0, height = 0.06 / 2.0;

float vertices[] = {
    //back face
    -width, -height, -length,  0.331f, 0.0f,
     width, -height, -length,  0.63f, 0.0f,
     width,  height, -length,  0.63f, 0.03f,
     width,  height, -length,  0.63f, 0.03f,
    -width,  height, -length,  0.331f, 0.03f,
    -width, -height, -length,  0.331f, 0.0f,

    //front face
    -width, -height,  length,  0.331f, 0.0f, //bottom left
     width, -height,  length,  0.63f, 0.0f, //bottom right
     width,  height,  length,  0.63f, 0.03f, //top right
     width,  height,  length,  0.63f, 0.03f, //top right
    -width,  height,  length,  0.331f, 0.03f, //top left
    -width, -height,  length,  0.331f, 0.0f, //bottom left

    //left face
    -width,  height,  length,  0.301f, 0.0f, //top left
    -width,  height, -length,  0.301f, 0.501f, //top right
    -width, -height, -length,  0.33f, 0.501f, //bottom right
    -width, -height, -length,  0.33f, 0.501f, //bottom right
    -width, -height,  length,  0.33f, 0.0f, //bottom left
    -width,  height,  length,  0.301f, 0.0f, //top left

    //right face
     width,  height,  length,  0.301f, 0.0f, //top left
     width,  height, -length,  0.301f, 0.501f, //top right
     width, -height, -length,  0.33f, 0.501f, //bottom right
     width, -height, -length,  0.33f, 0.501f, //bottom right
     width, -height,  length,  0.33f, 0.0f, //bottom left
     width,  height,  length,  0.301f, 0.0f, //top left

     //bottom face
    -width, -height, -length,  0.3f, 1.0f,
     width, -height, -length,  0.0f, 1.0f,
     width, -height,  length,  0.0f, 0.501f,
     width, -height,  length,  0.0f, 0.501f,
    -width, -height,  length,  0.3f, 0.501f,
    -width, -height, -length,  0.3f, 1.0f,

    //top face
    -width,  height, -length,  0.0f, 0.5f,
     width,  height, -length,  0.3f, 0.5f,
     width,  height,  length,  0.3f, 0.0f,
     width,  height,  length,  0.3f, 0.0f,
    -width,  height,  length,  0.0f, 0.0f,
    -width,  height, -length,  0.0f, 0.5f
};