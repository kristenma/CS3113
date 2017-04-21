#pragma once

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL_opengl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Matrix.h"
#include "ShaderProgram.h"


#ifndef Entity_h
#define Entity_h

class Entity {
    public:
    
    Entity(float w, float h, float x, float y, float s);
    
    void Draw(ShaderProgram *program);
    
    float x;
    float y;
    float rotation;
    
    float width;
    float height;
    
    float speed;
    float direction_x;
    float direction_y;
};


#endif /* Entity_h */
