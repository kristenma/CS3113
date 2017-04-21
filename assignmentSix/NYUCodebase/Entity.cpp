#include "Entity.h"
#include "ShaderProgram.h"

Entity::Entity (float w, float h, float x, float y, float s){
    x = x;
    y = y;
    width = w;
    height = h;
    speed = s;
}

void Entity::Draw(ShaderProgram *program) {
    
    float vertices[] = {x-width/2, y-height/2, x-width/2, y+height/2, x+width/2, y+height/2, x+width/2, y+height/2, x-width/2, y-height/2, x+width/2, y-height/2};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}
