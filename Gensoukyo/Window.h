#pragma once
#include "pch.h"
#include "Scene.h"
typedef bool (*FrameCallback)();

class RenderSystem {
protected:
	std::list<Renderer*> render_buffer;
	glm::mat4 current_transform;
public:
};

class Window
{
protected:
	GLFWwindow* window;
	GLFWframebuffersizefun resize;
	FrameCallback frame;

	Scene current_scene;
	RenderSystem render_system;
public:
	void setFrameCallback(FrameCallback frame) { this->frame = frame; };
	bool initialize();
	void run();

	Window();
	~Window();
};

