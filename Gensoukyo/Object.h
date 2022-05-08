#pragma once
#include "pch.h"
#include "Component.h"
class Scene;
class Object
{
protected:
	glm::mat4 world_transform;
public:
	glm::mat4 local_transform;
	std::string name;

	std::vector<Component*> components;
	Scene* location;

	template<typename T> T* GetComponent();
	void update();
};

