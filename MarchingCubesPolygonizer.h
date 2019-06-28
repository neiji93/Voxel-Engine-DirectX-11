#ifndef MARCHINGCUBESPOLYGONIZER_H
#define MARCHINGCUBESPOLYGONIZER_H

#include <vector>

class ScalarField;

typedef struct {
	float x, y, z;
} XYZ;

/*typedef struct {
	XYZ p[3];
} TRIANGLE;*/

typedef struct {
	XYZ p[8];
	float val[8];
} GRIDCELL;


class MarchingCubesPolygonizer
{
public:
	MarchingCubesPolygonizer(ScalarField * scalarField);
	virtual ~MarchingCubesPolygonizer();

	virtual void Polygonize(float isoValue, std::vector<float> * vertices);

protected:
	// The scalar field to be represented.
	ScalarField * mScalarField;

	// To reduce parameter passing.
	float mIsoValue;
	std::vector<float> * mVertices;

private:
	void FillGrid(float * grid, int z);
	void PolygonizeGrids(float *topVals, float *bottomVals, int z);
	bool PushPolygons(GRIDCELL &g);

	// Avoid unwanted copy and assign.
	MarchingCubesPolygonizer(const MarchingCubesPolygonizer&);
	const MarchingCubesPolygonizer& operator=(const MarchingCubesPolygonizer&);
};

#endif
