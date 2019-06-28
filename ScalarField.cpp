#include <stdexcept>
#include <cmath>

#include "ScalarField.h"


ScalarField::ScalarField(float x, float y, float z, float width, float depth, float height, uint32_t nbX, uint32_t nbY, uint32_t nbZ)
{
	mXOrig = x;
	mYOrig = y;
	mZOrig = z;
	mWidth = abs(width);
	mDepth = abs(depth);
	mHeight = abs(height);
	mXDivs = nbX;
	mYDivs = nbY;
	mZDivs = nbZ;

	if (mWidth * mDepth * mHeight == 0.0)
	{
		throw std::invalid_argument("One dimension is 0.");
	}
	if (mXDivs * mYDivs * mZDivs == 0.0)
	{
		throw std::invalid_argument("One number of divisions is 0.");
	}

	mXStep = mWidth / mXDivs;
	mYStep = mDepth / mYDivs;
	mZStep = mHeight / mZDivs;
}


 ScalarField::~ScalarField()
{
}


 float ScalarField::getValue(uint32_t i, uint32_t j, uint32_t k)
{
	if ( i > mXDivs )
	{
		throw std::invalid_argument("Index i out of bounds.");
	}
	if ( j > mYDivs )	{
		throw std::invalid_argument("Index i out of bounds.");
	}
	if ( k > mZDivs )
	{
		throw std::invalid_argument("Index i out of bounds.");
	}
	return doGetValue(i, j, k);
}
