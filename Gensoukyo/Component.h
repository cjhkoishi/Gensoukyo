#pragma once
#include "Shader.h"
class Object;
class Component
{
public:
	Object* owner;

	virtual void update() = 0;
};

class Camera:public Component
{
public:
	float fov;
	float near_z, far_z;
	float aspect;

	void getProjMat(glm::mat4 &mat) const;
	void getViewMat(glm::mat4 &mat) const;


	void update() override;
};

class Renderer :public Component
{
public:
	void update() override;

	virtual void paint() = 0;
};

class GizmoRenderer :public Renderer
{
public:
	Shader shad;
	unsigned VBO, VAO, EBO;

	void paint() override;

	GizmoRenderer();
};

