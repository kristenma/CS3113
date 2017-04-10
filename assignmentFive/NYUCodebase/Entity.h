#ifndef Entity_h
#define Entity_h

#include "Matrix.h"
#include "Vector.h"
#include "ShaderProgram.h"
#include <vector>

class Entity {
public:
    
    Entity(){};
    
    Matrix matrix;
    
    Vector position;
    Vector scale;
    float rotation;
    
    float velocity_x;
    float velocity_y;
    
    float acceleration_x;
    float acceleration_y;
    
    float friction_x;
    float friction_y;
    
    float vertices[12] = {
        
        0.5f, 0.5f,
        -0.5f, 0.5f,
        -0.5f, -0.5f,
        
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,

    };
    
    std::vector<Vector> normalVertices = {
        Vector(0.5f, 0.5f),
        Vector(-0.5f, 0.5f),
        Vector(-0.5f, -0.5f),
        
        Vector(-0.5f, -0.5f),
        Vector(0.5f, -0.5f),
        Vector(-0.5f, 0.5f)
    };
    
    float texcoords[12] = {
        0.5f, 0.5f,
        0.5f, 0.5f,
        0.5f, 0.5f,
        
        0.5f, 0.5f,
        0.5f, 0.5f,
        0.5f, 0.5f,
    };
    
    
    void draw(ShaderProgram& program);
    void update (float elapsed);    
    
};


#endif /* Entity_h */
