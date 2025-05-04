// This include
#include "vector2.h"

// Local includes
#include "renderer.h"

// Lib includes
#include <cassert>
#include <cmath>

// Default constructor
Vector2::Vector2()
	: x(0.0f)
	, y(0.0f)
{

}

// Constructor with parameters
Vector2::Vector2(float inx, float iny)
	: x(inx)
	, y(iny)
{

}

// Destructor
Vector2::~Vector2()
{

}

// Pub copy constructor
Vector2::Vector2(const Vector2& vector2) 
	: x(vector2.x)
	, y(vector2.y)
{

}

// Assignment operator pub
Vector2& Vector2::operator=(const Vector2& vector2)
{
	if (this != &vector2)
	{
		x = vector2.x;
		y = vector2.y;
	}

	return *this;
}

// Set vector values
void
Vector2::Set(float inx, float iny)
{
	x = inx;
	y = iny;
}

// Calculate trhe length of the vector
float
Vector2::LengthSquared() const
{
	return ((x * x) + (y * y));
}

// Calculate trhe length of the vector
float
Vector2::Length() const
{
	return (sqrtf(LengthSquared()));
}

void
Vector2::Normalise()
{
	float length = Length();
	if (length > 0)
	{
		float inverseLength = 1.0f / length;
		x = x / length;
		y = y / length;
	}
}

// Static method for calculating dot product
float
Vector2::DotProduct(const Vector2& veca, const Vector2& vecb)
{
	return (veca.x * vecb.x) + (veca.y * vecb.y);
}

// Static method for linear interpolation
Vector2
Vector2::Lerp(const Vector2& veca, const Vector2& vecb, float time)
{
	return (Vector2(veca + time * (vecb - veca)));
}

// Static method for reflection
Vector2
Vector2::Reflect(const Vector2& veca, const Vector2& vecb)
{
	float dotProduct = DotProduct(veca, vecb);
	return (veca - vecb * 2.0f * dotProduct);
}