#include "Node.h"

Node::Node() {
	;
};

Node::Node(bool _walkable, vec3f _worldPos) {
		walkable = _walkable;
		worldPos = _worldPos;
};