#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include "Entity.h"
using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


GLuint LoadTexture(Entity* ent) {
    //int w,h,comp;
    //unsigned char* image  = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    //if (image == NULL) {
    //        std::cout << "Unable to load image. Make sure the path is correct.\n";
    //        assert(false);
    //    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ent->width, ent->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    //stbi_image_free(image);
    return retTexture;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0,0,640,360);
    
    ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    
    
    Mix_Chunk *paddleCollision;
    paddleCollision = Mix_LoadWAV(RESOURCE_FOLDER"metal_bang.wav");
    
    Mix_Chunk *gameOver;
    gameOver = Mix_LoadWAV(RESOURCE_FOLDER"strong_punch.wav");
    

    Mix_Music *music;
    music = Mix_LoadMUS(RESOURCE_FOLDER"animal_crossing.mp3");
    
    Mix_PlayMusic(music, -1);
    
    //entity class initial
    
    Entity paddleOne(0.1, 1.0, -3.45, 0, 1.0);
    Entity paddleTwo(0.1, 1.0, 3.45, 0, 1.0);
    Entity ball(0.1, 0.1, 0, 0, 2.0);
    Entity top(7, 0.1, 1.95, 0, 0);
    Entity bottom(7, 0.1, -1.95, 0, 0);
    
    ball.direction_x = 1.0;
    ball.direction_y = 1.0;
    
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    SDL_Event event;
    
    float lastFrameTicks = 0.0f;
    
    bool done = false;
    while (!done) {
    
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        while (SDL_PollEvent(&event)) {
            
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        //input keys for paddles
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        if(keys[SDL_SCANCODE_DOWN]) {
            paddleTwo.y -= elapsed * paddleTwo.speed;
        } else if(keys[SDL_SCANCODE_UP]) {
            paddleTwo.y += elapsed * paddleTwo.speed;
        }
        
        if(keys[SDL_SCANCODE_S]) {
            paddleOne.y -= elapsed * paddleOne.speed;
        }else if(keys[SDL_SCANCODE_W]) {
            paddleOne.y += elapsed * paddleOne.speed;
        }
        
        //paddle collision
        
        if(paddleOne.y > 2.0-paddleOne.height/2-top.height) {
            paddleOne.y = 2.0-paddleOne.height/2-top.height;
        } else if (paddleOne.y < -2.0+paddleOne.height/2+bottom.height){
            paddleOne.y = -2.0+paddleOne.height/2+bottom.height;
        }
        
        if(paddleTwo.y > 2.0-paddleTwo.height/2-top.height) {
            paddleTwo.y = 2.0-paddleTwo.height/2-top.height;
        } else if (paddleTwo.y < -2.0+paddleTwo.height/2+bottom.height){
            paddleTwo.y = -2.0+paddleTwo.height/2+bottom.height;
        }
        
        
        //ball movement
        
        if((ball.y+(ball.height/2))  > (2.0-top.height)) {
            ball.direction_y = -1;
            Mix_PlayChannel( -1, paddleCollision, 0);
        }
        if ((ball.y-(ball.height/2)) < (-2.0+top.height)) {
            ball.direction_y = 1;
            Mix_PlayChannel( -1, paddleCollision, 0);
        }
        
        if(ball.x > 3.35) {
            if ((paddleTwo.y + paddleTwo.height)>(ball.y+ball.height/2) && (paddleTwo.y - paddleTwo.height)<(ball.y-ball.height/2)) {
                ball.direction_x = -1;
                Mix_PlayChannel( -1, paddleCollision, 0);
            }
            else {
                
                paddleOne.y = 0;
                paddleTwo.y = 0;
                ball.x = 0;
                ball.y = 0;
                
                glClearColor(1.0, 1.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);
                
                Mix_PlayChannel( -1, gameOver, 0);
                
                modelMatrix.identity();
                program.setModelMatrix(modelMatrix);
                program.setProjectionMatrix(projectionMatrix);
                program.setViewMatrix(viewMatrix);
            }
        } else if (ball.x < -3.35) {
            if ((paddleOne.y + paddleOne.height)>(ball.y+ball.height/2) && (paddleOne.y - paddleOne.height)<(ball.y-ball.height/2)) {
                ball.direction_x = 1;
                Mix_PlayChannel( -1, paddleCollision, 0);
            }
            else {
                
                paddleOne.y = 0;
                paddleTwo.y = 0;
                ball.x = 0;
                ball.y = 0;
                
                glClearColor(1.0, 1.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);
                Mix_PlayChannel( -1, gameOver, 0);
                
                modelMatrix.identity();
                program.setModelMatrix(modelMatrix);
                program.setProjectionMatrix(projectionMatrix);
                program.setViewMatrix(viewMatrix);
            }
        }
        
        ball.y += ball.direction_y*elapsed*ball.speed;
        ball.x += ball.direction_x*elapsed*ball.speed;
        
        //draw
        
        modelMatrix.identity();
        modelMatrix.Translate(ball.x*elapsed, ball.y*elapsed, 0);
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        ball.Draw(&program);
        
        modelMatrix.identity();
        modelMatrix.Translate(-3.45f, 0, 0);
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        paddleOne.Draw(&program);
        
        modelMatrix.identity();
        modelMatrix.Translate(3.45, 0, 0);
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        paddleTwo.Draw(&program);
        
        modelMatrix.identity();
        modelMatrix.Translate(0, 1.95, 0);
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        top.Draw(&program);
        
        modelMatrix.identity();
        modelMatrix.Translate(0, -1.95, 0);
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        bottom.Draw(&program);
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
