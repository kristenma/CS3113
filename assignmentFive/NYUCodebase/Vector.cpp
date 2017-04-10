#include "ShaderProgram.h"
#include "Vector.h"
#include "Matrix.h"
#include <stdio.h>

Vector::Vector(float x, float y) : x(x), y(y){}

Vector::Vector(){}

float Vector::length() {
    return sqrt(x*x + y*y);
    
}

void Vector::normalize() {
    
    x /= length();
    y /= length();

}

Vector Vector::operator*(const Matrix& mat) {
    Vector vec;
    
    vec.x = mat.m[0][0] * x + mat.m[1][0] * this->y + mat.m[2][0] * 0.0f + mat.m[3][0] * 1.0f;
    vec.y = mat.m[0][1] * x + mat.m[1][1] * this->y + mat.m[2][1] * 0.0f + mat.m[3][1] * 1;
    
    return vec;
}
