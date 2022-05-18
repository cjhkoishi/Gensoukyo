#include "pch.h"
#include "Scene.h"
#include "Window.h"

void Scene::update()
{
	while (!model.empty())
		model.pop();
	model.push(glm::mat4(1.0f));

	int current_hierarchy = -1;
	for (auto it : container) {
		if (it.hierarchy <= current_hierarchy)
		{
			int times = current_hierarchy - it.hierarchy + 1;
			for (int i = 0; i < times; i++)
				model.pop();
		}
		it.update();
		current_hierarchy = it.hierarchy;
	}

	if (!!wind) {
		int w, h;
		glfwGetWindowSize(wind->getGLFWWindow(), &w, &h);
		if (w != 0 && h != 0)
			render_system.camera->aspect = (float)w / h;
		render_system.camera->getViewMat(view);
		render_system.camera->getProjMat(projection);
	}
}

Object* Scene::createObject(std::string name)
{
	Object* obj = new Object;
	obj->name = name;
	obj->location = this;
	obj->local_transform = glm::mat4(1.f);
	container.push_back(DAGNode(obj, 0));
	return obj;
}

Scene::Scene() :
	view(glm::mat4(1)),
	projection(glm::mat4(1)),
	wind(NULL)
{

}

void Scene::DAGNode::update()
{
	obj->update();
	auto& model_s = obj->location->model;
	model_s.push(model_s.top() * obj->local_transform);
	obj->world_transform = model_s.top();
}

