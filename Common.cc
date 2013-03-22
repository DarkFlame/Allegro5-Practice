#include <cmath>

#include "Common.h"

int sign(int i)
{
    if (i>0) return +1;
    else if (i<0) return -1;
    else return 0;
}

void ResetString(char *&sString, const char *pNewString)
{
    //--Courtesy of Salty Justice
    //--Deallocates a string and sets it to the pNewString.  Passing NULL for the pNewString will
    //  simple deallocate and NULL the sString.
    //--MAKE SURE the sString was either NULL or a valid string before passing in to this, this call
    //  should not be used on raw allocated memory.
    if(sString == pNewString) return;

    free(sString);
    sString = NULL;
    if(!pNewString) return;

    sString = (char *)malloc(sizeof(char) * (strlen(pNewString) + 1));
    strcpy(sString, pNewString);
}

Vector2f::Vector2f()
{
    x = 0;
    y = 0;
}
Vector2f::Vector2f(float nx, float ny)
{
    x = nx;
    y = ny;
}

Vector2f Vector2f::operator*(Vector2f param)
{
    return Vector2f(x*param.x, y*param.y);
}
Vector2f Vector2f::operator/(Vector2f param)
{
    return Vector2f(x/param.x, y/param.y);
}
Vector2f Vector2f::operator+(Vector2f param)
{
    return Vector2f(x+param.x, y+param.y);
}
Vector2f Vector2f::operator-(Vector2f param)
{
    return Vector2f(x-param.x, y-param.y);
}
void Vector2f::operator+=(Vector2f param)
{
    x = x + param.x;
    y = y + param.y;
}
void Vector2f::operator-=(Vector2f param)
{
    x = x - param.x;
    y = y - param.y;
}

float Vector2f::angle(Vector2f other)
{
    return atan2(x-other.x, y-other.y);
}
float Vector2f::length()
{
    return sqrt(pow(x,2)+pow(y,2));
}
float Vector2f::lengthSquared()
{
    return pow(x,2)+pow(y,2);
}
