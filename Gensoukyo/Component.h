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

	void getProjMat(glm::mat4& mat) const;
	void getViewMat(glm::mat4& mat) const;


	void update() override;
};

class Renderer :public Component
{
	COMPONENT
public:
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
	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> faces;

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
	unsigned VBO, VAO, EBO;

	void paint() override;
	BVHRenderer();
};

class RayTracingRenderer :public Renderer {
	COMPONENT
};

