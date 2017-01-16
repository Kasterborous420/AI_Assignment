#pragma once
#include <math.h>
struct MyVector
{
	float x, y;
	MyVector() :x(0), y(0){}
	MyVector(float x, float y) :x(x), y(y){}
	void SetPosition(float _x, float _y){ x = _x; y = _y; }
	float GetX(){ return x; }
	float GetY(){ return y; }
	float Magnitude(){ return sqrt(x*x + y*y); }
	MyVector Normalize(){ float length = Magnitude(); return MyVector(x / length, y / length); }
	MyVector operator + (MyVector u){ return MyVector(x + u.x, y + u.y); }
	MyVector operator - (MyVector u){ return MyVector(u.x - x, u.y - y); }
	MyVector operator += (MyVector u){ return MyVector(x + u.x, y + u.y); }
	MyVector operator ~(){ return MyVector(-x, -y); }
	MyVector operator *(float scale){ return MyVector(x*scale, y*scale); }
	float operator * (MyVector  v){ return  x*v.x + y*v.y; }
};
