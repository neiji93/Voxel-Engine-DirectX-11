#include <cmath>

#include "MetaballScalarField.h"


MetaballScalarField::MetaballScalarField(float x, float y, float z, float width, float depth, float height, uint32_t nbX, uint32_t nbY, uint32_t nbZ) :
ScalarField(x, y, z, width, depth, height, nbX, nbY, nbZ)
{
}


MetaballScalarField::~MetaballScalarField()
{
}

int MetaballScalarField::AddMetaball(float xCenter, float yCenter, float zCenter)
{
	int result = mXCenters.size();

	// No check the ball is in the field.
	mXCenters.push_back(xCenter);
	mYCenters.push_back(yCenter);
	mZCenters.push_back(zCenter);

	return result;
}


void MetaballScalarField::MoveMetaball(int i, float xCenter, float yCenter, float zCenter)
{
	// No check the ball is in the field.
	mXCenters[i] = xCenter;
	mYCenters[i] = yCenter;
	mZCenters[i] = zCenter;
}


void MetaballScalarField::GetMetaball(int i, float &xCenter, float &yCenter, float &zCenter)
{
	xCenter = mXCenters[i];
	yCenter = mYCenters[i];
	zCenter = mZCenters[i];
}


int MetaballScalarField::Count()
{
	return mXCenters.size();
}


float MetaballScalarField::doGetValue(int i, int j, int k)
{
	float x = mXOrig + mXStep * i;
	float y = mYOrig + mYStep * j;
	float z = mZOrig + mZStep * k;

	float result = 0;
	for (uint32_t i = 0; i < mXCenters.size(); i++)
	{
		result += 1.f / ((x - mXCenters[i])*(x - mXCenters[i]) + (y - mYCenters[i])*(y - mYCenters[i]) + (z - mZCenters[i])*(z - mZCenters[i]));
	}
	return result;
}
