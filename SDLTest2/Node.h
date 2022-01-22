#pragma once
#include <vmmlib/vector.hpp>
using namespace vmml;

class Node
{
public:
	vec3f worldPos;
	bool walkable;
	float fcost;
	Node* left;
	Node* right;
	Node* parent;

	Node();
	Node(bool _walkable, vec3f _worldPos);

};

