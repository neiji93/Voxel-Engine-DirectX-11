#include <cmath>

#include "BallScalarField.h"


BallScalarField::BallScalarField(float xCenter, float yCenter, float zCenter, float x, float y, float z, float width, float depth, float height, uint32_t nbX, uint32_t nbY, uint32_t nbZ) :
	ScalarField(x, y, z, width, depth, height, nbX, nbY, nbZ)
{
	mXCenter = xCenter;
	mYCenter = yCenter;
	mZCenter = zCenter;
}


BallScalarField::~BallScalarField()
{
}


float BallScalarField::doGetValue(int i, int j, int k)
{
	float x = mXOrig + mXStep * i;
	float y = mYOrig + mYStep * j;
	float z = mZOrig + mZStep * k;

	return sqrt((x - mXCenter)*(x - mXCenter) + (y - mYCenter)*(y - mYCenter) + (z - mZCenter)*(z - mZCenter));
}
