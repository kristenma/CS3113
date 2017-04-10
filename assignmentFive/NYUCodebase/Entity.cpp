#include <stdio.h>
#include "Entity.h"

void Entity::draw(ShaderProgram& program) {
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texcoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 15);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

void Entity::update(float elapsed) {
    
    rotation += 1.0f * elapsed;
    
    this->velocity_x += this->acceleration_x * elapsed;
    this->velocity_y += this->acceleration_y * elapsed;
    
    this->position.x += this->velocity_x * elapsed;
    this->position.y += this->velocity_y * elapsed;
}
