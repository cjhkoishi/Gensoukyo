#pragma once
#include "pch.h"
#include "Scene.h"
#include "Shader.h"
typedef bool (*FrameCallback)(Window*);

class RenderSystem {
public:
	std::list<Renderer*> render_buffer;
	Camera* camera=NULL;

	void run();
};

class InputSystem {
private:
	bool mouse[5]{0,0,0,0,0};
	double mouse_pos[2]{0,0};
	double scroll_offset[2]{0,0};
	double last_mouse_pos[2]{ 0,0 };
public:	
	bool getMouseButtonState(int index) const{return mouse[index];};
	void getMousePos(double& x, double& y) { x = mouse_pos[0]; y = mouse_pos[1]; };
	void getScrollOffset(double& xf, double& yf) {
		xf = scroll_offset[0]; yf = scroll_offset[1]; scroll_offset[0] = scroll_offset[1] = 0;
	};
	void getMouseOffset(double& xf, double& yf) {
		xf = mouse_pos[0]-last_mouse_pos[0]; yf = mouse_pos[1] - last_mouse_pos[1]; 
		last_mouse_pos[0] = mouse_pos[0];
		last_mouse_pos[1] = mouse_pos[1];
	};
	friend class Window;
};

class ShaderAsset {
public:
	std::map<std::string, Shader> asset;
};

extern ShaderAsset shader_asset;
extern RenderSystem render_system;
extern InputSystem input_system;

class Window
{
protected:
	GLFWwindow* window;
	FrameCallback frame;

	Scene current_scene;

	
public:	

	void setFrameCallback(FrameCallback frame) { this->frame = frame; };
	bool initialize();
	void run();
	GLFWwindow* getGLFWWindow() { return window; };


	std::vector<Scene::DAGNode> getObjectHierarchy();

	Window();
	~Window();
};

