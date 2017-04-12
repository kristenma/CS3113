#include "Entity.h"
#include "ShaderProgram.h"

Entity::Entity(float x, float y, float w, float h, float vx, float vy, float ax, float ay, bool stat, EntityType type){
    x = x;
    y = y;
    width = w;
    height = h;
    velocity_x = vx;
    velocity_y = vy;
    acceleration_x = ax;
    acceleration_y = ay;
    isStatic = stat;
    entityType = type;
}

Entity::Entity() : x(0), y(0), velocity_x(0), velocity_y(0), acceleration_x(0), acceleration_y(0) {}


void Entity::findEdges(){
    bottom = y - (sprite.height/2);
    top = y + (sprite.height/2);
    left = x - (sprite.width / 2);
    right = x + (sprite.width/2);
}

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}



void Entity::Update(float elapsed) {
    if (!this->isStatic) {

        if (this->acceleration_x > 2.5f) this->acceleration_x = 2.5f;
        else if (this->acceleration_x < -2.5f) this->acceleration_x = -2.5f;

        if (this->acceleration_y > 0) this->acceleration_y-= elapsed;

        this->velocity_x = lerp(this->velocity_x, 0.0f, elapsed * -0.2);
        this->velocity_y = lerp(this->velocity_y, 0.0f, elapsed * -0.2);

        this->velocity_x += this->acceleration_x * elapsed;
        this->velocity_y += this->acceleration_y * elapsed;
        this->velocity_y += 2.0f * elapsed;

        this->x += this->velocity_x * elapsed;
        this->y += this->velocity_y * elapsed;
    }
}


void Entity::Render(ShaderProgram program, Matrix& matrix) {
    matrix.identity();
    matrix.Translate(this->x, this->y, 0);
    program.setModelMatrix(matrix);
    this->sprite.Draw(program);
}

bool Entity::collidesWith(Entity* ent) {
    
    //checks collision of left side of a with right side of b
    if ((this->x - this->width/2) > (ent->x + ent->width/2)) {
        return true;
    }
    
    //checks collision of right side of a with left side of b
    if ((this->x + this->width/2) < (ent->x - ent->width/2)) {
        return true;
    }
    
    //checks collision of bottom of a with top of b
    if ((this->y - this->height/2) > (ent->y + ent->height/2)) {
        return true;
    }
    
    //checks collision of top of a with bottom of b
    if ((this->y + this->height/2) < (ent->y - ent->height/2)) {
        return true;
    }
    
    return false;
}


