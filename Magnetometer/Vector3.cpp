/*
* Vector3.cpp
*
* Created: 10.11.2019 14:56:13
* Author: DELTA-PC
*/

#include "math.h"
#include "Vector3.h"

Vector3f::Vector3f(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

Vector3f::Vector3f()
{
	x = 0;
	y = 0;
	z = 0;
}

inline float& Vector3f::operator[](char component)
{
	return ((&x)[component]);
}

Vector3f Vector3f::operator+(const Vector3f rvalue)
{
	return Vector3f(x + rvalue.x,y + rvalue.y, z + rvalue.z);
}
Vector3f Vector3f::operator-(const Vector3f rvalue)
{
	return Vector3f(x - rvalue.x,y - rvalue.y, z - rvalue.z);
}
Vector3f Vector3f::operator*(const Vector3f rvalue)
{
	return Vector3f(x * rvalue.x,y * rvalue.y, z * rvalue.z);
}
Vector3f Vector3f::operator/(const Vector3f rvalue)
{
	return Vector3f(x / rvalue.x,y / rvalue.y, z / rvalue.z);
}


Vector3f Vector3f::operator*(const float rvalue)
{
	return Vector3f(x * rvalue,y * rvalue, z * rvalue);
}
Vector3f Vector3f::operator/(const float rvalue)
{
	return Vector3f(x / rvalue,y / rvalue, z / rvalue);
}
bool Vector3f::operator==(const Vector3f rvalue)
{
	return x == rvalue.x && y == rvalue.y && z == rvalue.z;
}



static Vector3f Vector3f::CrossProduct(Vector3f a, Vector3f b)
{
	return Vector3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

float Vector3f::DotProduct(Vector3f a, Vector3f b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Vector3f::Magnitude()
{
	float ms = 0;
	for(char i = 0; i < 3; i++)
	ms += pow((&x)[i],2);
	return sqrt(ms);
}

void Vector3f::Normalize()
{
	float m = Magnitude();
	x /= m;
	y /= m;
	z /= m;
}
