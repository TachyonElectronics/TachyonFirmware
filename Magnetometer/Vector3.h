/* 
* Vector3.h
*
* Created: 10.11.2019 14:56:13
* Author: DELTA-PC
*/

#ifndef __VECTOR3_H__
#define __VECTOR3_H__

#define X 0
#define Y 1
#define Z 2

class Vector3f
{
  public:
	float x;
	float y;
	float z;
	Vector3f();
	Vector3f(float _x, float _y, float _z);

	inline float& operator[](char component);
	Vector3f operator+(const Vector3f rvalue);
	Vector3f operator-(const Vector3f rvalue);
	Vector3f operator*(const Vector3f rvalue);
	Vector3f operator/(const Vector3f rvalue);
	Vector3f operator*(const float rvalue);
	Vector3f operator/(const float rvalue);
	bool operator==(const Vector3f rvalue);
	

	static Vector3f CrossProduct(Vector3f a, Vector3f b);

	static float DotProduct(Vector3f a, Vector3f b);

	float Magnitude();

	void Normalize();
	
};

#endif //__VECTOR3_H__
