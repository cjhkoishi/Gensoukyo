#include "pch.h"
#include "Scene.h"

void Scene::update()
{
	model[0]= glm::mat4(1.0f);
	unsigned current_hierarchy = 0;
	for (auto it : container) {
		it.update();
	}
}

void Scene::DAGNode::update()
{
	obj->update();
}
