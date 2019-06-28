#ifndef METABALLSCALARFIELD_H
#define METABALLSCALARFIELD_H

//***************************************************************************************
// Scalar field class holding an arbitrary number of metaballs. Iso-surfaces will be
// organic looking.
// See https://en.wikipedia.org/wiki/Metaballs
//***************************************************************************************

#include <vector>

#include "ScalarField.h"

class MetaballScalarField : public ScalarField
{
public:
	MetaballScalarField(float x, float y, float z, float width, float depth, float height, uint32_t nbX, uint32_t nbY, uint32_t nbZ);
	virtual ~MetaballScalarField();

	int AddMetaball(float xCenter, float yCenter, float zCenter);
	void MoveMetaball(int i, float xCenter, float yCenter, float zCenter);
	void GetMetaball(int i, float &xCenter, float &yCenter, float &zCenter);
	int Count();

protected:
	virtual float doGetValue(int i, int j, int k);

	std::vector<float> mXCenters;
	std::vector<float> mYCenters;
	std::vector<float> mZCenters;

private:
	MetaballScalarField(const MetaballScalarField&);
	const MetaballScalarField& operator=(const MetaballScalarField&);
};

#endif
