#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#include "ShaderProgram.h"
#include "Matrix.h"
#include "Vector.h"
#include "Entity.h"

using namespace std;


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

/////-----SETUP-----

SDL_Window* displayWindow;

std::vector<Entity> entities;
Matrix projectionMatrix;
Matrix viewMatrix;
Matrix modelMatrix;


/////-----SAT COLLISION-----

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2, Vector &penetration) {
    float normalX = -edgeY;
    float normalY = edgeX;
    float len = sqrtf(normalX*normalX + normalY*normalY);
    normalX /= len;
    normalY /= len;
    
    std::vector<float> e1Projected;
    std::vector<float> e2Projected;
    
    for(int i=0; i < points1.size(); i++) {
        e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
    }
    for(int i=0; i < points2.size(); i++) {
        e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
    }
    
    std::sort(e1Projected.begin(), e1Projected.end());
    std::sort(e2Projected.begin(), e2Projected.end());
    
    float e1Min = e1Projected[0];
    float e1Max = e1Projected[e1Projected.size()-1];
    float e2Min = e2Projected[0];
    float e2Max = e2Projected[e2Projected.size()-1];
    
    float e1Width = fabs(e1Max-e1Min);
    float e2Width = fabs(e2Max-e2Min);
    float e1Center = e1Min + (e1Width/2.0);
    float e2Center = e2Min + (e2Width/2.0);
    float dist = fabs(e1Center-e2Center);
    float p = dist - ((e1Width+e2Width)/2.0);
    
    if(p >= 0) {
        return false;
    }
    
    float penetrationMin1 = e1Max - e2Min;
    float penetrationMin2 = e2Max - e1Min;
    
    float penetrationAmount = penetrationMin1;
    if(penetrationMin2 < penetrationAmount) {
        penetrationAmount = penetrationMin2;
    }
    
    penetration.x = normalX * penetrationAmount;
    penetration.y = normalY * penetrationAmount;
    
    return true;
}

bool penetrationSort(Vector &p1, Vector &p2) {
    return p1.length() < p2.length();
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points, Vector &penetration) {
    std::vector<Vector> penetrations;
    for(int i=0; i < e1Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e1Points.size()-1) {
            edgeX = e1Points[0].x - e1Points[i].x;
            edgeY = e1Points[0].y - e1Points[i].y;
        } else {
            edgeX = e1Points[i+1].x - e1Points[i].x;
            edgeY = e1Points[i+1].y - e1Points[i].y;
        }
        Vector penetration;
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
        if(!result) {
            return false;
        }
        penetrations.push_back(penetration);
    }
    for(int i=0; i < e2Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e2Points.size()-1) {
            edgeX = e2Points[0].x - e2Points[i].x;
            edgeY = e2Points[0].y - e2Points[i].y;
        } else {
            edgeX = e2Points[i+1].x - e2Points[i].x;
            edgeY = e2Points[i+1].y - e2Points[i].y;
        }
        Vector penetration;
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
        
        if(!result) {
            return false;
        }
        penetrations.push_back(penetration);
    }
    
    std::sort(penetrations.begin(), penetrations.end(), penetrationSort);
    penetration = penetrations[0];
    
    Vector e1Center;
    for(int i=0; i < e1Points.size(); i++) {
        e1Center.x += e1Points[i].x;
        e1Center.y += e1Points[i].y;
    }
    e1Center.x /= (float)e1Points.size();
    e1Center.y /= (float)e1Points.size();
    
    Vector e2Center;
    for(int i=0; i < e2Points.size(); i++) {
        e2Center.x += e2Points[i].x;
        e2Center.y += e2Points[i].y;
    }
    e2Center.x /= (float)e2Points.size();
    e2Center.y /= (float)e2Points.size();
    
    Vector ba;
    ba.x = e1Center.x - e2Center.x;
    ba.y = e1Center.y - e2Center.y;
    
    if( (penetration.x * ba.x) + (penetration.y * ba.y) < 0.0f) {
        penetration.x *= -1.0f;
        penetration.y *= -1.0f;
    }
    
    return true;
}


/////-----OBJECT TO WORLD COORDINATES-----

//used Vector operator * instead

//Vector toWorldSpace (const Matrix& modelMatrix, Vector& vec){
//    float x = vec.x;
//    float y = vec.y;
//    x = modelMatrix.m[0][0] * vec.x + modelMatrix.m[0][1] * vec.y + modelMatrix.m[2][0] * 1.0f + modelMatrix.m[3][0] * 1.0f;
//    y = modelMatrix.m[1][0] * vec.x + modelMatrix.m[1][1] * vec.y + modelMatrix.m[2][1] * 1.0f + modelMatrix.m[3][1] * 1.0f;
//    return Vector(x,y);
//}

bool checkCollision(Entity shape1, Entity shape2){
    //call checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) with
    //vertices from shape1 and shape2 put into world space
    std::vector<Vector> edges1;
    std::vector<Vector> edges2;
    Vector penetration;
    
    std::vector<Vector> reset = {
        Vector(0.5f, 0.5f),
        Vector(-0.5f, 0.5f),
        Vector(-0.5f, -0.5f),
        
        Vector(-0.5f, -0.5f),
        Vector(0.5f, -0.5f),
        Vector(-0.5f, 0.5f)
    };
        
    for (int i = 0; i < shape1.normalVertices.size(); i++){
        edges1.push_back(shape1.normalVertices[i]*shape1.matrix);
    }
    for (int i = 0; i < shape2.normalVertices.size(); i++){
        edges2.push_back(shape2.normalVertices[i]*shape2.matrix);
    }
    
    if (checkSATCollision(shape1.normalVertices, shape2.normalVertices, penetration)) {
        shape1.position.x += (penetration.x / 2.0);
        shape1.position.y += (penetration.y / 2.0);
        shape1.acceleration_x *= -1.0f;
        shape1.acceleration_y *= -1.0f;

        shape2.position.x += (penetration.x / 2.0);
        shape2.position.y += (penetration.y / 2.0);
        shape2.acceleration_x *= -1.0f;
        shape2.acceleration_y *= -1.0f;
    }
    
    return checkSATCollision(edges1, edges2, penetration);
}


float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("colliding squares", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    SDL_Event event;
    bool done = false;
    float lastFrameTicks = 0.0f;
    glViewport(0, 0, 640, 360);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    ShaderProgram program(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl");
    projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);
    
    
    //initialize entities
    Entity entityOne;
    entities.push_back(entityOne);
    Entity entityTwo;
    entities.push_back(entityTwo);
    Entity entityThree;
    entities.push_back(entityThree);

    
    entityOne.matrix = modelMatrix;
    entityOne.matrix.identity();
    entityOne.matrix.Translate(20.0f, 0.0f, 0);
    entityOne.matrix.Scale(0.5f, 0.5f, 0);
    program.setModelMatrix(entityOne.matrix);
    entityOne.draw(program);
    entityOne.scale.x = 0.5f;
    entityOne.scale.y = 0.5f;
    entityOne.position.x = 20.0f;
    entityOne.position.y = 0.0f;
    entityOne.rotation = M_PI/15;
    entityOne.acceleration_x = 0.5f;
    entityOne.acceleration_y = 0.0f;
    
    entityTwo.matrix = modelMatrix;
    entityTwo.matrix.identity();
    entityTwo.matrix.Translate(-20.0f, 0.0f, 0);
    entityTwo.matrix.Scale(0.5f, 0.5f, 0);
    program.setModelMatrix(entityTwo.matrix);
    entityTwo.draw(program);
    entityTwo.scale.x = 0.5f;
    entityTwo.scale.y = 0.5f;
    entityTwo.position.x = -20.0f;
    entityTwo.position.y = 0.0f;
    entityTwo.rotation = M_PI/4;
    entityTwo.acceleration_x = 0.2f;
    entityTwo.acceleration_y = 0.0f;
    
    entityThree.matrix = modelMatrix;
    entityThree.matrix.identity();
    entityThree.matrix.Scale(0.5f, 0.5f, 0);
    entityThree.matrix.Translate(0.0f, 0.0f, 0.0f);
    program.setModelMatrix(entityThree.matrix);
    entityThree.draw(program);
    entityThree.position.x = 0.0f;
    entityThree.position.y = 0.0f;
    entityThree.rotation = -M_PI/4;
    entityThree.scale.x = 0.5f;
    entityThree.scale.y = 0.5f;
    entityThree.acceleration_x = 0.7f;
    entityThree.acceleration_y = 0.0f;

    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        #define FIXED_TIMESTEP 0.0166666f
        #define MAX_TIMESTEPS 6
        
        
        float fixedElapsed = elapsed;
        if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
            fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
        }
        while (fixedElapsed >= FIXED_TIMESTEP) {
            fixedElapsed -= FIXED_TIMESTEP;
            entityOne.update(FIXED_TIMESTEP);
            entityTwo.update(FIXED_TIMESTEP);
            entityThree.update(FIXED_TIMESTEP);
        }
        
        
        entityOne.update(fixedElapsed);
        entityTwo.update(fixedElapsed);
        entityThree.update(fixedElapsed);
        
        
        
        int maxChecks = 10;
        while(checkCollision(entityOne, entityTwo) && maxChecks > 0) {
            Vector responseVector = Vector(entityOne.position.x - entityTwo.position.x, entityOne.position.y - entityTwo.position.y);
            responseVector.normalize();

            entityOne.position.x += responseVector.x * 0.0002;
            entityOne.position.y += responseVector.y * 0.0002;
            entityTwo.position.x -= responseVector.x * 0.0002;
            entityTwo.position.y -= responseVector.y * 0.0002;
            maxChecks -= 1;
            //
        }
        
        maxChecks = 10;
        while(checkCollision(entityOne, entityThree) && maxChecks > 0) {
            Vector responseVector = Vector(entityOne.position.x - entityThree.position.x, entityOne.position.y - entityThree.position.y);
            responseVector.normalize();
            
            entityOne.position.x += responseVector.x * 0.0002;
            entityOne.position.y += responseVector.y * 0.0002;
            entityThree.position.x -= responseVector.x * 0.0002;
            entityThree.position.y -= responseVector.y * 0.0002;
            maxChecks -= 1;
            //
        }
        maxChecks = 10;
        while(checkCollision(entityTwo, entityThree) && maxChecks > 0) {
            Vector responseVector = Vector(entityThree.position.x - entityTwo.position.x, entityThree.position.y - entityTwo.position.y);
            responseVector.normalize();

            entityThree.position.x += responseVector.x * 0.0002;
            entityThree.position.y += responseVector.y * 0.0002;
            entityTwo.position.x -= responseVector.x * 0.0002;
            entityTwo.position.y -= responseVector.y * 0.0002;
            maxChecks -= 1;
            //
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        //for (int i = 0; i < entities.size(); i++) {
            //entities[i].velocity_x += entities[i].acceleration_x * elapsed;
            //entities[i].position.x += entities[i].velocity_x * elapsed;
            //modelMatrix.identity();
            //modelMatrix.Translate(entities[i].position.x, entities[i].position.y, 0);
            //modelMatrix.Rotate(entities[i].rotation);
            //program.setModelMatrix(modelMatrix);

            //entities[i].draw(program);
            //viewMatrix.identity();
            //viewMatrix.Translate(-entities[0].position.x, -1 * (entities[0].position.y), 0);
        //}
        
        entityOne.matrix.identity();
        entityOne.matrix.Translate(entityOne.position.x*elapsed, entityOne.position.y*elapsed, 0.0f);
        entityOne.matrix.Rotate(entityOne.rotation);
        entityOne.matrix.Scale(entityOne.scale.x, entityOne.scale.y, 0.0f);
        program.setModelMatrix(entityOne.matrix);
        entityOne.draw(program);

       
        entityTwo.matrix.identity();
        entityTwo.matrix.Translate(entityTwo.position.x*elapsed, entityTwo.position.y*elapsed, 0.0f);
        entityTwo.matrix.Rotate(entityTwo.rotation);
        entityTwo.matrix.Scale(entityTwo.scale.x, entityTwo.scale.y, 0.0f);
        program.setModelMatrix(entityTwo.matrix);
        entityTwo.draw(program);
        
        
        entityThree.matrix.identity();
        entityThree.matrix.Translate(entityThree.position.x*elapsed, entityThree.position.y*elapsed, 0.0f);
        entityThree.matrix.Rotate(entityThree.rotation);
        entityThree.matrix.Scale(entityThree.scale.x, entityThree.scale.y, 0.0f);
        program.setModelMatrix(entityThree.matrix);
        entityThree.draw(program);
        
    
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}

