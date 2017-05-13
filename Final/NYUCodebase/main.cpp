#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <time.h>
#include <vector>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <stdlib.h>
//#include <Windows.h>
#include <SDL_mixer.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

using namespace std;
SDL_Window* displayWindow;
bool shot = false;

//----- MATRIX INITIALIZATION -----
Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

void DrawText(ShaderProgram* program, int font, string text, float size, float spacing) {
    
    float texture_size = 1.0 / 16.0f;
    
    vector<float> vertexData;
    vector<float> texCoordData;
    
    for (int i = 0; i < text.size(); ++i) {
        
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    
    glUseProgram(program->programID);
    glBindTexture(GL_TEXTURE_2D, font);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
void lvl2(ShaderProgram* program, GLuint font) {
    viewMatrix.identity();
    modelMatrix.identity();
    modelMatrix.Translate(-3, 0.8, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "WATCH OUT FOR THE ENEMIES", 0.25f, 0.00f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.5, 0.4, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "THEY CAN SHOOT THROUGH", 0.25f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.5, 0.0, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "WALLS BUT YOU CAN'T", 0.25f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3, -1.5, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS [ENTER] TO CONTINUE", 0.25f, 0.0f);
    
    return;
}
void lvl3(ShaderProgram* program, GLuint font) {
    viewMatrix.identity();
    modelMatrix.identity();
    modelMatrix.Translate(-2, 0.4, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "THE SCREEN WILL MOVE", 0.25f, 0.00f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3, 0.0, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "MAKE SURE YOU STAY INSIDE", 0.25f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3, -1.5, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS [ENTER] TO CONTINUE", 0.25f, 0.0f);
    return;
}


GLuint LoadTexture(const char* filePath) {
    
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image.\n";
        //assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}

GLuint LoadBackground(const char* filePath) {
    
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image.\n";
        //assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    return retTexture;
}


// Spritesheet
class SS {
public:
    SS(unsigned int textureID, float size) : textureID(textureID), size(size){
        width = 500.0 / 1024.0;
        height = 500.0 / 1024.0;
    }
    SS(unsigned int textureID, float u, float v, float width, float height, float size)
    : textureID(textureID), u(u / 1024), v(v / 1024), width(width / 1024), height(height / 1024), size(size) {}
    
    float getSize(){ return size; }
    void Draw(ShaderProgram* program) {
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v + height,
            u + width, v,
            u, v,
            u + width, v,
            u, v + height,
            u + width, v + height
        };
        
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, -0.5f * size
        };
        
        glUseProgram(program->programID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
    
    float u;
    float v;
    float size;
    float width;
    float height;
    unsigned int textureID;
};

float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
    float retVal = dstMin + ((value - srcMin) / (srcMax - srcMin) * (dstMax - dstMin));
    if (retVal < dstMin) {
        retVal = dstMin;
    }
    if (retVal > dstMax) {
        retVal = dstMax;
    }
    return retVal;
}

// (Entity)
class GameObject {
public:
    GameObject(float x, float y, SS sprite) : x(x), y(y), sprite(sprite) {}
    
    SS sprite;
    float x;
    float y;
    float prevx;
    float prevy;
    float size = sprite.size;
    
    float defaultspeedx = 1.75f;
    float defaultspeedy = 2.1f;
    float speedx = 0.0f;
    float speedy = 0.0f;
    float maxspeedx = 2.75f;
    float maxspeedy = 3.1f;
    float minspeedx = 0.75f;
    float minspeedy = 1.1f;
    
    float gravity = -1.1f;
    
    float accelx = 1.5f;
    float accely = 1.5f;
    
    string powerup = "null";
    
    float lifetime = 2.0f;
    
    bool dead = false;
    
    bool cTop = false;
    bool cBottom = false;
    bool cLeft = false;
    bool cRight = false;
    bool cPart = false;
    
    bool moveup;
    bool moving;
    bool fall;
    
    bool isstatic = false;
    
};

class Particle {
public:
    Particle(float x, float y, SS sprite) : x(x), y(y), sprite(sprite) {}
    
    SS sprite;
    
    float x;
    float y;
    float size = sprite.size;
    
    float speedx;
    float speedy;
    
    bool squish = false;
    int squash = 0;
    
    string type;
    
    float lifetime = 0.0f;
    float defaultlifetime;
    
};

class ParticleEmitter {
public:
    ParticleEmitter(unsigned int particleCount) : particleCount(particleCount) {};
    
    float x;
    float y;
    
    float gravity = -1.1f;
    int particleCount;
    
    float maxLifetime = 20.0f;
    
    std::vector<Particle> particles;
    
};

float easeIn(float from, float to, float time) {
    float tVal = time*time*time*time*time;
    return (1.0f - tVal)*from + tVal*to;
}

void titleText(ShaderProgram* program, GLuint font) {
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.5f, 1.0f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "SHOOTERS", 0.70f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -0.2f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS 'I' FOR INSTRUCTIONS", 0.20f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -0.8f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS ENTER FOR GAME LEVELS", 0.20f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -1.4f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS Q TO QUIT AT ANY TIME", 0.20f, 0.0f);
    
    return;
}

void instructionText(ShaderProgram* program, GLuint font) {
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, 1.5f, 0.0f);
    viewMatrix.Scale(0.0, 0.0, 1.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "INSTRUCTIONS", 0.35f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, 1.2f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PLAYER 1 USES A-W-D KEYS TO MOVE", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, 0.9f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "AND SPACE TO SHOOT", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, 0.3f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PLAYER 2 USES ARROW KEYS TO MOVE", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, 0.0f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "AND F TO SHOOT", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -0.6f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "YOU CAN ONLY SHOOT IN THE", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -0.9f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "DIRECTION YOU'RE MOVING IN", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -1.5f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS [P] TO CONTINUE", 0.25f, 0.0f);
    
    return;
}
void powerUpText(ShaderProgram* program, GLuint font) {
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, 1.7f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "POWER UPS/DOWNS", 0.35f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.0f, 1.2f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "COLLECT TO SPEED UP", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.0f, 0.6f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "OBTAIN A SHIELD", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.0f, 0.0f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "COLLECT TO SPEED DOWN", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.0f, -0.6f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "STAY FROZEN", 0.15f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -0.9f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "POWERUPS WILL BLINK", 0.20f, 0.0f);
    modelMatrix.identity();
    modelMatrix.Translate(0.0f, -1.2f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "WHEN WEARING OFF", 0.20f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -1.5f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS [ENTER] TO CONTINUE", 0.25f, 0.0f);
    
    return;
}


void levelsText(ShaderProgram* program, GLuint font) {
    modelMatrix.identity();
    modelMatrix.Translate(-2.0f, 1.0f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "LEVELS", 0.70f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, 0.0f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS 1 FOR LEVEL 1", 0.30f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -0.6f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS 2 FOR LEVEL 2", 0.30f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-3.0f, -1.2f, 0.0f);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS 3 FOR LEVEL 3", 0.30f, 0.0f);
    
    return;
}

void playerOneLostText(ShaderProgram* program, GLuint font) {
    viewMatrix.identity();
    modelMatrix.identity();
    modelMatrix.Translate(-1.25, 0.4, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PLAYER 1 LOSES", 0.25f, 0.00f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.75, 0.0, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS [ENTER] TO START", 0.25f, 0.0f);
}

void playerTwoLostText(ShaderProgram* program, GLuint font) {
    viewMatrix.identity();
    modelMatrix.identity();
    modelMatrix.Translate(-1.25, 0.4, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PLAYER 2 LOSES", 0.25f, 0.0f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.75, 0.0, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS [ENTER] TO START", 0.25f, 0.0f);
    
    return;
}

void bothLoseText(ShaderProgram* program, GLuint font) {
    viewMatrix.identity();
    modelMatrix.identity();
    modelMatrix.Translate(-1.25, 0.4, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "YOU BOTH LOSE", 0.25f, 0.00f);
    
    modelMatrix.identity();
    modelMatrix.Translate(-2.75, 0.0, 0.0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, font, "PRESS [ENTER] TO START", 0.25f, 0.0f);
    
    return;
}


//----- CONTAINTERS OF OBJECTS -----
vector<GameObject> enemies;
vector<GameObject> playerBullets;
vector<GameObject> player2Bullets;
vector<GameObject> enemyBullets;
vector<GameObject> blockies;
vector<GameObject> AI;

//----- CREATES ENEMIES -----
void start(ShaderProgram* program, Matrix& modelMatrix, SS enemy, vector<GameObject> b) {
    
    for (int i = b.size() - 1; i >= 0;){
        GameObject enemyObj1(b[i].x, b[i].y + 0.1, enemy);
        //GameObject enemyObj1(0, 0.1, enemy);
        enemyObj1.speedx = 1.0;//0.5;
        enemyObj1.speedy = 0.0;// 0.7;
        enemyObj1.dead = false;
        modelMatrix.identity();
        modelMatrix.Translate(enemyObj1.x, enemyObj1.y, 0.0);
        program->setModelMatrix(modelMatrix);
        enemies.push_back(enemyObj1);
        i -= 27;
    }
    
}
void start3(ShaderProgram* program, Matrix& modelMatrix, SS enemy, vector<GameObject> b) {
    
    for (int i = b.size() - 1; i >= 0;){
        GameObject enemyObj1(b[i].x, b[i].y + 0.1, enemy);
        //GameObject enemyObj1(0, 0.1, enemy);
        enemyObj1.speedx = 1.0;//0.5;
        enemyObj1.speedy = 0.0;// 0.7;
        enemyObj1.dead = false;
        modelMatrix.identity();
        modelMatrix.Translate(enemyObj1.x, enemyObj1.y, 0.0);
        program->setModelMatrix(modelMatrix);
        enemies.push_back(enemyObj1);
        i -= 54;
    }
    
}

vector<GameObject> drawBlocks(SS blocks){
    float randx;
    float randy = -1.8;
    vector <GameObject> blockies;
    for (int i = 0; i < 6; i++){
        randx = -3 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2 + 3))); // low and then high - low
        //randy = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.5 + 1))); // low and then high - low
        float z = 0.7;
        randy += z;
        for (int k = 0; k < 15; k++){
            GameObject blockObj(0.0, -10.0, blocks);
            blockObj.x = randx + (k * blockObj.size);
            blockObj.y = randy;
            blockies.push_back(blockObj);
        }
        if (randx + 2 < 2){
            for (int k = 0; k < 15; k++){
                GameObject blockObj(0.0, -10.0, blocks);
                blockObj.x = randx + (k * blockObj.size) + 3;
                blockObj.y = randy;
                blockies.push_back(blockObj);
            }
        }
        
    }
    return blockies;
}
vector<GameObject> drawBlocks3(SS blocks){
    float randx;
    float randy = -1.5;
    vector <GameObject> blockies;
    for (int i = 0; i < 10; i++){
        randx = -3 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2 + 3))); // low and then high - low
        //randy = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.5 + 1))); // low and then high - low
        float z = 0.7;
        randy += z;
        for (int k = 0; k < 15; k++){
            GameObject blockObj(0.0, -10.0, blocks);
            blockObj.x = randx + (k * blockObj.size);
            blockObj.y = randy;
            blockies.push_back(blockObj);
        }
        if (randx + 3 < 2){
            for (int k = 0; k < 15; k++){
                GameObject blockObj(0.0, -10.0, blocks);
                blockObj.x = randx + (k * blockObj.size) + 3;
                blockObj.y = randy;
                blockies.push_back(blockObj);
            }
        }
    }
    return blockies;
}


GameObject enemyShooting(vector<GameObject> &enemies, SS enemyBullet){
    //int randomEnemy = 0;
    //if (enemies.size() != 0){
    int randomEnemy = rand() % enemies.size();
    while (enemies[randomEnemy].dead == true){
        randomEnemy = rand() % enemies.size();
    }
    //}
    
    GameObject enemyBulletObj(0.0, -10.0, enemyBullet);
    enemyBulletObj.speedy = 2.0;
    enemyBulletObj.x = enemies[randomEnemy].x;
    enemyBulletObj.y = enemies[randomEnemy].y - (enemyBulletObj.size / 2) - (enemies[randomEnemy].size / 2);
    enemyBulletObj.dead = false;
    
    return enemyBulletObj;
    
}

GameObject shoot(GameObject shooter, SS playerBullet){
    GameObject playerBulletObj(0.0, -10.0, playerBullet);
    playerBulletObj.x = shooter.x;
    playerBulletObj.y = shooter.y;
    playerBulletObj.dead = false;
    return playerBulletObj;
}


//----- OBJECT - OBJECT COLLISION -----
bool objectCollision(GameObject objOne, GameObject objTwo) {
    if ((!(objOne.x + (objOne.size / 2) < objTwo.x - (objTwo.size / 1.5)
           || objOne.x - (objOne.size / 2) > objTwo.x + (objTwo.size / 1.5)
           || objOne.y + (objOne.size / 2) < objTwo.y - (objTwo.size / 1.5)
           || objOne.y - (objOne.size / 2) > objTwo.y + (objTwo.size / 1.5)
           ))){
        
        return true;
        
    }
    return false;
}


//----- OBJECT - PARTICLE COLLISION -----
bool particleCollision(GameObject object, Particle particle) {
    if ((!(object.x + (object.size / 2) < particle.x - (particle.size / 1.5)
           || object.x - (object.size / 2) > particle.x + (particle.size / 1.5)
           || object.y + (object.size / 2) < particle.y - (particle.size / 1.5)
           || object.y - (object.size / 2) > particle.y + (particle.size / 1.5)
           ))){
        
        return true;
        
    }
    return false;
}



bool pointVectorCollision(const vector<GameObject> objOne, GameObject obj) {
    for (int i = 0; i < objOne.size(); i++){
        if ((!(objOne[i].x + (objOne[i].size / 2) < obj.x - (obj.size / 1.5)
               || objOne[i].x - (objOne[i].size / 2) > obj.x + (obj.size / 1.5)
               || objOne[i].y + (objOne[i].size / 2) < obj.y - (obj.size / 1.5)
               || objOne[i].y - (objOne[i].size / 2) > obj.y + (obj.size / 1.5)
               
               ))){
            
            return true;
            
        }
        
    }
    return false;
}


int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Space Invader", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 500, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    srand(time(NULL));
    glViewport(0, 0, 800, 500);
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    // Loading texture
    GLuint font = LoadTexture(RESOURCE_FOLDER"font2.png");
    GLuint spriteTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    GLuint block = LoadTexture(RESOURCE_FOLDER"houseBeige.png");
    GLuint player1 = LoadTexture(RESOURCE_FOLDER"player.png");
    GLuint player2ss = LoadTexture(RESOURCE_FOLDER"player2.png");
    GLuint speedup = LoadTexture(RESOURCE_FOLDER"speedup.png");
    GLuint shielding = LoadTexture(RESOURCE_FOLDER"shield.png");
    GLuint slowdown = LoadTexture(RESOURCE_FOLDER"slowdown.png");
    GLuint freeze = LoadTexture(RESOURCE_FOLDER"freeze.png");
    GLuint blink = LoadTexture(RESOURCE_FOLDER"transparent.png");
    
    
    SS player(player1, 0.0, 0.0, 1000.0, 1000.0, 0.4);
    SS player2(player2ss, 0.0, 0.0, 1000.0, 1000.0, 0.4);
    SS enemy(spriteTexture, 224.0, 496.0, 103.0, 84.0, 0.3);
    SS playerBullet(spriteTexture, 856.0, 131.0, 9.0, 37.0, 0.3);
    SS player2Bullet(spriteTexture, 856.0, 131.0, 9.0, 37.0, 0.3);
    SS enemyBullet(spriteTexture, 858.0, 475.0, 9.0, 37.0, 0.3);
    SS blocks(block, 0.0, 0.0, 1000.0, 500.0, 0.1);
    SS speedingup(speedup, 0.0, 0.0, 1000.0, 1000.0, 0.4);
    SS shields(shielding, 0.0, 0.0, 1000.0, 1000.0, 0.4);
    SS slowingdown(slowdown, 0.0, 0.0, 1000.0, 1000.0, 0.4);
    SS froze(freeze, 0.0, 0.0, 1000.0, 1000.0, 0.4);
    SS blinking(blink, 0.0, 0.0, 1000.0, 1000.0, 0.4);
    
    enum GameState { GAME_STATE_TITLE, GAME_STATE_GAME, GAME_STATE_PLAYER1LOST, GAME_STATE_PLAYER2LOST, GAME_STATE_INSTRUCTION, GAME_STATE_CONTROLS, GAME_STATE_LEVEL2_INSTRUCTION, GAME_STATE_LEVEL3_INSTRUCTION, GAME_STATE_POWERUPS, GAME_STATE_LEVELS, GAME_STATE_LEVEL_ONE, GAME_STATE_LEVEL_TWO, GAME_STATE_LEVEL_THREE, GAME_STATE_BOTHDIE };
    
    GameState state = GAME_STATE_TITLE;
    
    
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2, 2, -1.0, 1.0);
    
    glUseProgram(program.programID);
    
    //start(&program, modelMatrix, enemy);
    
    //----- INITIALIZING PLAYERS -----
    GameObject playerObj(0.0f, -1.75f, player);
    playerObj.speedx = 1.75;
    playerObj.speedy = 2.1;
    playerObj.speedx = playerObj.defaultspeedx;
    playerObj.speedy = playerObj.defaultspeedy;
    playerObj.dead = false;
    
    GameObject playerBulletObj(0.0, -10.0, playerBullet);
    playerBulletObj.speedy = 0.0;
    playerBulletObj.dead = true;
    playerBullets.push_back(playerBulletObj);
    
    
    GameObject player2Obj(0.0f, -1.75f, player2);
    player2Obj.speedx = 1.75;
    player2Obj.speedy = 2.1;
    player2Obj.speedx = player2Obj.defaultspeedx;
    player2Obj.speedy = player2Obj.defaultspeedy;
    player2Obj.dead = false;
    
    GameObject player2BulletObj(0.0, -10.0, player2Bullet);
    player2BulletObj.speedy = 0.0;
    player2BulletObj.dead = true;
    player2Bullets.push_back(player2BulletObj);
    
    //----- INITIALIZING POWERUPS -----
    Particle speedUp(0.0, 0.0, speedingup); //POWERUP
    speedUp.type = "speedup";
    speedUp.defaultlifetime = 1.0f;
    speedUp.lifetime = 1.0f;
    speedUp.speedx = -0.2 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.2 + 0.2)));
    Particle shield(0.0, 0.0, shields); //POWERUP
    shield.type = "shield";
    shield.defaultlifetime = 4.0f;
    shield.lifetime = 4.0;
    shield.speedx = -0.2 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.2 + 0.2)));
    
    Particle speedDown(0.0, 0.0, slowingdown); //POWERDOWN
    speedDown.type = "speeddown";
    speedDown.defaultlifetime = 3.0f;
    speedDown.lifetime = 3.0f;
    speedDown.speedx = -0.2 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.2 + 0.2)));
    Particle frozen(0.0, 0.0, froze); //POWERDOWN
    frozen.type = "frozen";
    frozen.defaultlifetime = 2.0;
    frozen.lifetime = 2.0;
    frozen.speedx = -0.2 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.2 + 0.2)));
    
    ParticleEmitter powerUps(4);
    powerUps.y = 5.0f;
    
    powerUps.particles.push_back(speedUp);
    powerUps.particles.push_back(shield);
    powerUps.particles.push_back(speedDown);
    powerUps.particles.push_back(frozen);
    
    for (int i = 0; i < powerUps.particles.size(); i++) {
        powerUps.particles[i].x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (-3.5 - 3.5)));
        powerUps.particles[i].y = powerUps.y;
        
    }
    
    
    
    float numEnemiesAlive = enemies.size();
    float shootingDelay = 0.0;
    float playerSD = 0.0;
    float player2SD = 0.0;
    float lastFrameTicks = 0.0;
    bool standing = true;
    float enemyDelay = 0.0;
    float transy = 0.0;
    bool screenup = true;
    
    float screenShakeValue = 10.0f;
    float screenShakeSpeed = 20.0f;
    float screenShakeIntensity = 10.0f;
    
    
    SDL_Event event;
    bool done = false;
    
    int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    Mix_Chunk *shootSound;
    shootSound = Mix_LoadWAV(RESOURCE_FOLDER"shoot.wav");
    Mix_Chunk *gameoverSound;
    gameoverSound = Mix_LoadWAV(RESOURCE_FOLDER"gameover.wav");
    Mix_Music *music;
    
    Mix_Chunk *powerUpSound;
    powerUpSound = Mix_LoadWAV(RESOURCE_FOLDER"powerUp.wav");
    
    Mix_Chunk *powerDownSound;
    powerDownSound = Mix_LoadWAV(RESOURCE_FOLDER"powerDown.wav");
    
    Mix_Chunk *startSound;
    startSound = Mix_LoadWAV(RESOURCE_FOLDER"start.wav");
    
    music = Mix_LoadMUS(RESOURCE_FOLDER"Elevator-music.mp3");
    Mix_PlayMusic(music, -1);
    
    int p1clicks = 0;
    int p2clicks = 0;
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        
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
        GLuint stars = LoadTexture(RESOURCE_FOLDER"stars.png");
        SS star(stars, 0.0, 0.0, 2000, 1000, 10.0);
        
        
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        modelMatrix.identity();
        program.setModelMatrix(modelMatrix);
        star.Draw(&program);
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        
        if (keys[SDL_SCANCODE_Q]) {
            done = true;
        }
        
        if (state == GAME_STATE_TITLE || state == GAME_STATE_PLAYER1LOST || state == GAME_STATE_PLAYER2LOST || state == GAME_STATE_BOTHDIE) {
            for (int i = 0; i < enemies.size(); i++){
                enemies[i].dead = false;
            }
            
            playerObj.powerup = "null";
            player2Obj.powerup = "null";
            
            for (int i = 0; i < powerUps.particles.size(); i++) {
                powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                powerUps.particles[i].y = powerUps.y;
                powerUps.particles[i].squish = false;
                powerUps.particles[i].squash = 0.0f;
                
                modelMatrix.identity();
                modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                modelMatrix.Scale(1.0, 1.0, 0.0);
                program.setModelMatrix(modelMatrix);
            }
            
            playerObj.x = 0.0f;
            playerObj.y = -1.75f;
            player2Obj.x = 0.0f;
            player2Obj.y = -1.75f;
            
            playerObj.sprite = player;
            player2Obj.sprite = player2;
            
            
            viewMatrix.identity();
            transy = 0.0;
            if (keys[SDL_SCANCODE_I]){
                state = GAME_STATE_INSTRUCTION;
            }
            
            
            if (keys[SDL_SCANCODE_RETURN]) {
                
                state = GAME_STATE_LEVELS;
            }
            
        }
        else if (state == GAME_STATE_INSTRUCTION){
            
            if (keys[SDL_SCANCODE_P]) {
                state = GAME_STATE_POWERUPS;
            }
        }
        else if (state == GAME_STATE_POWERUPS){
            
            if (keys[SDL_SCANCODE_RETURN]) {
                state = GAME_STATE_LEVELS;
            }
        }
        
        else if (state == GAME_STATE_LEVELS) {
            playerObj.moveup = true;
            player2Obj.moveup = true;
            
            
            if (enemies.size() != 0){
                for (int i = 0; i < enemies.size(); ++i) {
                    enemies[i].dead = true;
                    
                }
                enemies.clear();
            }
            
            playerBullets.clear();
            player2Bullets.clear();
            enemyBullets.clear();
            playerObj.x = 0.0;
            playerObj.y = -1.75;
            
            player2Obj.x = 1.0;
            player2Obj.y = -1.75;
            
            if (keys[SDL_SCANCODE_1]){
                state = GAME_STATE_LEVEL_ONE;
                blockies = drawBlocks(blocks);
                start(&program, modelMatrix, enemy, blockies);
                Mix_PlayChannel(-1, startSound, 0);
                
            }
            else if (keys[SDL_SCANCODE_2]){
                //state = GAME_STATE_LEVEL_TWO;
                state = GAME_STATE_LEVEL2_INSTRUCTION;
                blockies = drawBlocks(blocks);
                start(&program, modelMatrix, enemy, blockies);
                for (int i = 0; i < enemies.size(); i++){
                    GameObject a(0.0f, -1.75f, player2);
                    AI.push_back(a);
                }
                Mix_PlayChannel(-1, startSound, 0);
            }
            else if (keys[SDL_SCANCODE_3]){
                //state = GAME_STATE_LEVEL_THREE;
                state = GAME_STATE_LEVEL3_INSTRUCTION;
                blockies = drawBlocks3(blocks);
                start3(&program, modelMatrix, enemy, blockies);
                for (int i = 0; i < enemies.size(); i++){
                    GameObject a(0.0f, -1.75f, player2);
                    AI.push_back(a);
                }
                Mix_PlayChannel(-1, startSound, 0);
            }
        }
        else if (state == GAME_STATE_LEVEL2_INSTRUCTION){
            lvl2(&program, font);
            if (keys[SDL_SCANCODE_RETURN]){
                state = GAME_STATE_LEVEL_TWO;
                
            }
        }
        else if (state == GAME_STATE_LEVEL3_INSTRUCTION){
            lvl3(&program, font);
            if (keys[SDL_SCANCODE_RETURN]){
                state = GAME_STATE_LEVEL_THREE;
                
            }
        }
        else if (state == GAME_STATE_LEVEL_ONE) {
            
            playerObj.moving = false;
            player2Obj.moving = false;
            playerObj.speedx = playerObj.defaultspeedx;
            playerObj.speedy = playerObj.defaultspeedy;
            player2Obj.speedx = player2Obj.defaultspeedx;
            player2Obj.speedy = player2Obj.defaultspeedy;
            
            
            
            for (int i = 0; i < blockies.size(); i++){
                modelMatrix.identity();
                modelMatrix.Translate(blockies[i].x, blockies[i].y, 0.0);
                program.setModelMatrix(modelMatrix);
                blockies[i].sprite.Draw(&program);
            }
            
            
            
            if (keys[SDL_SCANCODE_D]) {
                
                if (playerObj.powerup == "speedup") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    playerObj.speedx += playerObj.accelx * elapsed;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    playerObj.speedx -= playerObj.accelx * elapsed;
                    
                }
                if (playerObj.powerup != "frozen") {
                    playerObj.moving = true;
                    playerObj.prevx = playerObj.x;
                    if (playerObj.x < 3.25){
                        player2Obj.speedx += player2Obj.accelx * elapsed;
                        playerObj.x += elapsed * playerObj.speedx;
                    }
                }
                
                
            }
            else if (keys[SDL_SCANCODE_A]) {
                if (playerObj.powerup == "speedup") {
                    
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    playerObj.speedx -= playerObj.accelx * elapsed;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    playerObj.speedx += playerObj.accelx * elapsed;
                    
                }
                
                if (playerObj.powerup != "frozen") {
                    playerObj.moving = true;
                    playerObj.prevx = playerObj.x;
                    if (playerObj.x > -3.25) {
                        playerObj.speedx -= player2Obj.accelx * elapsed;
                        playerObj.x -= elapsed * playerObj.speedx;
                    }
                }
            }
            else if (keys[SDL_SCANCODE_W]) {
                
                if (playerObj.powerup == "speedup") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    playerObj.speedy += playerObj.accely * elapsed;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    playerObj.speedy -= playerObj.accely * elapsed;
                }
                
                if (playerObj.powerup != "frozen" || playerObj.powerup == "null") {
                    playerObj.moving = true;
                    
                    
                    if (playerObj.cBottom) {
                        playerObj.speedy += playerObj.accely * elapsed;
                        playerObj.y += elapsed * playerObj.speedy;
                    }
                    
                    if (playerObj.prevy < playerObj.y) {
                        playerObj.moveup = true;
                    }
                    
                    else if (playerObj.prevy > playerObj.y && !playerObj.cBottom){
                        playerObj.moving = false;
                        playerObj.moveup = false;
                    }
                    
                    if (!playerObj.cTop && playerObj.moveup){
                        playerObj.speedy += playerObj.accely * elapsed;
                        playerObj.y += elapsed * playerObj.speedy;
                    }
                    
                    playerObj.prevy = playerObj.y;
                    playerObj.prevx = playerObj.x;
                }
            }
            
            p1clicks += 1;
            
            if (p1clicks < 200) {
                if (playerObj.powerup != "null") {
                    if (100 <= p1clicks && p1clicks <= 120) {
                        playerObj.sprite = blinking;
                    }
                    else if (121 <= p1clicks && p1clicks <= 140) {
                        playerObj.sprite = player;
                    }
                    else if (141 <= p1clicks && p1clicks <= 160) {
                        playerObj.sprite = blinking;
                    }
                    else if (161 <= p1clicks && p1clicks <= 180) {
                        playerObj.sprite = player;
                    }
                    else if (181 <= p1clicks && p1clicks < 200) {
                        playerObj.sprite = blinking;
                    }
                    
                }
            }
            else if (p1clicks == 200) {
                playerObj.powerup = "null";
                p1clicks = 0;
                playerObj.sprite = player;
            }
            
            modelMatrix.identity();
            modelMatrix.Translate(playerObj.x, playerObj.y, 0.0);
            program.setModelMatrix(modelMatrix);
            playerObj.sprite.Draw(&program);
            
            if (keys[SDL_SCANCODE_LEFT]) {
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    player2Obj.speedx -= player2Obj.accelx * elapsed;
                    
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    player2Obj.speedx += player2Obj.accelx * elapsed;
                    
                }
                
                if (player2Obj.powerup != "frozen") {
                    player2Obj.moving = true;
                    player2Obj.prevx = player2Obj.x;
                    if (player2Obj.x > -3.25) {
                        player2Obj.speedx -= player2Obj.accelx * elapsed;
                        player2Obj.x -= elapsed * player2Obj.speedx;
                    }
                }
            }
            else if (keys[SDL_SCANCODE_RIGHT]) {
                
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    player2Obj.speedx += player2Obj.accelx * elapsed;
                    
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    player2Obj.speedx -= player2Obj.accelx * elapsed;
                    
                    
                }
                if (player2Obj.powerup != "frozen") {
                    player2Obj.moving = true;
                    player2Obj.prevx = player2Obj.x;
                    if (player2Obj.x < 3.25){
                        player2Obj.speedx += player2Obj.accelx * elapsed;
                        player2Obj.x += elapsed * player2Obj.speedx;
                    }
                }
            }
            
            else if (keys[SDL_SCANCODE_UP]) {
                
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    player2Obj.speedy += player2Obj.accely * elapsed;
                    
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    player2Obj.speedy -= player2Obj.accely * elapsed;
                    
                }
                
                if (player2Obj.powerup != "frozen" || player2Obj.powerup == "null") {
                    
                    player2Obj.moving = true;
                    if (player2Obj.cBottom) {
                        player2Obj.speedy += player2Obj.accely * elapsed;
                        player2Obj.y += elapsed * player2Obj.speedy;
                    }
                    if (player2Obj.prevy < player2Obj.y) {
                        player2Obj.moveup = true;
                    }
                    
                    else if (player2Obj.prevy > player2Obj.y && !player2Obj.cBottom){
                        player2Obj.moving = false;
                        player2Obj.moveup = false;
                    }
                    if (!player2Obj.cTop && player2Obj.moveup){
                        player2Obj.speedy += player2Obj.accely * elapsed;
                        player2Obj.y += elapsed * player2Obj.speedy;
                    }
                    player2Obj.prevy = player2Obj.y;
                    player2Obj.prevx = player2Obj.x;
                    
                }
                
            }
            
            p2clicks += 1;
            
            if (p2clicks < 200) {
                if (player2Obj.powerup != "null") {
                    if (100 <= p2clicks && p2clicks <= 120) {
                        player2Obj.sprite = blinking;
                    }
                    else if (121 <= p2clicks && p2clicks <= 140) {
                        player2Obj.sprite = player2;
                    }
                    else if (141 <= p2clicks && p2clicks <= 160) {
                        player2Obj.sprite = blinking;
                    }
                    else if (161 <= p2clicks && p2clicks <= 180) {
                        player2Obj.sprite = player2;
                    }
                    else if (181 <= p2clicks && p2clicks < 200) {
                        player2Obj.sprite = blinking;
                    }
                    
                }
            }
            
            else if (p2clicks == 200) {
                player2Obj.powerup = "null";
                p2clicks = 0;
                player2Obj.sprite = player2;
            }
            
            modelMatrix.identity();
            modelMatrix.Translate(player2Obj.x, player2Obj.y, 0.0);
            program.setModelMatrix(modelMatrix);
            player2Obj.sprite.Draw(&program);
            
            if (keys[SDL_SCANCODE_F]) {
                if (playerObj.powerup != "frozen") {
                    
                    if (playerSD > 0.15){
                        Mix_PlayChannel(-1, shootSound, 0);
                        playerSD = 0.0;
                        
                        //DEPENDING ON WHICH WAY THEY'RE GOING, THEY SHOOT IN THAT DIRECTION
                        if (playerObj.x == playerObj.prevx){ // meaning they're going up/down
                            if (playerObj.y > playerObj.prevy){
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedy = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                            else{
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedy = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                        }
                        else {
                            if (playerObj.x > playerObj.prevx){
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedx = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                            
                            else{
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedx = -5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                        }
                    }
                }
            }
            
            if (keys[SDL_SCANCODE_SPACE]) {
                if (playerObj.powerup != "frozen") {
                    if (player2SD > 0.15){
                        Mix_PlayChannel(-1, shootSound, 0);
                        player2SD = 0.0;
                        
                        //DEPENDING ON WHICH WAY THEY'RE GOING, THEY SHOOT IN THAT DIRECTION
                        if (player2Obj.x == player2Obj.prevx){ // meaning they're going up/down
                            if (player2Obj.y > player2Obj.prevy){
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedy = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                            else{
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedy = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                        }
                        else {
                            if (player2Obj.x > player2Obj.prevx){
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedx = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                            
                            else{
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedx = -5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                        }
                    }
                }
            }
            
        }
        else if (state == GAME_STATE_LEVEL_TWO) {
            
            playerObj.moving = false;
            player2Obj.moving = false;
            playerObj.speedx = playerObj.defaultspeedx;
            playerObj.speedy = playerObj.defaultspeedy;
            player2Obj.speedx = player2Obj.defaultspeedx;
            player2Obj.speedy = player2Obj.defaultspeedy;
            
            
            
            
            for (int i = 0; i < blockies.size(); i++){
                modelMatrix.identity();
                modelMatrix.Translate(blockies[i].x, blockies[i].y, 0.0);
                program.setModelMatrix(modelMatrix);
                blockies[i].sprite.Draw(&program);
            }
            
            
            
            if (keys[SDL_SCANCODE_D]) {
                
                if (playerObj.powerup == "speedup") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    
                }
                if (playerObj.powerup != "frozen") {
                    playerObj.moving = true;
                    playerObj.prevx = playerObj.x;
                    if (playerObj.x < 3.25){
                        playerObj.x += elapsed * playerObj.speedx;
                    }
                }
                
                
            }
            else if (keys[SDL_SCANCODE_A]) {
                if (playerObj.powerup == "speedup") {
                    
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    
                }
                
                if (playerObj.powerup != "frozen") {
                    playerObj.moving = true;
                    playerObj.prevx = playerObj.x;
                    if (playerObj.x > -3.25) {
                        playerObj.x -= elapsed * playerObj.speedx;
                    }
                }
            }
            else if (keys[SDL_SCANCODE_W]) {
                
                if (playerObj.powerup == "speedup") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    
                }
                
                if (playerObj.powerup != "frozen" || playerObj.powerup == "null") {
                    playerObj.moving = true;
                    
                    
                    if (playerObj.cBottom) {
                        playerObj.y += elapsed * playerObj.speedy;
                    }
                    
                    if (playerObj.prevy < playerObj.y) {
                        playerObj.moveup = true;
                    }
                    
                    else if (playerObj.prevy > playerObj.y && !playerObj.cBottom){
                        playerObj.moving = false;
                        playerObj.moveup = false;
                    }
                    
                    if (!playerObj.cTop && playerObj.moveup){
                        playerObj.y += elapsed * playerObj.speedy;
                    }
                    
                    playerObj.prevy = playerObj.y;
                    playerObj.prevx = playerObj.x;
                }
            }
            
            p1clicks += 1;
            
            if (p1clicks < 200) {
                if (playerObj.powerup != "null") {
                    if (100 <= p1clicks && p1clicks <= 120) {
                        playerObj.sprite = blinking;
                    }
                    else if (121 <= p1clicks && p1clicks <= 140) {
                        playerObj.sprite = player;
                    }
                    else if (141 <= p1clicks && p1clicks <= 160) {
                        playerObj.sprite = blinking;
                    }
                    else if (161 <= p1clicks && p1clicks <= 180) {
                        playerObj.sprite = player;
                    }
                    else if (181 <= p1clicks && p1clicks < 200) {
                        playerObj.sprite = blinking;
                    }
                    
                }
            }
            else if (p1clicks == 200) {
                playerObj.powerup = "null";
                p1clicks = 0;
                playerObj.sprite = player;
            }
            
            
            modelMatrix.identity();
            modelMatrix.Translate(playerObj.x, playerObj.y, 0.0);
            program.setModelMatrix(modelMatrix);
            playerObj.sprite.Draw(&program);
            
            if (keys[SDL_SCANCODE_LEFT]) {
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    
                }
                
                if (player2Obj.powerup != "frozen") {
                    player2Obj.moving = true;
                    player2Obj.prevx = player2Obj.x;
                    if (player2Obj.x > -3.25) {
                        player2Obj.x -= elapsed * player2Obj.speedx;
                    }
                }
            }
            else if (keys[SDL_SCANCODE_RIGHT]) {
                
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    
                }
                if (player2Obj.powerup != "frozen") {
                    player2Obj.moving = true;
                    player2Obj.prevx = player2Obj.x;
                    if (player2Obj.x < 3.25){
                        player2Obj.x += elapsed * player2Obj.speedx;
                    }
                }
            }
            
            else if (keys[SDL_SCANCODE_UP]) {
                
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    
                }
                
                if (player2Obj.powerup != "frozen" || player2Obj.powerup == "null") {
                    
                    player2Obj.moving = true;
                    if (player2Obj.cBottom) {
                        player2Obj.y += elapsed * player2Obj.speedy;
                    }
                    if (player2Obj.prevy < player2Obj.y) {
                        player2Obj.moveup = true;
                    }
                    
                    else if (player2Obj.prevy > player2Obj.y && !player2Obj.cBottom){
                        player2Obj.moving = false;
                        player2Obj.moveup = false;
                    }
                    if (!player2Obj.cTop && player2Obj.moveup){
                        player2Obj.y += elapsed * player2Obj.speedy;
                    }
                    player2Obj.prevy = player2Obj.y;
                    player2Obj.prevx = player2Obj.x;
                    
                }
                
            }
            
            p2clicks += 1;
            
            if (p2clicks < 200) {
                if (player2Obj.powerup != "null") {
                    if (100 <= p2clicks && p2clicks <= 120) {
                        player2Obj.sprite = blinking;
                    }
                    else if (121 <= p2clicks && p2clicks <= 140) {
                        player2Obj.sprite = player2;
                    }
                    else if (141 <= p2clicks && p2clicks <= 160) {
                        player2Obj.sprite = blinking;
                    }
                    else if (161 <= p2clicks && p2clicks <= 180) {
                        player2Obj.sprite = player2;
                    }
                    else if (181 <= p2clicks && p2clicks < 200) {
                        player2Obj.sprite = blinking;
                    }
                    
                }
            }
            
            else if (p2clicks == 200) {
                player2Obj.powerup = "null";
                p2clicks = 0;
                player2Obj.sprite = player2;
            }
            
            modelMatrix.identity();
            modelMatrix.Translate(player2Obj.x, player2Obj.y, 0.0);
            program.setModelMatrix(modelMatrix);
            player2Obj.sprite.Draw(&program);
            
            if (keys[SDL_SCANCODE_F]) {
                if (playerObj.powerup != "frozen") {
                    
                    if (playerSD > 0.15){
                        Mix_PlayChannel(-1, shootSound, 0);
                        playerSD = 0.0;
                        
                        //DEPENDING ON WHICH WAY THEY'RE GOING, THEY SHOOT IN THAT DIRECTION
                        if (playerObj.x == playerObj.prevx){ // meaning they're going up/down
                            if (playerObj.y > playerObj.prevy){
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedy = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                            else{
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedy = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                        }
                        else {
                            if (playerObj.x > playerObj.prevx){
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedx = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                            
                            else{
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedx = -5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                        }
                        
                        
                    }
                }
            }
            
            if (keys[SDL_SCANCODE_SPACE]) {
                if (playerObj.powerup != "frozen") {
                    if (player2SD > 0.15){
                        Mix_PlayChannel(-1, shootSound, 0);
                        player2SD = 0.0;
                        
                        //DEPENDING ON WHICH WAY THEY'RE GOING, THEY SHOOT IN THAT DIRECTION
                        if (player2Obj.x == player2Obj.prevx){ // meaning they're going up/down
                            if (player2Obj.y > player2Obj.prevy){
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedy = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                            else{
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedy = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                        }
                        else {
                            if (player2Obj.x > player2Obj.prevx){
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedx = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                            
                            else{
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedx = -5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                        }
                    }
                }
            }
            
        }
        else if (state == GAME_STATE_LEVEL_THREE) {
            
            playerObj.moving = false;
            player2Obj.moving = false;
            playerObj.speedx = playerObj.defaultspeedx;
            playerObj.speedy = playerObj.defaultspeedy;
            player2Obj.speedx = player2Obj.defaultspeedx;
            player2Obj.speedy = player2Obj.defaultspeedy;
            
            
            
            
            for (int i = 0; i < blockies.size(); i++){
                modelMatrix.identity();
                modelMatrix.Translate(blockies[i].x, blockies[i].y, 0.0);
                program.setModelMatrix(modelMatrix);
                blockies[i].sprite.Draw(&program);
            }
            
            
            
            if (keys[SDL_SCANCODE_D]) {
                
                if (playerObj.powerup == "speedup") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    
                }
                if (playerObj.powerup != "frozen") {
                    playerObj.moving = true;
                    playerObj.prevx = playerObj.x;
                    if (playerObj.x < 3.25){
                        playerObj.x += elapsed * playerObj.speedx;
                    }
                }
                
                
            }
            else if (keys[SDL_SCANCODE_A]) {
                if (playerObj.powerup == "speedup") {
                    
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    
                }
                
                if (playerObj.powerup != "frozen") {
                    playerObj.moving = true;
                    playerObj.prevx = playerObj.x;
                    if (playerObj.x > -3.25) {
                        playerObj.x -= elapsed * playerObj.speedx;
                    }
                }
            }
            else if (keys[SDL_SCANCODE_W]) {
                
                if (playerObj.powerup == "speedup") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                }
                
                else if (playerObj.powerup == "speeddown") {
                    playerObj.speedx = playerObj.maxspeedx;
                    playerObj.speedy = playerObj.maxspeedy;
                    
                }
                
                if (playerObj.powerup != "frozen" || playerObj.powerup == "null") {
                    playerObj.moving = true;
                    
                    
                    if (playerObj.cBottom) {
                        playerObj.y += elapsed * playerObj.speedy;
                    }
                    
                    if (playerObj.prevy < playerObj.y) {
                        playerObj.moveup = true;
                    }
                    
                    else if (playerObj.prevy > playerObj.y && !playerObj.cBottom){
                        playerObj.moving = false;
                        playerObj.moveup = false;
                    }
                    
                    if (!playerObj.cTop && playerObj.moveup){
                        playerObj.y += elapsed * playerObj.speedy;
                    }
                    
                    playerObj.prevy = playerObj.y;
                    playerObj.prevx = playerObj.x;
                }
            }
            
            p1clicks += 1;
            
            if (p1clicks < 200) {
                if (playerObj.powerup != "null") {
                    if (100 <= p1clicks && p1clicks <= 120) {
                        playerObj.sprite = blinking;
                    }
                    else if (121 <= p1clicks && p1clicks <= 140) {
                        playerObj.sprite = player;
                    }
                    else if (141 <= p1clicks && p1clicks <= 160) {
                        playerObj.sprite = blinking;
                    }
                    else if (161 <= p1clicks && p1clicks <= 180) {
                        playerObj.sprite = player;
                    }
                    else if (181 <= p1clicks && p1clicks < 200) {
                        playerObj.sprite = blinking;
                    }
                    
                }
            }
            else if (p1clicks == 200) {
                playerObj.powerup = "null";
                p1clicks = 0;
                playerObj.sprite = player;
            }
            
            
            modelMatrix.identity();
            modelMatrix.Translate(playerObj.x, playerObj.y, 0.0);
            program.setModelMatrix(modelMatrix);
            playerObj.sprite.Draw(&program);
            
            if (keys[SDL_SCANCODE_LEFT]) {
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    
                }
                
                if (player2Obj.powerup != "frozen") {
                    player2Obj.moving = true;
                    player2Obj.prevx = player2Obj.x;
                    if (player2Obj.x > -3.25) {
                        player2Obj.x -= elapsed * player2Obj.speedx;
                    }
                }
            }
            else if (keys[SDL_SCANCODE_RIGHT]) {
                
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    
                }
                if (player2Obj.powerup != "frozen") {
                    player2Obj.moving = true;
                    player2Obj.prevx = player2Obj.x;
                    if (player2Obj.x < 3.25){
                        player2Obj.x += elapsed * player2Obj.speedx;
                    }
                }
            }
            
            else if (keys[SDL_SCANCODE_UP]) {
                
                if (player2Obj.powerup == "speedup") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                }
                
                else if (player2Obj.powerup == "speeddown") {
                    player2Obj.speedx = player2Obj.maxspeedx;
                    player2Obj.speedy = player2Obj.maxspeedy;
                    
                }
                
                if (player2Obj.powerup != "frozen" || player2Obj.powerup == "null") {
                    
                    player2Obj.moving = true;
                    if (player2Obj.cBottom) {
                        player2Obj.y += elapsed * player2Obj.speedy;
                    }
                    if (player2Obj.prevy < player2Obj.y) {
                        player2Obj.moveup = true;
                    }
                    
                    else if (player2Obj.prevy > player2Obj.y && !player2Obj.cBottom){
                        player2Obj.moving = false;
                        player2Obj.moveup = false;
                    }
                    if (!player2Obj.cTop && player2Obj.moveup){
                        player2Obj.y += elapsed * player2Obj.speedy;
                    }
                    player2Obj.prevy = player2Obj.y;
                    player2Obj.prevx = player2Obj.x;
                    
                }
                
            }
            
            p2clicks += 1;
            
            if (p2clicks < 200) {
                if (player2Obj.powerup != "null") {
                    if (100 <= p2clicks && p2clicks <= 120) {
                        player2Obj.sprite = blinking;
                    }
                    else if (121 <= p2clicks && p2clicks <= 140) {
                        player2Obj.sprite = player2;
                    }
                    else if (141 <= p2clicks && p2clicks <= 160) {
                        player2Obj.sprite = blinking;
                    }
                    else if (161 <= p2clicks && p2clicks <= 180) {
                        player2Obj.sprite = player2;
                    }
                    else if (181 <= p2clicks && p2clicks < 200) {
                        player2Obj.sprite = blinking;
                    }
                    
                }
            }
            
            else if (p2clicks == 200) {
                player2Obj.powerup = "null";
                p2clicks = 0;
                player2Obj.sprite = player2;
            }
            
            modelMatrix.identity();
            modelMatrix.Translate(player2Obj.x, player2Obj.y, 0.0);
            program.setModelMatrix(modelMatrix);
            player2Obj.sprite.Draw(&program);
            
            if (keys[SDL_SCANCODE_F]) {
                if (playerObj.powerup != "frozen") {
                    
                    if (playerSD > 0.15){
                        Mix_PlayChannel(-1, shootSound, 0);
                        playerSD = 0.0;
                        
                        //DEPENDING ON WHICH WAY THEY'RE GOING, THEY SHOOT IN THAT DIRECTION
                        if (playerObj.x == playerObj.prevx){ // meaning they're going up/down
                            if (playerObj.y > playerObj.prevy){
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedy = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                            else{
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedy = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                        }
                        else {
                            if (playerObj.x > playerObj.prevx){
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedx = 5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                            
                            else{
                                GameObject playerBulletObj = shoot(playerObj, playerBullet);
                                playerBulletObj.speedx = -5.0;
                                playerBullets.push_back(playerBulletObj);
                            }
                        }
                        
                        
                    }
                }
            }
            
            if (keys[SDL_SCANCODE_SPACE]) {
                if (playerObj.powerup != "frozen") {
                    if (player2SD > 0.15){
                        Mix_PlayChannel(-1, shootSound, 0);
                        player2SD = 0.0;
                        
                        //DEPENDING ON WHICH WAY THEY'RE GOING, THEY SHOOT IN THAT DIRECTION
                        if (player2Obj.x == player2Obj.prevx){ // meaning they're going up/down
                            if (player2Obj.y > player2Obj.prevy){
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedy = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                            else{
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedy = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                        }
                        else {
                            if (player2Obj.x > player2Obj.prevx){
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedx = 5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                            
                            else{
                                GameObject player2BulletObj = shoot(player2Obj, player2Bullet);
                                player2BulletObj.speedx = -5.0;
                                player2Bullets.push_back(player2BulletObj);
                            }
                        }
                    }
                }
            }
            
        }
        float translateSpeed = 0.01;
        
        switch (state) {
            case GAME_STATE_TITLE:
                titleText(&program, font);
                break;
                
            case GAME_STATE_INSTRUCTION:
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.0f, 1.5f, 0.0f);
                modelMatrix.Translate(0.0f, translateSpeed += elapsed, 0.0f);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, font, "INSTRUCTIONS", 0.35f, 0.0f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.0f, 1.2f, 0.0f);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, font, "PLAYER 1 USES ARROW KEYS TO MOVE", 0.15f, 0.0f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.0f, 0.9f, 0.0f);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, font, "AND SPACE TO SHOOT", 0.15f, 0.0f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.0f, 0.3f, 0.0f);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, font, "PLAYER 2 USES A-W-D KEYS TO MOVE", 0.15f, 0.0f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.0f, 0.0f, 0.0f);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, font, "AND F TO SHOOT", 0.15f, 0.0f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.0f, -0.6f, 0.0f);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, font, "YOU CAN ONLY SHOOT IN THE", 0.15f, 0.0f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.0f, -0.9f, 0.0f);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, font, "DIRECTION YOU'RE MOVING IN", 0.15f, 0.0f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.0f, -1.5f, 0.0f);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, font, "PRESS [P] TO CONTINUE", 0.25f, 0.0f);
                
                break;
                
            case GAME_STATE_POWERUPS:
                powerUpText(&program, font);
                
                //-----DRAW POWER UPS -----
                modelMatrix.identity();
                modelMatrix.Translate(-2.5f, 1.2f, 0.0f);
                program.setModelMatrix(modelMatrix);
                speedUp.sprite.Draw(&program);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5f, 0.6f, 0.0f);
                program.setModelMatrix(modelMatrix);
                shield.sprite.Draw(&program);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5f, 0.0f, 0.0f);
                program.setModelMatrix(modelMatrix);
                speedDown.sprite.Draw(&program);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5f, -0.5f, 0.0f);
                program.setModelMatrix(modelMatrix);
                frozen.sprite.Draw(&program);
                break;
                
            case GAME_STATE_LEVELS:
                levelsText(&program, font);
                break;
                
            case GAME_STATE_PLAYER1LOST:
                playerOneLostText(&program, font);
                screenShakeValue += elapsed;
                viewMatrix.Translate(0.0f, sin(screenShakeValue*1.0f)*0.3, 0.0f);
                break;
                
            case GAME_STATE_PLAYER2LOST:
                playerTwoLostText(&program, font);
                screenShakeValue += elapsed;
                viewMatrix.Translate(0.0f, sin(screenShakeValue*1.0f)*0.3, 0.0f);
                break;
                
            case GAME_STATE_BOTHDIE:
                bothLoseText(&program, font);
                screenShakeValue += elapsed;
                viewMatrix.Translate(0.0f, sin(screenShakeValue*1.0f)*0.3, 0.0f);
                break;
                
            case GAME_STATE_LEVEL_ONE:
                
                
                viewMatrix.identity();
                program.setViewMatrix(viewMatrix);
                
                if ((playerObj.y < -transy - 2 || playerObj.y > -transy + 2) && (player2Obj.y < -transy - 2 || player2Obj.y > -transy + 2)) {
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    
                    state = GAME_STATE_BOTHDIE;
                }
                
                else if (playerObj.y < -transy - 2 || playerObj.y > -transy + 2){
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    state = GAME_STATE_PLAYER1LOST;
                }
                else if (player2Obj.y < -transy - 2 || player2Obj.y > -transy + 2){
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    state = GAME_STATE_PLAYER2LOST;
                }
                
                playerObj.cTop = false;
                playerObj.cBottom = false;
                playerObj.cLeft = false;
                playerObj.cRight = false;
                
                player2Obj.cTop = false;
                player2Obj.cBottom = false;
                player2Obj.cLeft = false;
                player2Obj.cRight = false;
                
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.3, 1.8, 0.0);
                program.setModelMatrix(modelMatrix);
                
                for (int i = 0; i < blockies.size(); ++i){
                    //PLAYER OBJ COLLISION WITH BLOCKS
                    if (objectCollision(playerObj, blockies[i])){
                        
                        
                        if ((blockies[i].x - (blockies[i].size / 2) <= playerObj.x + (playerObj.size / 2))){
                            
                            playerObj.cRight = true;
                            blockies[i].cLeft = true;
                        }
                        if (blockies[i].y > playerObj.y){
                            playerObj.cTop = true;
                            blockies[i].cBottom = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - playerObj.y + (playerObj.size / 2));
                            playerObj.y -= 0;
                            
                        }
                        if (blockies[i].x + (blockies[i].size / 2) >= playerObj.x - (playerObj.size / 2)){
                            playerObj.cLeft = true;
                            blockies[i].cRight = true;
                            playerObj.moveup = true;
                            
                        }
                        if (blockies[i].y < playerObj.y){
                            playerObj.cBottom = true;
                            blockies[i].cTop = true;
                            playerObj.y = blockies[i].y + blockies[i].size;
                            playerObj.moveup = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - playerObj.y + (playerObj.size / 2));
                            playerObj.y += penetration + 0.01;
                            
                            
                        }
                        
                    }
                    //PLAYER 2 OBJ COLLISION WITH BLOCKS
                    if (objectCollision(player2Obj, blockies[i])){
                        
                        
                        if ((blockies[i].x - (blockies[i].size / 2) <= player2Obj.x + (player2Obj.size / 2))){
                            
                            player2Obj.cRight = true;
                            blockies[i].cLeft = true;
                        }
                        if (blockies[i].y > player2Obj.y){
                            player2Obj.cTop = true;
                            blockies[i].cBottom = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - player2Obj.y + (player2Obj.size / 2));
                            player2Obj.y -= 0;
                            
                        }
                        if (blockies[i].x + (blockies[i].size / 2) >= player2Obj.x - (player2Obj.size / 2)){
                            player2Obj.cLeft = true;
                            blockies[i].cRight = true;
                            player2Obj.moveup = true;
                            
                        }
                        if (blockies[i].y < player2Obj.y){
                            player2Obj.cBottom = true;
                            blockies[i].cTop = true;
                            player2Obj.y = blockies[i].y + blockies[i].size;
                            player2Obj.moveup = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - player2Obj.y + (player2Obj.size / 2));
                            player2Obj.y += penetration + 0.01;
                            
                            
                        }
                        
                    }
                    
                }
                
                if (playerObj.y > -1.75){ // gravity
                    if (!playerObj.cBottom && !playerObj.moving){
                        playerObj.cTop = false;
                        playerObj.moveup = false;
                        playerObj.prevy = playerObj.y;
                        playerObj.y += playerObj.gravity * elapsed;
                        playerObj.fall = true;
                        playerObj.isstatic = true;
                    }
                    
                }
                
                if (player2Obj.y > -1.75) {
                    if (!player2Obj.cBottom && !player2Obj.moving){
                        player2Obj.cTop = false;
                        player2Obj.moveup = false;
                        player2Obj.prevy = player2Obj.y;
                        player2Obj.y += player2Obj.gravity * elapsed;
                        player2Obj.fall = true;
                        player2Obj.isstatic = true;
                    }
                }
                if (player2Obj.y < -1.7){
                    player2Obj.moveup = true;
                }
                if (playerObj.y < -1.7){
                    playerObj.moveup = true;
                }
                
                
                modelMatrix.identity();
                modelMatrix.Translate(playerObj.x, playerObj.y, 0.0);
                program.setModelMatrix(modelMatrix);
                playerObj.sprite.Draw(&program);
                
                modelMatrix.identity();
                modelMatrix.Translate(player2Obj.x, player2Obj.y, 0.0);
                program.setModelMatrix(modelMatrix);
                player2Obj.sprite.Draw(&program);
                
                for (int i = 0; i < playerBullets.size(); ++i) {
                    playerBullets[i].y += elapsed * playerBullets[i].speedy;
                    playerBullets[i].x += elapsed * playerBullets[i].speedx;
                    modelMatrix.identity();
                    modelMatrix.Translate(playerBullets[i].x, playerBullets[i].y, 0.0);
                    program.setModelMatrix(modelMatrix);
                    if (!playerBullets[i].dead){
                        playerBullets[i].sprite.Draw(&program);
                    }
                    else{
                        playerBullets.erase(playerBullets.begin() + i);
                    }
                }
                for (int i = 0; i < player2Bullets.size(); ++i) {
                    player2Bullets[i].y += elapsed * player2Bullets[i].speedy;
                    player2Bullets[i].x += elapsed * player2Bullets[i].speedx;
                    modelMatrix.identity();
                    modelMatrix.Translate(player2Bullets[i].x, player2Bullets[i].y, 0.0);
                    program.setModelMatrix(modelMatrix);
                    if (!player2Bullets[i].dead){
                        player2Bullets[i].sprite.Draw(&program);
                    }
                    else{
                        player2Bullets.erase(player2Bullets.begin() + i);
                    }
                }
                for (int i = 0; i < playerBullets.size(); i++){
                    if (objectCollision(playerBullets[i], player2Obj)){
                        playerBullets[i].dead = true;
                        playerBullets[i].x = -10.0;
                        playerBullets[i].y = -10.0;
                        playerBullets[i].speedy = 0.0;
                        playerBullets[i].speedx = 1.0;
                        if (player2Obj.powerup != "shield") {
                            player2Obj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            state = GAME_STATE_PLAYER2LOST;
                        }
                    }
                    if (playerBullets[i].y >= 7.0 || playerBullets[i].x < -4.0 || playerBullets[i].x > 4.0) {
                        playerBullets[i].dead = true;
                        playerBullets[i].x = -10.0;
                        playerBullets[i].y = -10.0;
                        playerBullets[i].speedy = 0.0;
                        playerBullets[i].speedx = 1.0;
                    }
                    for (int j = 0; j < blockies.size(); ++j){
                        if (objectCollision(playerBullets[i], blockies[j])){
                            
                            playerBullets[i].dead = true;
                            playerBullets[i].x = -10.0;
                            playerBullets[i].y = -10.0;
                            playerBullets[i].speedy = 0.0;
                            playerBullets[i].speedx = 1.0;
                        }
                    }
                }
                
                for (int i = 0; i < player2Bullets.size(); i++){
                    if (objectCollision(player2Bullets[i], playerObj)){
                        player2Bullets[i].dead = true;
                        player2Bullets[i].x = -10.0;
                        player2Bullets[i].y = -10.0;
                        player2Bullets[i].speedy = 0.0;
                        player2Bullets[i].speedx = 1.0;
                        
                        if (playerObj.powerup != "shield") {
                            playerObj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER1LOST;
                            
                        }
                    }
                    if (player2Bullets[i].y >= 7.0 || player2Bullets[i].x < -4.0 || player2Bullets[i].x > 4.0) {
                        player2Bullets[i].dead = true;
                        player2Bullets[i].x = -10.0;
                        player2Bullets[i].y = -10.0;
                        player2Bullets[i].speedy = 0.0;
                        player2Bullets[i].speedx = 1.0;
                    }
                    for (int j = 0; j < blockies.size(); ++j){
                        if (objectCollision(blockies[j], player2Bullets[i])){
                            
                            player2Bullets[i].dead = true;
                            player2Bullets[i].x = -10.0;
                            player2Bullets[i].y = -10.0;
                            player2Bullets[i].speedy = 0.0;
                            player2Bullets[i].speedx = 1.0;
                        }
                    }
                    
                }
                
                shootingDelay += elapsed;
                playerSD += elapsed;
                player2SD += elapsed;
                
                
                //-----POWER UPS-----
                modelMatrix.identity();
                program.setModelMatrix(modelMatrix);
                for (int i = 0; i < powerUps.particles.size(); ++i) {
                    powerUps.particles[i].squish = false;
                    powerUps.particles[i].y += elapsed * powerUps.gravity;
                    powerUps.particles[i].lifetime += elapsed;
                    
                    if (powerUps.particles[i].lifetime >= powerUps.maxLifetime) {
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].squish = false;
                        
                        powerUps.particles[i].lifetime = powerUps.particles[i].defaultlifetime;
                        
                    }
                    
                    cout << powerUps.particles[i].squash << endl;
                    
                    //----- PARTICLE COLLISION WITH BLOCKS -----
                    for (int j = 0; j < blockies.size(); j++) {
                        if (particleCollision(blockies[j], powerUps.particles[i])) {
                            
                            if (blockies[j].y < powerUps.particles[i].y){
                                
                                blockies[j].cTop = true;
                                powerUps.particles[i].y = blockies[j].y + blockies[j].size;
                                float penetration = fabs(blockies[j].y + (blockies[j].size / 2) - powerUps.particles[i].y + (powerUps.particles[i].size / 2));
                                powerUps.particles[i].y += penetration + 0.01;
                                
                                powerUps.particles[i].x += elapsed * powerUps.particles[i].speedx;
                                powerUps.particles[i].y += elapsed * powerUps.gravity;
                            }
                            
                            if (powerUps.particles[i].squash < 10) {
                                powerUps.particles[i].squish = true;
                                powerUps.particles[i].squash += elapsed;
                                
                                
                            }
                            else if (powerUps.particles[i].squash == 10) {
                                powerUps.particles[i].squish = false;
                                powerUps.particles[i].squash = 0;
                            }
                            
                        }
                        
                    }
                    
                    //----- PARTICLES FALL OFF SCREEN -----
                    if (powerUps.particles[i].y < -2.0) {
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        powerUps.particles[i].squish = false;
                        
                        
                        powerUps.particles[i].lifetime = 2;
                    }
                    
                    //----- PARTICLE COLLISION WITH WALLS -----
                    if (powerUps.particles[i].x > 3.55) {
                        
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].x += powerUps.particles[i].speedx * elapsed;
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        
                    }
                    
                    if (powerUps.particles[i].x < -3.55) {
                        
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].x += powerUps.particles[i].speedx * elapsed;
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        
                    }
                    
                    
                    //----- PLAYER/PARTICLE COLLISION TEST -----
                    if (particleCollision(playerObj, powerUps.particles[i])) {
                        p1clicks = 0;
                        playerObj.sprite = player;
                        playerObj.cPart = true;
                        
                        if (powerUps.particles[i].type == "speedup" || powerUps.particles[i].type == "shield") {
                            Mix_PlayChannel(-1, powerUpSound, 0);
                        }
                        
                        else if (powerUps.particles[i].type == "speeddown" || powerUps.particles[i].type == "frozen") {
                            Mix_PlayChannel(-1, powerDownSound, 0);
                        }
                        
                        playerObj.powerup = powerUps.particles[i].type;
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        
                        powerUps.particles[i].lifetime = 2;
                        
                    }
                    
                    if (particleCollision(player2Obj, powerUps.particles[i])) {
                        p2clicks = 0;
                        player2Obj.sprite = player2;
                        player2Obj.cPart = true;
                        
                        if (powerUps.particles[i].type == "speedup" || powerUps.particles[i].type == "shield") {
                            Mix_PlayChannel(-1, powerUpSound, 0);
                        }
                        
                        if (powerUps.particles[i].type == "speeddown" || powerUps.particles[i].type == "frozen") {
                            Mix_PlayChannel(-1, powerDownSound, 0);
                        }
                        
                        player2Obj.powerup = powerUps.particles[i].type;
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        
                        powerUps.particles[i].lifetime = 2;
                        
                    }
                    
                    //-----DRAW-----
                    if (powerUps.particles[i].squish == true) {
                        float scale_y = mapValue(fabs(1.0), 0.0, 5.0, 0.8, 1.0);
                        float scale_x = mapValue(fabs(1.0), 5.0, 0.0, 1.0, 1.6);
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        modelMatrix.Scale(scale_x, scale_y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        //powerUps.particles[i].sprite.Draw(&program);
                    }
                    else {
                        
                        modelMatrix.identity();
                        modelMatrix.Scale(1.0, 1.0, 0.0);
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        
                    }
                    powerUps.particles[i].sprite.Draw(&program);
                    
                }
                
                //----- PLAYER/PLAYER COLLISION -----
                
                if (objectCollision(playerObj, player2Obj)){
                    playerObj.dead = true;
                    player2Obj.dead = true;
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    
                    state = GAME_STATE_BOTHDIE;
                }
                break;
                
            case GAME_STATE_LEVEL_TWO:
                
                viewMatrix.identity();
                program.setViewMatrix(viewMatrix);
                
                if ((playerObj.y < -transy - 2 || playerObj.y > -transy + 2) && (player2Obj.y < -transy - 2 || player2Obj.y > -transy + 2)) {
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    
                    state = GAME_STATE_BOTHDIE;
                }
                
                else if (playerObj.y < -transy - 2 || playerObj.y > -transy + 2){
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    state = GAME_STATE_PLAYER1LOST;
                }
                else if (player2Obj.y < -transy - 2 || player2Obj.y > -transy + 2){
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    state = GAME_STATE_PLAYER2LOST;
                }
                
                playerObj.cTop = false;
                playerObj.cBottom = false;
                playerObj.cLeft = false;
                playerObj.cRight = false;
                
                player2Obj.cTop = false;
                player2Obj.cBottom = false;
                player2Obj.cLeft = false;
                player2Obj.cRight = false;
                
                
                for (int i = 0; i < enemies.size(); i++){
                    enemies[i].cTop = false;
                    enemies[i].cBottom = false;
                    enemies[i].cLeft = false;
                    enemies[i].cRight = false;
                }
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.3, 1.8, 0.0);
                program.setModelMatrix(modelMatrix);
                
                for (int i = 0; i < blockies.size(); ++i){
                    //PLAYER OBJ COLLISION WITH BLOCKS
                    if (objectCollision(playerObj, blockies[i])){
                        
                        
                        if ((blockies[i].x - (blockies[i].size / 2) <= playerObj.x + (playerObj.size / 2))){
                            
                            playerObj.cRight = true;
                            blockies[i].cLeft = true;
                        }
                        if (blockies[i].y > playerObj.y){
                            playerObj.cTop = true;
                            blockies[i].cBottom = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - playerObj.y + (playerObj.size / 2));
                            playerObj.y -= 0;
                            
                        }
                        if (blockies[i].x + (blockies[i].size / 2) >= playerObj.x - (playerObj.size / 2)){
                            playerObj.cLeft = true;
                            blockies[i].cRight = true;
                            playerObj.moveup = true;
                            
                        }
                        if (blockies[i].y < playerObj.y){
                            playerObj.cBottom = true;
                            blockies[i].cTop = true;
                            playerObj.y = blockies[i].y + blockies[i].size;
                            playerObj.moveup = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - playerObj.y + (playerObj.size / 2));
                            playerObj.y += penetration + 0.01;
                            
                        }
                        
                    }
                    //PLAYER 2 OBJ COLLISION WITH BLOCKS
                    if (objectCollision(player2Obj, blockies[i])){
                        
                        
                        if ((blockies[i].x - (blockies[i].size / 2) <= player2Obj.x + (player2Obj.size / 2))){
                            
                            player2Obj.cRight = true;
                            blockies[i].cLeft = true;
                        }
                        if (blockies[i].y > player2Obj.y){
                            player2Obj.cTop = true;
                            blockies[i].cBottom = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - player2Obj.y + (player2Obj.size / 2));
                            player2Obj.y -= 0;
                            
                        }
                        if (blockies[i].x + (blockies[i].size / 2) >= player2Obj.x - (player2Obj.size / 2)){
                            player2Obj.cLeft = true;
                            blockies[i].cRight = true;
                            player2Obj.moveup = true;
                            
                        }
                        if (blockies[i].y < player2Obj.y){
                            player2Obj.cBottom = true;
                            blockies[i].cTop = true;
                            player2Obj.y = blockies[i].y + blockies[i].size;
                            player2Obj.moveup = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - player2Obj.y + (player2Obj.size / 2));
                            player2Obj.y += penetration + 0.01;
                            
                        }
                        
                    }
                    
                }
                for (int j = 0; j < enemies.size(); j++){
                    
                    if (objectCollision(enemies[j], playerObj)){
                        
                        if (playerObj.powerup != "shield") {
                            enemies[j].dead = true;
                            playerObj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER1LOST;
                        }
                        
                    }
                    
                    if (objectCollision(enemies[j], player2Obj)){
                        if (player2Obj.powerup != "shield") {
                            enemies[j].dead = true;
                            player2Obj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER2LOST;
                        }
                    }
                }
                
                if (playerObj.y > -1.75){ // gravity
                    if (!playerObj.cBottom && !playerObj.moving){
                        playerObj.cTop = false;
                        playerObj.moveup = false;
                        playerObj.prevy = playerObj.y;
                        playerObj.y += playerObj.gravity * elapsed;
                        playerObj.fall = true;
                        playerObj.isstatic = true;
                    }
                    
                }
                
                if (player2Obj.y > -1.75) {
                    if (!player2Obj.cBottom && !player2Obj.moving){
                        player2Obj.cTop = false;
                        player2Obj.moveup = false;
                        player2Obj.prevy = player2Obj.y;
                        player2Obj.y += player2Obj.gravity * elapsed;
                        player2Obj.fall = true;
                        player2Obj.isstatic = true;
                    }
                }
                if (player2Obj.y < -1.7){
                    player2Obj.moveup = true;
                }
                if (playerObj.y < -1.7){
                    playerObj.moveup = true;
                }
                
                for (int i = 0; i < enemies.size(); i++){
                    if (enemies[i].y > -1.75){
                        if (!enemies[i].cBottom){
                            enemies[i].y -= enemies[i].speedy * elapsed;
                        }
                    }
                    
                }
                
                modelMatrix.identity();
                modelMatrix.Translate(playerObj.x, playerObj.y, 0.0);
                program.setModelMatrix(modelMatrix);
                playerObj.sprite.Draw(&program);
                
                modelMatrix.identity();
                modelMatrix.Translate(player2Obj.x, player2Obj.y, 0.0);
                program.setModelMatrix(modelMatrix);
                player2Obj.sprite.Draw(&program);
                
                for (int i = 0; i < playerBullets.size(); ++i) {
                    playerBullets[i].y += elapsed * playerBullets[i].speedy;
                    playerBullets[i].x += elapsed * playerBullets[i].speedx;
                    modelMatrix.identity();
                    modelMatrix.Translate(playerBullets[i].x, playerBullets[i].y, 0.0);
                    program.setModelMatrix(modelMatrix);
                    if (!playerBullets[i].dead){
                        playerBullets[i].sprite.Draw(&program);
                    }
                    else{
                        playerBullets.erase(playerBullets.begin() + i);
                    }
                }
                for (int i = 0; i < player2Bullets.size(); ++i) {
                    player2Bullets[i].y += elapsed * player2Bullets[i].speedy;
                    player2Bullets[i].x += elapsed * player2Bullets[i].speedx;
                    modelMatrix.identity();
                    modelMatrix.Translate(player2Bullets[i].x, player2Bullets[i].y, 0.0);
                    program.setModelMatrix(modelMatrix);
                    if (!player2Bullets[i].dead){
                        player2Bullets[i].sprite.Draw(&program);
                    }
                    else{
                        player2Bullets.erase(player2Bullets.begin() + i);
                    }
                }
                for (int i = 0; i < playerBullets.size(); i++){
                    if (objectCollision(playerBullets[i], player2Obj)){
                        playerBullets[i].dead = true;
                        playerBullets[i].x = -10.0;
                        playerBullets[i].y = -10.0;
                        playerBullets[i].speedy = 0.0;
                        playerBullets[i].speedx = 1.0;
                        if (player2Obj.powerup != "shield") {
                            player2Obj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            state = GAME_STATE_PLAYER2LOST;
                        }
                    }
                    if (playerBullets[i].y >= 7.0 || playerBullets[i].x < -4.0 || playerBullets[i].x > 4.0) {
                        playerBullets[i].dead = true;
                        playerBullets[i].x = -10.0;
                        playerBullets[i].y = -10.0;
                        playerBullets[i].speedy = 0.0;
                        playerBullets[i].speedx = 1.0;
                    }
                    for (int j = 0; j < blockies.size(); ++j){
                        if (objectCollision(playerBullets[i], blockies[j])){
                            
                            playerBullets[i].dead = true;
                            playerBullets[i].x = -10.0;
                            playerBullets[i].y = -10.0;
                            playerBullets[i].speedy = 0.0;
                            playerBullets[i].speedx = 1.0;
                        }
                    }
                    
                    for (int j = 0; j < enemies.size(); ++j) {
                        if (objectCollision(playerBullets[i], enemies[j])) {
                            playerBullets[i].dead = true;
                            playerBullets[i].x = 10.0;
                            playerBullets[i].speedy = 0.0;
                            enemies[j].y = 10.0;
                            enemies[j].dead = true;
                            numEnemiesAlive--;
                        }
                    }
                }
                
                for (int i = 0; i < player2Bullets.size(); i++){
                    if (objectCollision(player2Bullets[i], playerObj)){
                        player2Bullets[i].dead = true;
                        player2Bullets[i].x = -10.0;
                        player2Bullets[i].y = -10.0;
                        player2Bullets[i].speedy = 0.0;
                        player2Bullets[i].speedx = 1.0;
                        
                        if (playerObj.powerup != "shield") {
                            playerObj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER1LOST;
                            
                        }
                    }
                    if (player2Bullets[i].y >= 7.0 || player2Bullets[i].x < -4.0 || player2Bullets[i].x > 4.0) {
                        player2Bullets[i].dead = true;
                        player2Bullets[i].x = -10.0;
                        player2Bullets[i].y = -10.0;
                        player2Bullets[i].speedy = 0.0;
                        player2Bullets[i].speedx = 1.0;
                    }
                    for (int j = 0; j < blockies.size(); ++j){
                        if (objectCollision(blockies[j], player2Bullets[i])){
                            
                            player2Bullets[i].dead = true;
                            player2Bullets[i].x = -10.0;
                            player2Bullets[i].y = -10.0;
                            player2Bullets[i].speedy = 0.0;
                            player2Bullets[i].speedx = 1.0;
                        }
                    }
                    
                    for (int j = 0; j < enemies.size(); ++j) {
                        if (objectCollision(enemies[j], player2Bullets[i])) {
                            player2Bullets[i].dead = true;
                            player2Bullets[i].x = 10.0;
                            player2Bullets[i].speedy = 0.0;
                            enemies[j].y = 10.0;
                            enemies[j].dead = true;
                            numEnemiesAlive--;
                        }
                    }
                }
                for (int i = 0; i < enemies.size(); ++i) {
                    if (!enemies[i].dead) {
                        enemies[i].x += elapsed * enemies[i].speedx;
                        modelMatrix.identity();
                        modelMatrix.Translate(enemies[i].x, enemies[i].y, 0);
                        program.setModelMatrix(modelMatrix);
                        enemies[i].sprite.Draw(&program);
                    }
                }
                
                for (int i = 0; i < enemies.size(); i++) {
                    if (enemies[i].speedx > 0){
                        AI[i].x = enemies[i].x + 0.2;
                        AI[i].y = enemies[i].y - (enemies[i].size / 2) - 0.1;
                    }
                    else{
                        AI[i].x = enemies[i].x - 0.2;
                        AI[i].y = enemies[i].y - (enemies[i].size / 2) - 0.1;
                    }
                    
                    if (enemies[i].x > 3.35) {
                        enemies[i].speedx = -0.75;
                    }
                    else if (enemies[i].x < -3.35) {
                        enemies[i].speedx = 0.75;
                    }
                    
                    if (!pointVectorCollision(blockies, AI[i])){
                        enemies[i].speedx *= -1;
                    }
                }
                
                shootingDelay += elapsed;
                playerSD += elapsed;
                player2SD += elapsed;
                
                if (shootingDelay > 0.95) {
                    
                    GameObject enemyBulletObj = enemyShooting(enemies, enemyBullet);
                    shootingDelay = 0;
                    
                    enemyBullets.push_back(enemyBulletObj);
                }
                
                ///------ENEMY BULLET / PLAYER COLLISION-------
                for (int i = 0; i < enemyBullets.size(); i++){
                    enemyBullets[i].y -= elapsed * enemyBullets[i].speedy;
                    modelMatrix.identity();
                    modelMatrix.Translate(enemyBullets[i].x, enemyBullets[i].y, 0.0);
                    program.setModelMatrix(modelMatrix);
                    
                    if (!enemyBullets[i].dead){
                        enemyBullets[i].sprite.Draw(&program);
                    }
                    
                    else{
                        enemyBullets.erase(enemyBullets.begin() + i);
                    }
                    
                    if (objectCollision(enemyBullets[i], playerObj)) {
                        enemyBullets[i].y = -10.0;
                        enemyBullets[i].speedy = 0.0;
                        // we died
                        
                        numEnemiesAlive = enemies.size();
                        enemyBullets[i].dead = true;
                        if (playerObj.powerup != "shield") {
                            playerObj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER1LOST;
                        }
                        
                    }
                    
                    if (objectCollision(enemyBullets[i], player2Obj)) {
                        enemyBullets[i].y = -10.0;
                        enemyBullets[i].speedy = 0.0;
                        // we died
                        
                        numEnemiesAlive = enemies.size();
                        enemyBullets[i].dead = true;
                        if (player2Obj.powerup != "shield") {
                            player2Obj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER2LOST;
                        }
                    }
                }
                
                //-----POWER UPS-----
                modelMatrix.identity();
                program.setModelMatrix(modelMatrix);
                for (int i = 0; i < powerUps.particles.size(); ++i) {
                    powerUps.particles[i].squish = false;
                    powerUps.particles[i].squash = 0.0f;
                    powerUps.particles[i].y += elapsed * powerUps.gravity;
                    powerUps.particles[i].lifetime += elapsed;
                    powerUps.particles[i].squash += elapsed;
                    
                    if (powerUps.particles[i].lifetime >= powerUps.maxLifetime) {
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].squish = false;
                        
                        powerUps.particles[i].lifetime = powerUps.particles[i].defaultlifetime;
                        
                    }
                    
                    //----- PARTICLE COLLISION WITH BLOCKS -----
                    for (int j = 0; j < blockies.size(); j++) {
                        if (particleCollision(blockies[j], powerUps.particles[i])) {
                            
                            if (blockies[j].y < powerUps.particles[i].y){
                                
                                blockies[j].cTop = true;
                                powerUps.particles[i].y = blockies[j].y + blockies[j].size;
                                float penetration = fabs(blockies[j].y + (blockies[j].size / 2) - powerUps.particles[i].y + (powerUps.particles[i].size / 2));
                                powerUps.particles[i].y += penetration + 0.01;
                                
                                powerUps.particles[i].x += elapsed * powerUps.particles[i].speedx;
                                powerUps.particles[i].y += elapsed * powerUps.gravity;
                            }
                            if (powerUps.particles[i].squash < 2) {
                                powerUps.particles[i].squish = true;
                            }
                            else if (powerUps.particles[i].squash == 2) {
                                powerUps.particles[i].squish = false;
                                powerUps.particles[i].squash = 0;
                            }
                        }
                    }
                    //----- PARTICLES FALL OFF SCREEN -----
                    if (powerUps.particles[i].y < -2.0) {
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        powerUps.particles[i].squish = false;
                        
                        
                        powerUps.particles[i].lifetime = 2;
                    }
                    
                    //----- PARTICLE COLLISION WITH WALLS -----
                    if (powerUps.particles[i].x > 3.55) {
                        
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].x += powerUps.particles[i].speedx * elapsed;
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        
                    }
                    
                    if (powerUps.particles[i].x < -3.55) {
                        
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].x += powerUps.particles[i].speedx * elapsed;
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                    }
                    
                    
                    //----- PLAYER/PARTICLE COLLISION TEST -----
                    if (particleCollision(playerObj, powerUps.particles[i])) {
                        p1clicks = 0;
                        playerObj.sprite = player;
                        playerObj.cPart = true;
                        
                        if (powerUps.particles[i].type == "speedup" || powerUps.particles[i].type == "shield") {
                            Mix_PlayChannel(-1, powerUpSound, 0);
                        }
                        
                        else if (powerUps.particles[i].type == "speeddown" || powerUps.particles[i].type == "frozen") {
                            Mix_PlayChannel(-1, powerDownSound, 0);
                        }
                        
                        playerObj.powerup = powerUps.particles[i].type;
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        
                        powerUps.particles[i].lifetime = 2;
                        
                    }
                    
                    if (particleCollision(player2Obj, powerUps.particles[i])) {
                        p2clicks = 0;
                        player2Obj.sprite = player2;
                        player2Obj.cPart = true;
                        
                        if (powerUps.particles[i].type == "speedup" || powerUps.particles[i].type == "shield") {
                            Mix_PlayChannel(-1, powerUpSound, 0);
                        }
                        
                        if (powerUps.particles[i].type == "speeddown" || powerUps.particles[i].type == "frozen") {
                            Mix_PlayChannel(-1, powerDownSound, 0);
                        }
                        
                        player2Obj.powerup = powerUps.particles[i].type;
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        
                        powerUps.particles[i].lifetime = 2;
                        
                    }
                    
                    //-----DRAW-----
                    if (powerUps.particles[i].squish == true) {
                        float scale_y = mapValue(fabs(1.0), 0.0, 5.0, 0.8, 1.0);
                        float scale_x = mapValue(fabs(1.0), 5.0, 0.0, 1.0, 1.6);
                        
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        modelMatrix.Scale(scale_x, scale_y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        //powerUps.particles[i].sprite.Draw(&program);
                    }
                    else {
                        
                        modelMatrix.identity();
                        modelMatrix.Scale(1.0, 1.0, 0.0);
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        
                    }
                    powerUps.particles[i].sprite.Draw(&program);
                    
                    
                }
                
                //----- PLAYER/PLAYER COLLISION -----
                
                if (objectCollision(playerObj, player2Obj)){
                    playerObj.dead = true;
                    player2Obj.dead = true;
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    
                    state = GAME_STATE_BOTHDIE;
                }
                
                
                break;
                
            case GAME_STATE_LEVEL_THREE:
                
                if (transy >= -4.5 && screenup){
                    transy -= 0.15*elapsed;
                }
                
                else{
                    screenup = false;
                    transy += 0.25*elapsed;
                    if (transy >= 0.0){
                        screenup = true;
                    }
                }
                viewMatrix.identity();
                viewMatrix.Translate(0.0, transy, 0.0);
                program.setViewMatrix(viewMatrix);
                
                if ((playerObj.y < -transy - 2 || playerObj.y > -transy + 2) && (player2Obj.y < -transy - 2 || player2Obj.y > -transy + 2)) {
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    
                    state = GAME_STATE_BOTHDIE;
                }
                
                else if (playerObj.y < -transy - 2 || playerObj.y > -transy + 2){
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    state = GAME_STATE_PLAYER1LOST;
                }
                else if (player2Obj.y < -transy - 2 || player2Obj.y > -transy + 2){
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    state = GAME_STATE_PLAYER2LOST;
                }
                
                playerObj.cTop = false;
                playerObj.cBottom = false;
                playerObj.cLeft = false;
                playerObj.cRight = false;
                
                player2Obj.cTop = false;
                player2Obj.cBottom = false;
                player2Obj.cLeft = false;
                player2Obj.cRight = false;
                
                
                for (int i = 0; i < enemies.size(); i++){
                    enemies[i].cTop = false;
                    enemies[i].cBottom = false;
                    enemies[i].cLeft = false;
                    enemies[i].cRight = false;
                }
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.3, 1.8, 0.0);
                program.setModelMatrix(modelMatrix);
                
                for (int i = 0; i < blockies.size(); ++i){
                    //PLAYER OBJ COLLISION WITH BLOCKS
                    if (objectCollision(playerObj, blockies[i])){
                        
                        
                        if ((blockies[i].x - (blockies[i].size / 2) <= playerObj.x + (playerObj.size / 2))){
                            
                            playerObj.cRight = true;
                            blockies[i].cLeft = true;
                        }
                        if (blockies[i].y > playerObj.y){
                            playerObj.cTop = true;
                            blockies[i].cBottom = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - playerObj.y + (playerObj.size / 2));
                            playerObj.y -= 0;
                            
                        }
                        if (blockies[i].x + (blockies[i].size / 2) >= playerObj.x - (playerObj.size / 2)){
                            playerObj.cLeft = true;
                            blockies[i].cRight = true;
                            playerObj.moveup = true;
                            
                        }
                        if (blockies[i].y < playerObj.y){
                            playerObj.cBottom = true;
                            blockies[i].cTop = true;
                            playerObj.y = blockies[i].y + blockies[i].size;
                            playerObj.moveup = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - playerObj.y + (playerObj.size / 2));
                            playerObj.y += penetration + 0.01;
                        }
                        
                    }
                    //PLAYER 2 OBJ COLLISION WITH BLOCKS
                    if (objectCollision(player2Obj, blockies[i])){
                        
                        
                        if ((blockies[i].x - (blockies[i].size / 2) <= player2Obj.x + (player2Obj.size / 2))){
                            
                            player2Obj.cRight = true;
                            blockies[i].cLeft = true;
                        }
                        if (blockies[i].y > player2Obj.y){
                            player2Obj.cTop = true;
                            blockies[i].cBottom = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - player2Obj.y + (player2Obj.size / 2));
                            player2Obj.y -= 0;
                            
                        }
                        if (blockies[i].x + (blockies[i].size / 2) >= player2Obj.x - (player2Obj.size / 2)){
                            player2Obj.cLeft = true;
                            blockies[i].cRight = true;
                            player2Obj.moveup = true;
                            
                        }
                        if (blockies[i].y < player2Obj.y){
                            player2Obj.cBottom = true;
                            blockies[i].cTop = true;
                            player2Obj.y = blockies[i].y + blockies[i].size;
                            player2Obj.moveup = true;
                            float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - player2Obj.y + (player2Obj.size / 2));
                            player2Obj.y += penetration + 0.01;
                            
                        }
                        
                    }
                    
                }
                for (int j = 0; j < enemies.size(); j++){
                    
                    if (objectCollision(enemies[j], playerObj)){
                        
                        if (playerObj.powerup != "shield") {
                            enemies[j].dead = true;
                            playerObj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER1LOST;
                        }
                        
                    }
                    
                    if (objectCollision(enemies[j], player2Obj)){
                        if (player2Obj.powerup != "shield") {
                            enemies[j].dead = true;
                            player2Obj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER2LOST;
                        }
                    }
                }
                
                if (playerObj.y > -1.75){ // gravity
                    if (!playerObj.cBottom && !playerObj.moving){
                        playerObj.cTop = false;
                        playerObj.moveup = false;
                        playerObj.prevy = playerObj.y;
                        playerObj.y += playerObj.gravity * elapsed;
                        playerObj.fall = true;
                        playerObj.isstatic = true;
                    }
                    
                }
                
                if (player2Obj.y > -1.75) {
                    if (!player2Obj.cBottom && !player2Obj.moving){
                        player2Obj.cTop = false;
                        player2Obj.moveup = false;
                        player2Obj.prevy = player2Obj.y;
                        player2Obj.y += player2Obj.gravity * elapsed;
                        player2Obj.fall = true;
                        player2Obj.isstatic = true;
                    }
                }
                if (player2Obj.y < -1.7){
                    player2Obj.moveup = true;
                }
                if (playerObj.y < -1.7){
                    playerObj.moveup = true;
                }
                
                for (int i = 0; i < enemies.size(); i++){
                    if (enemies[i].y > -1.75){
                        if (!enemies[i].cBottom){
                            enemies[i].y -= enemies[i].speedy * elapsed;
                        }
                    }
                }
                
                
                
                modelMatrix.identity();
                modelMatrix.Translate(playerObj.x, playerObj.y, 0.0);
                program.setModelMatrix(modelMatrix);
                playerObj.sprite.Draw(&program);
                
                modelMatrix.identity();
                modelMatrix.Translate(player2Obj.x, player2Obj.y, 0.0);
                program.setModelMatrix(modelMatrix);
                player2Obj.sprite.Draw(&program);
                
                for (int i = 0; i < playerBullets.size(); ++i) {
                    playerBullets[i].y += elapsed * playerBullets[i].speedy;
                    playerBullets[i].x += elapsed * playerBullets[i].speedx;
                    modelMatrix.identity();
                    modelMatrix.Translate(playerBullets[i].x, playerBullets[i].y, 0.0);
                    program.setModelMatrix(modelMatrix);
                    if (!playerBullets[i].dead){
                        playerBullets[i].sprite.Draw(&program);
                    }
                    else{
                        playerBullets.erase(playerBullets.begin() + i);
                    }
                }
                for (int i = 0; i < player2Bullets.size(); ++i) {
                    player2Bullets[i].y += elapsed * player2Bullets[i].speedy;
                    player2Bullets[i].x += elapsed * player2Bullets[i].speedx;
                    modelMatrix.identity();
                    modelMatrix.Translate(player2Bullets[i].x, player2Bullets[i].y, 0.0);
                    program.setModelMatrix(modelMatrix);
                    if (!player2Bullets[i].dead){
                        player2Bullets[i].sprite.Draw(&program);
                    }
                    else{
                        player2Bullets.erase(player2Bullets.begin() + i);
                    }
                }
                for (int i = 0; i < playerBullets.size(); i++){
                    if (objectCollision(playerBullets[i], player2Obj)){
                        playerBullets[i].dead = true;
                        playerBullets[i].x = -10.0;
                        playerBullets[i].y = -10.0;
                        playerBullets[i].speedy = 0.0;
                        playerBullets[i].speedx = 1.0;
                        if (player2Obj.powerup != "shield") {
                            player2Obj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            state = GAME_STATE_PLAYER2LOST;
                        }
                    }
                    if (playerBullets[i].y >= 7.0 || playerBullets[i].x < -4.0 || playerBullets[i].x > 4.0) {
                        playerBullets[i].dead = true;
                        playerBullets[i].x = -10.0;
                        playerBullets[i].y = -10.0;
                        playerBullets[i].speedy = 0.0;
                        playerBullets[i].speedx = 1.0;
                    }
                    for (int j = 0; j < blockies.size(); ++j){
                        if (objectCollision(playerBullets[i], blockies[j])){
                            
                            playerBullets[i].dead = true;
                            playerBullets[i].x = -10.0;
                            playerBullets[i].y = -10.0;
                            playerBullets[i].speedy = 0.0;
                            playerBullets[i].speedx = 1.0;
                        }
                    }
                    
                    for (int j = 0; j < enemies.size(); ++j) {
                        if (objectCollision(playerBullets[i], enemies[j])) {
                            playerBullets[i].dead = true;
                            playerBullets[i].x = 10.0;
                            playerBullets[i].speedy = 0.0;
                            enemies[j].y = 10.0;
                            enemies[j].dead = true;
                            numEnemiesAlive--;
                        }
                    }
                }
                
                for (int i = 0; i < player2Bullets.size(); i++){
                    if (objectCollision(player2Bullets[i], playerObj)){
                        player2Bullets[i].dead = true;
                        player2Bullets[i].x = -10.0;
                        player2Bullets[i].y = -10.0;
                        player2Bullets[i].speedy = 0.0;
                        player2Bullets[i].speedx = 1.0;
                        
                        if (playerObj.powerup != "shield") {
                            playerObj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER1LOST;
                            
                        }
                    }
                    if (player2Bullets[i].y >= 7.0 || player2Bullets[i].x < -4.0 || player2Bullets[i].x > 4.0) {
                        player2Bullets[i].dead = true;
                        player2Bullets[i].x = -10.0;
                        player2Bullets[i].y = -10.0;
                        player2Bullets[i].speedy = 0.0;
                        player2Bullets[i].speedx = 1.0;
                    }
                    for (int j = 0; j < blockies.size(); ++j){
                        if (objectCollision(blockies[j], player2Bullets[i])){
                            
                            player2Bullets[i].dead = true;
                            player2Bullets[i].x = -10.0;
                            player2Bullets[i].y = -10.0;
                            player2Bullets[i].speedy = 0.0;
                            player2Bullets[i].speedx = 1.0;
                        }
                    }
                    
                    for (int j = 0; j < enemies.size(); ++j) {
                        if (objectCollision(enemies[j], player2Bullets[i])) {
                            player2Bullets[i].dead = true;
                            player2Bullets[i].x = 10.0;
                            player2Bullets[i].speedy = 0.0;
                            enemies[j].y = 10.0;
                            enemies[j].dead = true;
                            numEnemiesAlive--;
                        }
                    }
                }
                for (int i = 0; i < enemies.size(); ++i) {
                    if (!enemies[i].dead) {
                        enemies[i].x += elapsed * enemies[i].speedx;
                        modelMatrix.identity();
                        modelMatrix.Translate(enemies[i].x, enemies[i].y, 0);
                        program.setModelMatrix(modelMatrix);
                        enemies[i].sprite.Draw(&program);
                    }
                }
                
                for (int i = 0; i < enemies.size(); i++) {
                    if (enemies[i].speedx > 0){
                        AI[i].x = enemies[i].x + 0.2;
                        AI[i].y = enemies[i].y - (enemies[i].size / 2) - 0.1;
                    }
                    else{
                        AI[i].x = enemies[i].x - 0.2;
                        AI[i].y = enemies[i].y - (enemies[i].size / 2) - 0.1;
                    }
                    
                    if (enemies[i].x > 3.35) {
                        enemies[i].speedx = -0.75;
                    }
                    else if (enemies[i].x < -3.35) {
                        enemies[i].speedx = 0.75;
                    }
                    
                    //for (int j = 0; j < blockies.size(); j++){
                    if (!pointVectorCollision(blockies, AI[i])){
                        enemies[i].speedx *= -1;
                    }
                }
                
                shootingDelay += elapsed;
                playerSD += elapsed;
                player2SD += elapsed;
                
                if (shootingDelay > 0.95) {
                    
                    GameObject enemyBulletObj = enemyShooting(enemies, enemyBullet);
                    shootingDelay = 0;
                    
                    enemyBullets.push_back(enemyBulletObj);
                }
                
                ///------ENEMY BULLET / PLAYER COLLISION-------
                for (int i = 0; i < enemyBullets.size(); i++){
                    enemyBullets[i].y -= elapsed * enemyBullets[i].speedy;
                    modelMatrix.identity();
                    modelMatrix.Translate(enemyBullets[i].x, enemyBullets[i].y, 0.0);
                    program.setModelMatrix(modelMatrix);
                    
                    if (!enemyBullets[i].dead){
                        enemyBullets[i].sprite.Draw(&program);
                    }
                    
                    else{
                        enemyBullets.erase(enemyBullets.begin() + i);
                    }
                    
                    if (objectCollision(enemyBullets[i], playerObj)) {
                        enemyBullets[i].y = -10.0;
                        enemyBullets[i].speedy = 0.0;
                        // we died
                        
                        numEnemiesAlive = enemies.size();
                        enemyBullets[i].dead = true;
                        if (playerObj.powerup != "shield") {
                            playerObj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER1LOST;
                        }
                        
                    }
                    
                    if (objectCollision(enemyBullets[i], player2Obj)) {
                        enemyBullets[i].y = -10.0;
                        enemyBullets[i].speedy = 0.0;
                        // we died
                        
                        numEnemiesAlive = enemies.size();
                        enemyBullets[i].dead = true;
                        if (player2Obj.powerup != "shield") {
                            player2Obj.dead = true;
                            Mix_PlayChannel(-1, gameoverSound, 0);
                            
                            state = GAME_STATE_PLAYER2LOST;
                        }
                    }
                }
                
                //-----POWER UPS-----
                modelMatrix.identity();
                program.setModelMatrix(modelMatrix);
                for (int i = 0; i < powerUps.particles.size(); ++i) {
                    powerUps.particles[i].squish = false;
                    powerUps.particles[i].squash = 0.0f;
                    powerUps.particles[i].y += elapsed * powerUps.gravity;
                    powerUps.particles[i].lifetime += elapsed;
                    powerUps.particles[i].squash += elapsed;
                    
                    if (powerUps.particles[i].lifetime >= powerUps.maxLifetime) {
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].squish = false;
                        
                        powerUps.particles[i].lifetime = powerUps.particles[i].defaultlifetime;
                        
                    }
                    
                    
                    //----- PARTICLE COLLISION WITH BLOCKS -----
                    for (int j = 0; j < blockies.size(); j++) {
                        if (particleCollision(blockies[j], powerUps.particles[i])) {
                            
                            if (blockies[j].y < powerUps.particles[i].y){
                                
                                blockies[j].cTop = true;
                                powerUps.particles[i].y = blockies[j].y + blockies[j].size;
                                float penetration = fabs(blockies[j].y + (blockies[j].size / 2) - powerUps.particles[i].y + (powerUps.particles[i].size / 2));
                                powerUps.particles[i].y += penetration + 0.01;
                                
                                powerUps.particles[i].x += elapsed * powerUps.particles[i].speedx;
                                powerUps.particles[i].y += elapsed * powerUps.gravity;
                            }
                            if (powerUps.particles[i].squash < 2) {
                                powerUps.particles[i].squish = true;
                            }
                            else if (powerUps.particles[i].squash == 2) {
                                powerUps.particles[i].squish = false;
                                powerUps.particles[i].squash = 0;
                            }
                            
                        }
                        
                    }
                    
                    //----- PARTICLES FALL OFF SCREEN -----
                    if (powerUps.particles[i].y < -2.0) {
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        powerUps.particles[i].squish = false;
                        
                        
                        powerUps.particles[i].lifetime = 2;
                    }
                    
                    //----- PARTICLE COLLISION WITH WALLS -----
                    if (powerUps.particles[i].x > 3.55) {
                        
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].x += powerUps.particles[i].speedx * elapsed;
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        
                        //powerUps.particles[i].sprite.Draw(&program);
                        
                    }
                    
                    if (powerUps.particles[i].x < -3.55) {
                        
                        powerUps.particles[i].speedx *= -1;
                        powerUps.particles[i].x += powerUps.particles[i].speedx * elapsed;
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        
                        //powerUps.particles[i].sprite.Draw(&program);
                    }
                    
                    
                    //----- PLAYER/PARTICLE COLLISION TEST -----
                    if (particleCollision(playerObj, powerUps.particles[i])) {
                        p1clicks = 0;
                        playerObj.sprite = player;
                        playerObj.cPart = true;
                        
                        if (powerUps.particles[i].type == "speedup" || powerUps.particles[i].type == "shield") {
                            Mix_PlayChannel(-1, powerUpSound, 0);
                        }
                        
                        else if (powerUps.particles[i].type == "speeddown" || powerUps.particles[i].type == "frozen") {
                            Mix_PlayChannel(-1, powerDownSound, 0);
                        }
                        
                        playerObj.powerup = powerUps.particles[i].type;
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        
                        powerUps.particles[i].lifetime = 2;
                        
                    }
                    
                    if (particleCollision(player2Obj, powerUps.particles[i])) {
                        p2clicks = 0;
                        player2Obj.sprite = player2;
                        player2Obj.cPart = true;
                        
                        if (powerUps.particles[i].type == "speedup" || powerUps.particles[i].type == "shield") {
                            Mix_PlayChannel(-1, powerUpSound, 0);
                        }
                        
                        if (powerUps.particles[i].type == "speeddown" || powerUps.particles[i].type == "frozen") {
                            Mix_PlayChannel(-1, powerDownSound, 0);
                        }
                        
                        player2Obj.powerup = powerUps.particles[i].type;
                        powerUps.particles[i].x = -3.5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.5 + 3.5)));
                        powerUps.particles[i].y = powerUps.y;
                        
                        powerUps.particles[i].lifetime = 2;
                        
                    }
                    
                    //-----DRAW-----
                    if (powerUps.particles[i].squish == true) {
                        float scale_y = mapValue(fabs(1.0), 0.0, 5.0, 0.8, 1.0);
                        float scale_x = mapValue(fabs(1.0), 5.0, 0.0, 1.0, 1.6);
                        
                        
                        modelMatrix.identity();
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        modelMatrix.Scale(scale_x, scale_y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        //powerUps.particles[i].sprite.Draw(&program);
                    }
                    else {
                        
                        modelMatrix.identity();
                        modelMatrix.Scale(1.0, 1.0, 0.0);
                        modelMatrix.Translate(powerUps.particles[i].x, powerUps.particles[i].y, 0.0);
                        program.setModelMatrix(modelMatrix);
                        
                    }
                    powerUps.particles[i].sprite.Draw(&program);
                    
                    
                }
                
                //----- PLAYER/PLAYER COLLISION -----
                
                if (objectCollision(playerObj, player2Obj)){
                    playerObj.dead = true;
                    player2Obj.dead = true;
                    Mix_PlayChannel(-1, gameoverSound, 0);
                    
                    state = GAME_STATE_BOTHDIE;
                }
                break;
        }
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}
