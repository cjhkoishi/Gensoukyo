#pragma once
#include "pch.h"
#include "Component.h"
class Scene;
class Object
{
protected:

public:
	glm::mat4 local_transform;	
	glm::mat4 world_transform;
	std::string name;

	std::vector<Component*> components;
	Scene* location;

	glm::vec3 getPosition();
	glm::quat getRotation();
	glm::vec3 getScale();

	void setPosition(const glm::vec3& position);
	void setRotation(const glm::quat& rotation);
	void setScale(const glm::vec3& scale);

	template<typename T> T* GetComponent();
	void update();
	template<typename T> T* addComponent();
};

template<typename T>
inline T* Object::GetComponent()
{
	return nullptr;
}

template<typename T>
inline T* Object::addComponent()
{
	T* comp=new T;
	comp->owner = this;
	components.push_back(comp);
	return comp;
}
