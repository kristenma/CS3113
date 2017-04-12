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

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
int mapWidth;
int mapHeight;
unsigned char** levelData;
float TILE_SIZE = 0.5f;
int SPRITE_COUNT_X = 30;
int SPRITE_COUNT_Y = 30;
vector<Entity*> entities;

/////-----BOOLEAN VALUES-----

bool done = false;
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };
int state = STATE_GAME_LEVEL;
bool moveLeft = false;
bool moveRight = false;
bool jump = false;

/////-----SPRITESHEETS-----

GLuint font;
GLuint spritesheet;


float fullScreenVertices[] = {
    -3.55f, -2.0f,
    3.55f, -2.0f,
    3.55f, 2.0f,
    -3.55f, -2.0f,
    3.55f, 2.0f,
    -3.55f, 2.0f
};

float wholeTexCoords[] = {
    0.0, 1.0,
    1.0, 1.0,
    1.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    0.0, 0.0
};

/////-----MATRICES-----

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

/////-----TIME-----

float lastFrameTicks = 0.0f;
float ticks;
float elapsed;

/////-----EVENTS-----

SDL_Event event;


/////-----LOAD TEXTURE-----

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return retTexture;
}

void draw(GLuint texture, float vertices[], ShaderProgram program, float texCoords[]){
    glBindTexture(GL_TEXTURE_2D, texture);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}

void DrawSpriteSheetSprite(ShaderProgram program, int index, int spriteCountX, int spriteCountY, GLuint textureID) {
    float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
    float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
    
    float spriteWidth = 1.0 / (float)spriteCountX;
    float spriteHeight = 1.0 / (float)spriteCountY;
    
    GLfloat texCoords[] = {
        u, v + spriteHeight,
        u + spriteWidth, v,
        u, v,
        u + spriteWidth, v,
        u, v + spriteHeight,
        u + spriteWidth, v + spriteHeight
    };
    
    float vertices[] = { -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE };
    
    
    //glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    //glEnableVertexAttribArray(program.positionAttribute);
    //glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    //glEnableVertexAttribArray(program.texCoordAttribute);
    
    //glBindTexture(GL_TEXTURE_2D, textureID);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //glDisableVertexAttribArray(program.positionAttribute);
    //glDisableVertexAttribArray(program.texCoordAttribute);
    
    draw(textureID, vertices, program, texCoords);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}




/////-----READ TILE MAP-----

bool readHeader(std::ifstream &stream) {
    string line;
    mapWidth = -1;
    mapHeight = -1;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "width") {
            mapWidth = atoi(value.c_str());
        } else if(key == "height"){
            mapHeight = atoi(value.c_str());
        } }
    if(mapWidth == -1 || mapHeight == -1) {
        return false;
    } else { // allocate our map data
        levelData = new unsigned char*[mapHeight];
        for(int i = 0; i < mapHeight; ++i) {
            levelData[i] = new unsigned char[mapWidth];
        }
        return true;
    }
}


bool readLayerData(std::ifstream &stream) {
    string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data") {
            for(int y=0; y < mapHeight; y++) {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                for(int x=0; x < mapWidth; x++) {
                    getline(lineStream, tile, ',');
                    unsigned char val =  (unsigned char)atoi(tile.c_str());
                    if(val > 0) {
                        // be careful, the tiles in this format are indexed from 1 not 0
                        levelData[y][x] = val-1;
                    } else {
                        levelData[y][x] = 0;
                    }
                }
            }
        }
    }
    return true;
}



void placeEntity(string type, float x, float y) {
    Entity* entity;
    if (type == "p") {
        entity->entityType = ENTITY_PLAYER;
        entity->isStatic = false;
    }
    else if (type == "e") {
        entity->entityType = ENTITY_ENEMY;
        entity->isStatic = true;
    }
    entity->x = x;
    entity->y = y;
    entities.push_back(entity);
}



bool readEntityData(std::ifstream &stream) {
    string line;
    string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
        } else if(key == "location") {
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            float placeX = atoi(xPosition.c_str())*TILE_SIZE;
            float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
            placeEntity(type, placeX, placeY);
        }
    }
    return true;
}



void readFile() {
    std::ifstream infile("mymap.txt");
    std::string line;
    while (getline(infile, line)) {
        if (line == "[header]") {
            if (!readHeader(infile)) {
                return;
            }
        }
        else if (line == "[layer]") {
            readLayerData(infile);
        }
        else if (line == "[Object Layer 1]") {
            readEntityData(infile);
        }
    }
    
    for (int i = 0; i < entities.size(); i++) {
        cout << i << endl;
    }
    return;
}


/////-----SETUP-----

ShaderProgram Setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    font = LoadTexture(RESOURCE_FOLDER"font1.png");
    spritesheet = LoadTexture(RESOURCE_FOLDER"spritesheet_rgba.png");
    Entity* player = new Entity;
    player->sprite = SheetSprite(spritesheet, 0/1024.0f, 0/1024.0f, 72/512.0f, 97/512.0f, 0.5);
    player->y = 5.0f;
    player->findEdges();
    entities.push_back(player);
    projectionMatrix.setOrthoProjection(-3.0, 3.0, -2.0f, 2.0f, -1.0f, 1.0f);
    ShaderProgram program(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
    glUseProgram(program.programID);
    
    readFile();
    
    program.setModelMatrix(modelMatrix);
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);
    return program;
}


void drawTiles(ShaderProgram program) {
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    int draw = 0;
    
    for(int y=0; y < mapHeight; y++) {
        for(int x=0; x < mapWidth; x++) {
            if (levelData[y][x]!= 0) {
                
                draw +=1;
                
                float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
                float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
                
                float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
                vertexData.insert(vertexData.end(), {
                    TILE_SIZE * x, -TILE_SIZE * y,
                    TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                    (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                    TILE_SIZE * x, -TILE_SIZE * y,
                    (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                    (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y
                });
                texCoordData.insert(texCoordData.end(), {
                    u, v,
                    u, v+(spriteHeight),
                    u+spriteWidth, v+(spriteHeight),
                    u, v,
                    u+spriteWidth, v+(spriteHeight),
                    u+spriteWidth, v
                });
            }
        }
    }
    
    glUseProgram(program.programID);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, spritesheet);
    glDrawArrays(GL_TRIANGLES, 0, draw*6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}


////----------- PROCESS EVENTS ---------------

void ProcessMainMenu() {
    // our SDL event loop
    // check input events
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
            
        }
        else if (event.type == SDL_KEYDOWN){
            state = STATE_GAME_LEVEL;
        }
    }
}

void ProcessGame() {
    // our SDL event loop
    // check input events
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        else if (event.type == SDL_KEYDOWN){
            if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                moveLeft = true;
                for (int i = 0; i < entities.size(); i++) {
                    if (entities[i]->entityType == ENTITY_PLAYER) {
                        entities[i]->velocity_x = -2.0f;
                    }
                    break;
                }
                
                
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                moveRight = true;
                for (int i = 0; i < entities.size(); i++) {
                    if (entities[i]->entityType == ENTITY_PLAYER) {
                        entities[i]->velocity_x = 2.0f;
                    }
                    break;
                }
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                jump = true;
                for (int i = 0; i < entities.size(); i++) {
                    if (entities[i]->entityType == ENTITY_PLAYER) {
                        entities[i]->velocity_y = -2.0f;
                    }
                    break;
                }
            }
        }
        else if (event.type == SDL_KEYUP){
            if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                moveLeft = false;
                
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                moveRight = false;
                
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                jump = false;
                
            }
        }
    }
}

void ProcessGameOver(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
            
        }
    }
}

void ProcessEvents() {
    // our SDL event loop
    // check input events
    switch (state) {
        case STATE_MAIN_MENU:
            ProcessMainMenu();
            break;
        case STATE_GAME_LEVEL:
            ProcessGame();
            break;
        case STATE_GAME_OVER:
            ProcessGameOver();
            break;
        default:
            break;
    }
}


/////-----RENDERING-----

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


void RenderMainMenu(ShaderProgram program){
    // for all main menu elements
    // setup transforms, render sprites
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

void RenderGameLevel(ShaderProgram program){
    // for all game elements
    // setup transforms, render sprites
    modelMatrix.identity();
    program.setModelMatrix(modelMatrix);
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);

    drawTiles(program);
    
    modelMatrix.identity();
    program.setModelMatrix(modelMatrix);
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);

    DrawSpriteSheetSprite(program, 21 , SPRITE_COUNT_X, SPRITE_COUNT_X, spritesheet);
    
    modelMatrix.identity();
    program.setModelMatrix(modelMatrix);
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);
    
    for (int i = 0; i < entities.size(); i++) {
        if (entities[i]->entityType == ENTITY_PLAYER) {
            modelMatrix.Translate(entities[i]->x, entities[i]->y, 0);
            program.setModelMatrix(modelMatrix);
            entities[i]->sprite.Draw(program);
            viewMatrix.identity();
            viewMatrix.Translate(-entities[i]->x, -entities[i]->y, 0);
            program.setViewMatrix(viewMatrix);

        }
    }
        /*
     Does not recognize the position of the entity inside the vector
     
     for(int i=0; i < entities.size(); i++) {
     modelMatrix.identity();
     std::cout << entities[i].position.x << std::endl;
     modelMatrix.Translate(entities[i].position.x, entities[i].position.y, 0);
     program.setModelMatrix(modelMatrix);
     entities[i].sprite.Draw(program);
     
     
     }*/
    
    
}

void RenderGameOver(ShaderProgram program){
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}


void Render(ShaderProgram program) {
    cout << "Entered Render" << "\n";
    glClear(GL_COLOR_BUFFER_BIT);
    switch(state) {
        case STATE_MAIN_MENU:
            cout << "Enter Main Menu Render" << "\n";
            RenderMainMenu(program);
            break;
        case STATE_GAME_LEVEL:
            RenderGameLevel(program);
            break;
        case STATE_GAME_OVER:
            RenderGameOver(program);
            break;
    }
}


////---------------- MOVEMENT & COLLISION -----------------------

//void UpdateMainMenu(){
// move stuff and check for collisions
//}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
    *gridX = (int)(worldX / TILE_SIZE);
    *gridY = (int)(-worldY / TILE_SIZE);
}




void tileCollision (Entity* ent) {
    int gridX, gridY;
    
    worldToTileCoordinates(ent->x + TILE_SIZE/2, ent->y, &gridX, &gridY);
    if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18) {
        ent->x += ent->x + TILE_SIZE / 2 - gridX * TILE_SIZE - TILE_SIZE / 2;
        ent->velocity_x = 0;
        ent->acceleration_x = 0;
        
        ent->collidedRight = true;
    }
    else {ent->collidedRight = false;}
    
    worldToTileCoordinates(ent->x - ent->sprite.size/2, ent->y, &gridX, &gridY);
    if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18) {
        ent->velocity_x = 0;
        ent->acceleration_x = 0;
        ent->x += ent->x + 0.05f + TILE_SIZE /2 - TILE_SIZE * gridX;
        
        ent->collidedLeft = true;
    }
    else {ent->collidedLeft = false;}
    
    
    worldToTileCoordinates(ent->x, ent->y + TILE_SIZE/2, &gridX, &gridY);
    if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18) {
        
        ent->y -= ent->y + TILE_SIZE/2 - ((-TILE_SIZE*gridY) - TILE_SIZE);
        ent->velocity_y = 0;
        
        ent->collidedTop = true;
    }
    else {ent->collidedTop = false;}
    
    worldToTileCoordinates(ent->x, ent->y - TILE_SIZE/2, &gridX, &gridY);
    if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18) {
        ent->velocity_y = 0;
        ent->y += (-TILE_SIZE*gridY) - (ent->y - TILE_SIZE/2);
        ent->collidedBottom = true;
    }
    else {ent->collidedBottom = false;}
}


void UpdateGameLevel(){
    
    for (int i = 0; i < entities.size(); i++) {
        if (entities[i]->entityType == ENTITY_PLAYER) {
            entities[i]->velocity_y += entities[i]->gravity * elapsed;
            entities[i]->y += entities[i]->velocity_y * elapsed;
            if (moveLeft || moveRight){
                entities[i]->x += entities[i]->velocity_x * elapsed;
            }
            int gridX;
            int gridY;
            
            worldToTileCoordinates(entities[i]->x, entities[i]->y, &gridX, &gridY);
            cout << "grid before: " << gridX << " " << gridY << std::endl;
            if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight)){
                cout << "grid after: " << gridX << " " << gridY << std::endl;
                entities[i]->velocity_y = 0;
            }

        }
    }
}



void Update() {
    // move stuff and check for collisions
    ticks = (float) SDL_GetTicks()/1000.0f;
    elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    
    switch(state) {
        case STATE_MAIN_MENU:
            //UpdateMainMenu();
            break;
        case STATE_GAME_LEVEL:
            UpdateGameLevel();
            break;
    }
}



void Cleanup() {
    SDL_Quit();
    
}

int main() {
    ShaderProgram prog = Setup();
    while(!done) {
        ProcessEvents();
        Update();
        Render(prog);
        SDL_GL_SwapWindow(displayWindow);
    }
    Cleanup();
    return 0;
}

