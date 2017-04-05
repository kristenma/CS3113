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
#include "SheetSprite.h"
#include "Vector3.h"


#ifndef Entity_h
#define Entity_h

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN};

class Entity {
    public:
        Entity(float x, float y, float w, float h, float vx, float vy, float ax, float ay, bool stat, EntityType type);
        //Entity();
    
        void Update(float elapsed);
        void Render(ShaderProgram *program, Matrix& matrix);
        bool collidesWith( Entity* entity);
    
        SheetSprite sprite;
    
        float x;
        float y;
    
        float width;
        float height;
    
        float velocity_x;
        float velocity_y;
    
        float acceleration_x;
        float acceleration_y;
    
        bool isStatic;

        EntityType entityType;

        bool collidedTop;
        bool collidedBottom;
        bool collidedLeft;
        bool collidedRight;
    
};

#endif /* Entity_h */
