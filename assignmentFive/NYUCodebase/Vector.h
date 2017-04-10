#ifndef Vector_h
#define Vector_h


class Vector {
public:
    
    Vector();
    Vector(float x, float y);
    
    float length();
    void normalize();
    
    Vector operator* (const Matrix& mat);
    
    float x;
    float y;
 
};


#endif /* Vector_h */
