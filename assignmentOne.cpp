#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image  = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct.\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
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
    
    float lastFrameTicks = 0.0f;
    
    glViewport(0,0,640,360);
    
    ShaderProgram programOne(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    ShaderProgram programTwo(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    ShaderProgram programThree(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint texOne = LoadTexture(RESOURCE_FOLDER"alienPink.png");
    GLuint texTwo = LoadTexture(RESOURCE_FOLDER"alienBlue.png");
    GLuint texThree = LoadTexture(RESOURCE_FOLDER"flowerWhite.png");
    
    Matrix projectionMatrixOne;
    Matrix modelMatrixOne;
    Matrix viewMatrixOne;
    
    Matrix projectionMatrixTwo;
    Matrix modelMatrixTwo;
    Matrix viewMatrixTwo;
    
    Matrix projectionMatrixThree;
    Matrix modelMatrixThree;
    Matrix viewMatrixThree;
    
    modelMatrixOne.Translate(1.0, 0.0, 0.0);
    modelMatrixOne.Scale(0.5, 0.5, 0.0);
    
    modelMatrixTwo.Translate(-1.0, 0.0, 0.0);
    modelMatrixTwo.Scale(0.5, 0.5, 0.0);
    
    modelMatrixThree.Scale(0.5, 0.5, 0.0);
    
    projectionMatrixOne.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    projectionMatrixTwo.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    projectionMatrixThree.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

    glUseProgram(programOne.programID);
    glUseProgram(programTwo.programID);
    glUseProgram(programThree.programID);

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        //glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        programOne.setModelMatrix(modelMatrixOne);
        programOne.setProjectionMatrix(projectionMatrixOne);
        programOne.setViewMatrix(viewMatrixOne);
        
        glBindTexture(GL_TEXTURE_2D,texOne);
        
        float verticesOne[] = {-0.5, -1.0, 0.5, -1.0, 0.5, 1.0, -0.5, -1.0, 0.5, 1.0, -0.5, 1.0};
        glVertexAttribPointer(programOne.positionAttribute, 2, GL_FLOAT, false, 0, verticesOne);
        glEnableVertexAttribArray(programOne.positionAttribute);
        
        float texCoordsOne[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(programOne.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsOne);
        glEnableVertexAttribArray(programOne.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(programOne.positionAttribute);
        glDisableVertexAttribArray(programOne.texCoordAttribute);
        
        programTwo.setModelMatrix(modelMatrixTwo);
        programTwo.setProjectionMatrix(projectionMatrixTwo);
        programTwo.setViewMatrix(viewMatrixTwo);
        
        glBindTexture(GL_TEXTURE_2D,texTwo);
        
        float verticesTwo[] = {-0.5, -1.0, 0.5, -1.0, 0.5, 1.0, -0.5, -1.0, 0.5, 1.0, -0.5, 1.0};
        glVertexAttribPointer(programTwo.positionAttribute, 2, GL_FLOAT, false, 0, verticesTwo);
        glEnableVertexAttribArray(programTwo.positionAttribute);
        
        float texCoordsTwo[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(programTwo.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsTwo);
        glEnableVertexAttribArray(programTwo.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(programTwo.positionAttribute);
        glDisableVertexAttribArray(programTwo.texCoordAttribute);
        
        programThree.setModelMatrix(modelMatrixThree);
        programThree.setProjectionMatrix(projectionMatrixThree);
        programThree.setViewMatrix(viewMatrixThree);
        
        glBindTexture(GL_TEXTURE_2D,texThree);
        
        float verticesThree[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(programThree.positionAttribute, 2, GL_FLOAT, false, 0, verticesThree);
        glEnableVertexAttribArray(programThree.positionAttribute);
        
        float texCoordsThree[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(programThree.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsThree);
        glEnableVertexAttribArray(programThree.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        modelMatrixThree.Rotate((90.0*(3.141592653589793238462643/180))*elapsed);
        
        glDisableVertexAttribArray(programThree.positionAttribute);
        glDisableVertexAttribArray(programThree.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);

        
    }
    
    SDL_Quit();
    return 0;
}
