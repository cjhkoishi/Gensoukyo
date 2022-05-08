#include "pch.h"
#include "Scene.h"

void Scene::update()
{
	for (auto it : container) {
		it.second->update();
	}
}
