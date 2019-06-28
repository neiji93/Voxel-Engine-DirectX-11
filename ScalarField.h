#ifndef SCALARFIELD_H
#define SCALARFIELD_H

//***************************************************************************************
// Generic abstract scalar field class: a axis aligned 3D grid of float values.
// The grid is uniform in each of the 3 main directions.
//
// This class is intended to be subclassed.
//***************************************************************************************

#include <cstdint>

class ScalarField
{
public:
	ScalarField(float x, float y, float z, float width, float depth, float height, uint32_t nbX, uint32_t nbY, uint32_t nbZ);
	virtual ~ScalarField();

	float getValue(uint32_t i, uint32_t j, uint32_t k);

	uint32_t getXCount() { return mXDivs + 1; };
	uint32_t getYCount() { return mYDivs + 1; };
	uint32_t getZCount() { return mZDivs + 1; };

	float getXOrig() { return mXOrig; };
	float getYOrig() { return mYOrig; };
	float getZOrig() { return mZOrig; };

	float getWidth() { return mWidth; };
	float getDepth() { return mDepth; };
	float getHeight() { return mHeight; };

	float getXStep() { return mXStep; };
	float getYStep() { return mYStep; };
	float getZStep() { return mZStep; };

protected:
	virtual float doGetValue(int i, int j, int k) = 0;

	// Grid origin.
	float mXOrig;
	float mYOrig;
	float mZOrig;

	// Grid dimensions.
	float mWidth;
	float mDepth;
	float mHeight;

	// Number of grid intervals in each direction.
	uint32_t mXDivs;
	uint32_t mYDivs;
	uint32_t mZDivs;

	// Grid steps.
	float mXStep;
	float mYStep;
	float mZStep;

private:
    ScalarField( const ScalarField& );
    const ScalarField& operator=( const ScalarField& );
};

#endif
