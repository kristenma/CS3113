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

SDL_Window* displayWindow;

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6


int mapWidth;
int mapHeight;
unsigned char** levelData;
float TILE_SIZE = 0.5;
int SPRITE_COUNT_X = 16;
int SPRITE_COUNT_Y = 8;
vector<Entity*> entities;



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


void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX, int spriteCountY, GLuint textureID) {
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
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



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
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}





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
            } }
    }
    return true;
}



void placeEntity(string& type, float x, float y) {
    Entity* entity;
    if (type == "player") {
        entity->entityType = ENTITY_PLAYER;
        entity->isStatic = false;
    }
    else if (type == "enemy") {
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


void drawTiles(ShaderProgram *program, GLuint spriteSheetTex) {
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    int amtToDraw = 0;
    
    for(int y=0; y < mapHeight; y++) {
        for(int x=0; x < mapWidth; x++) {
            if (levelData[y][x]!= 0) {
                
                amtToDraw += 1;
                
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
    
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, spriteSheetTex);
    glDrawArrays(GL_TRIANGLES, 0, amtToDraw*6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


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







int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    SDL_Event event;
    bool done = false;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    
    GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"spritesheet_rgba.png");
    SheetSprite playerSprite (spriteSheetTexture, 32.0f/128.0f, 16.0f/128.0f, 23.0f/192.0f, 23.0f/128.0f, 0.5f);
    SheetSprite enemySprite (spriteSheetTexture, 25.0f/256.0f, 12.0f/128.0f, 23.0f/256.0f, 23.0f/128.0f, 0.5f);
    
    ifstream infile(RESOURCE_FOLDER"mymap.txt");
    string line;
    while (getline(infile, line)) {
        if(line == "[header]") {
            if(!readHeader(infile)) {
                return 0; }
        } else if(line == "[layer]") {
            readLayerData(infile);
        } else if(line == "[ObjectsLayer]") {
            readEntityData(infile);
        }
    }
    
    Matrix projectionMatrix;
    Matrix viewMatrix;
    Matrix modelMatrix;
    
    projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    
    
    for (int i = 0; i < entities.size(); i++) {
        if (entities[i]->entityType == ENTITY_PLAYER) {
            entities[i]->sprite = playerSprite;
            entities[i]->width = playerSprite.width;
            entities[i]->height = playerSprite.height;
            entities[i]->velocity_x = 0;
            entities[i]->velocity_y = 0;
            entities[i]->acceleration_x = 0;
            entities[i]->acceleration_y = 0;

        }
        else if (entities[i]->entityType == ENTITY_ENEMY) {
            entities[i]->sprite = enemySprite;
            entities[i]->width = enemySprite.width;
            entities[i]->height = enemySprite.height;
            entities[i]->velocity_x = 0;
            entities[i]->velocity_y = 0;
            entities[i]->acceleration_x = 0;
            entities[i]->acceleration_y = 0;
        }
    }
    
    
    program.setProjectionMatrix(projectionMatrix);
    
    float lastFrameTicks = 0.0f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!done) {
        
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        float fixedElapsed = elapsed;
        if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
            fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
        }
        while (fixedElapsed >= FIXED_TIMESTEP) {
            fixedElapsed -= FIXED_TIMESTEP;
        }
        
        float px = 0;
        float pvy = 0;
        float pax = 0;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            
            else if (event.type == SDL_KEYDOWN) {
                for (int i = 0; i < entities.size(); i++) {
                    if (entities[i]->entityType == ENTITY_PLAYER) {
                        if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT || event.key.keysym.scancode == SDL_SCANCODE_SEMICOLON || event.key.keysym.scancode == SDL_SCANCODE_D) {
                            pax = 5;
                        }
                        else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT || event.key.keysym.scancode == SDL_SCANCODE_K || event.key.keysym.scancode == SDL_SCANCODE_A) {
                            pax = -5;
                        }
                        else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                            pvy = 3.1;
                        }

                    }
                }
            
            }
            else if (event.type == SDL_KEYUP) {
                for (int i = 0; i < entities.size(); i++) {
                    if (entities[i]->entityType == ENTITY_PLAYER) {
                        if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT || event.key.keysym.scancode == SDL_SCANCODE_LEFT || event.key.keysym.scancode == SDL_SCANCODE_SEMICOLON || event.key.keysym.scancode == SDL_SCANCODE_D || event.key.keysym.scancode == SDL_SCANCODE_K || event.key.keysym.scancode == SDL_SCANCODE_A) {
                                pax = 0;
                        } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                            pvy = 0;
                        }
                    }
                }
            }
            
            for (int i = 0; i < entities.size(); ++i) {
                //if (entities[i]->entityType == ENTITY_PLAYER) {
                //    entities[i]->acceleration_x = pax;
                //    if (entities[i]->collidedBottom) {
                //        entities[i]->velocity_y = pvy;
                //    }
                //}
                
                //if (!entities[i]->isStatic) {
                //    entities[i]->acceleration_y = -5;
                //}
                //entities[i]->collidedTop = false;
                //entities[i]->collidedBottom = false;
                //entities[i]->collidedLeft = false;
                //entities[i]->collidedRight = false;
                
                entities[i]->Update(FIXED_TIMESTEP);
                entities[i]->Render(&program, modelMatrix);
            }
            
            
            for (int i = 0; i < entities.size(); ++i) {
                if (entities[i]->entityType == ENTITY_PLAYER) {
                    for (int j = 0; j < entities.size(); ++j) {
                        if (entities[j]->entityType == ENTITY_ENEMY && entities[i]->collidesWith(entities[j])) {
                            entities.erase(entities.begin() + j);
                            break;
                        }
                    }
                    break;
                }
            }
            
            glClear(GL_COLOR_BUFFER_BIT);
            for (int y = 0; y < mapHeight; ++y) {
                for (int x = 0; x < mapWidth; ++x) {
                    modelMatrix.identity();
                    modelMatrix.Translate(x*TILE_SIZE, (mapHeight - y - 1)*TILE_SIZE, 0);
                    program.setModelMatrix(modelMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    program.setViewMatrix(viewMatrix);
                    DrawSpriteSheetSprite(&program, levelData[y][x], 20, 10, spriteSheetTexture);
                }
            }
            
            for (int i = 0; i < entities.size(); ++i) {
                modelMatrix.identity();
                modelMatrix.Translate(entities[i]->x, entities[i]->y + mapHeight*TILE_SIZE, 0);
                
                program.setModelMatrix(modelMatrix);
                program.setProjectionMatrix(projectionMatrix);
                program.setViewMatrix(viewMatrix);
                if (entities[i]->entityType == ENTITY_PLAYER) {
                    DrawSpriteSheetSprite(&program, 60, 20, 1, spriteSheetTexture);
                    
                    modelMatrix.identity();
                    
                    modelMatrix.Translate(entities[i]->x, entities[i]->y + mapHeight*TILE_SIZE + TILE_SIZE, 0);
                    
                    program.setModelMatrix(modelMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    program.setViewMatrix(viewMatrix);
                    DrawSpriteSheetSprite(&program, 40, 20, 1, spriteSheetTexture);
                    
                    viewMatrix.identity();
                    viewMatrix.Translate(-entities[i]->x, -1 * (entities[i]->y + mapHeight*TILE_SIZE), 0);
                }

            }			
            SDL_GL_SwapWindow(displayWindow);
            break;
        
        }
    }
    
    SDL_Quit();
    return 0;
}
