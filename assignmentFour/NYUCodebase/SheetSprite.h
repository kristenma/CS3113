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


#ifndef SheetSprite_h
#define SheetSprite_h

class SheetSprite {
public:
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size);
    
    void Draw(ShaderProgram program);
    
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};


#endif /* SheetSprite_h */
