// COMP710 GP Framework 2024
#ifndef __MATRIX4_H_
#define __MATRIX4_H_

struct Matrix4
{
	float m[4][4];
};

void SetZero(Matrix4& mat);
void SetIdentity(Matrix4& mat);
void CreateOrthoProjection(Matrix4& mat, float width, float height);

#endif // __MATRIX4_H_