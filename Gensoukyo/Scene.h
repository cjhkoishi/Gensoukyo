#pragma once
#include "pch.h"
#include "Object.h"
class Scene
{
public:
	std::vector<std::pair<unsigned,Object*>> container;

	void update();
};

