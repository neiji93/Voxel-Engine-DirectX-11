#ifndef BALLSCALARFIELD_H
#define BALLSCALARFIELD_H

//***************************************************************************************
// Scalar field class that represents the distance to a point. Iso-surfaces will be
// spheres.
//***************************************************************************************

#include "ScalarField.h"

class BallScalarField : public ScalarField
{
public:
	BallScalarField(float xCenter, float yCenter, float zCenter, float x, float y, float z, float width, float depth, float height, uint32_t nbX, uint32_t nbY, uint32_t nbZ);
	virtual ~BallScalarField();

protected:
	virtual float doGetValue(int i, int j, int k);

	float mXCenter;
	float mYCenter;
	float mZCenter;

private:
	BallScalarField(const BallScalarField&);
	const BallScalarField& operator=(const BallScalarField&);
};

#endif
