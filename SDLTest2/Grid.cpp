#include "Grid.h"
#include <vector>
using namespace std;

Grid::Grid() {
}

void Grid::Start() {
		nodeDiameter = nodeRadius * 2.0;
		gridSizeX = (int)round(gridWorldSize.x() / nodeDiameter);
		gridSizeY = (int)round(gridWorldSize.y() / nodeDiameter);
	}

bool Grid::CheckSphere(vec3f _point, float _radius) {
	return true;
}

Node* Grid::NodeFromWorldPoint(vec2f _worldPosition) {
	return pGrid[(int)round(_worldPosition.x())][(int)round(_worldPosition.y())];
}

void Grid::CreateGrid() {
	vec3f agentPos;
	pGrid = new Node ** [gridSizeX];
	vec3f worldBottomLeft = agentPos - vec3f::RIGHT * gridWorldSize.x() / 2 - vec3f::UP * gridWorldSize.y() / 2;
	//grid[1, 1] = new Node();
	for (int i = 0; i < gridSizeX; i++) {
		pGrid[i] = new Node*[gridSizeY];
		for (int j = 0; j < gridSizeX; j++) {
			vec3f worldPoint = worldBottomLeft + vec3f::RIGHT * (i * nodeDiameter + nodeRadius) + vec3f::UP * (i * nodeDiameter + nodeRadius);
			bool walkable = CheckSphere(worldPoint, nodeRadius);
			pGrid[i][j] = new Node(walkable, worldPoint);
			pGrid[i][j]->fcost = 0;
		}
	}


}

