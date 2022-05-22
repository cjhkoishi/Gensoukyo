#pragma once
#include "Shader.h"
#include "BVH.h"
#define COMPONENT public: virtual size_t get_type_id() {return typeid(*this).hash_code();};


class Object;
class Component
{
	COMPONENT
public:
	Object* owner = NULL;

	virtual void update() = 0;
	Component();
};

class Camera :public Component
{
	COMPONENT
public:
	float fov;
	float near_z, far_z;
	float aspect;
	bool isChanged = false;

	void getProjMat(glm::mat4& mat) const;
	void getViewMat(glm::mat4& mat) const;


	void update() override;
};

class Renderer :public Component
{
	COMPONENT
public:
	bool isVisible = true;

	void update() override;

	virtual void paint() = 0;
};

class GizmoRenderer :public Renderer
{
	COMPONENT
public:
	Shader shad;
	unsigned VBO, VAO, EBO;

	void paint() override;

	GizmoRenderer();
};

class Mesh :public Component
{
	COMPONENT
public:
	vector<vec3> vertices;
	vector<ivec3> faces;
	vector<vec3> normals;

	void update() override;
	void loadFromFile(std::string filename);

	Mesh(std::string obj = "");
};

class MeshRenderer :public Renderer
{
	COMPONENT
private:
	bool isDataUpdated = false;
public:
	Shader shader;
	unsigned VBO, VAO, EBO;
	int mode = 0;

	void paint() override;
	void updateBufferData();
	MeshRenderer();
};

class BVHRenderer :public Renderer {
	COMPONENT
public:
	Shader shader;
	BVHNode* bvh;
	vector<BoundingBox> boxs;
	unsigned VBO, VAO, EBO;


	void paint() override;
	BVHRenderer();
};

class RayTracingRenderer :public Renderer {
	COMPONENT
public:
	Camera* watcher;
	GLuint TBO[2], TEX[2], VAO, VBO, EBO, FBO, SUM_TEX, PRE_FBO, PRE_TEX, SUM_VBO, SUM_EBO;
	Shader shader, shader_pass2;
	Object* rayTracingData = NULL;
	int render_proc = 0;

	void paint() override;

	RayTracingRenderer();
};

