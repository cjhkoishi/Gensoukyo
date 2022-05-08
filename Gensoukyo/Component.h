#pragma once
class Object;
class Component
{
public:
	Object* owner;

	virtual void update() = 0;
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
	void paint(const glm::mat4& transform) override;
};

