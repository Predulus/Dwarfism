#pragma once
#include <cmath>
#include <vmmlib/vector.hpp>
#include "Node.h"
using namespace vmml;

class Grid
{
public:
	unsigned short unwalkableMask;
	Node*** pGrid;
	vec2f gridWorldSize;
	float nodeRadius;
	float nodeDiameter;
	int gridSizeX;
	int gridSizeY;

	Grid();

	void Start();

	void CreateGrid();

	bool CheckSphere(vec3f, float);

	Node* NodeFromWorldPoint(vec2f _worldPosition);
};

