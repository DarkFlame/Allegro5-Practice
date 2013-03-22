#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void ResetString(char *&sString, const char *pNewString);

int sign(int i);

class Vector2f
{
    public:
    float x;
    float y;

    Vector2f();
    Vector2f(float nx, float ny);

    Vector2f operator*(Vector2f param);
    Vector2f operator/(Vector2f param);
    Vector2f operator+(Vector2f param);
    Vector2f operator-(Vector2f param);
    void operator+=(Vector2f param);
    void operator-=(Vector2f param);

    float angle(Vector2f other);
    float length();
    float lengthSquared();
};
#endif // _COMMON_
