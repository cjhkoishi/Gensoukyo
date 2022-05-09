#pragma once
#include "pch.h"
#include "Object.h"
class Scene
{
protected:
	glm::mat4 model[64], view, projection;
public:
	struct DAGNode {
		Object* obj;
		unsigned hierarchy;

		void update();
		DAGNode parent();
		std::vector<DAGNode> children();
	};

	std::vector<DAGNode> container;

	void update();

	friend class Object;
};

