#pragma once
#include "pch.h"
#include "Object.h"
class Window;
class Scene
{
public:
	glm::mat4 view, projection;
	std::stack<glm::mat4> model;

	Window* wind;

	struct DAGNode {
		Object* obj;
		int hierarchy;

		void update();
		DAGNode(Object* obj, int hierarchy) :obj(obj), hierarchy(hierarchy) {};
	};

	std::vector<DAGNode> container;

	void update();
	Object* createObject(std::string name);

	Scene();

	friend class Object;
	friend class Component;
};

