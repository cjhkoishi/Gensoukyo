#include "pch.h"
#include "Component.h"
#include "Object.h"
#include "Shader.h"
#include "Window.h"

Component::Component() :
	owner(NULL)
{

}

void Renderer::update()
{
	render_system.render_buffer.push_back(this);
}

void GizmoRenderer::paint()
{
	//std::cout <<get_type_id()<< std::endl;
	shad.use();
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glUniformMatrix4fv(glGetUniformLocation(shad.ID, "model"), 1, GL_FALSE, (GLfloat*)&owner->world_transform);
	glUniformMatrix4fv(glGetUniformLocation(shad.ID, "view"), 1, GL_FALSE, (GLfloat*)&owner->location->view);
	glUniformMatrix4fv(glGetUniformLocation(shad.ID, "projection"), 1, GL_FALSE, (GLfloat*)&owner->location->projection);
	glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);
}

GizmoRenderer::GizmoRenderer()
{
	float vertices[12]{
	0,0,0,
	1,0,0,
	0,1,0,
	0,0,1,
	};
	unsigned indices[6]{
	0,1,0,2,0,3,
	};

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	shad = shader_asset.asset["default"];

}

void Camera::getProjMat(glm::mat4& mat) const
{
	mat = glm::perspective(fov, aspect, near_z, far_z);
}

void Camera::getViewMat(glm::mat4& mat) const
{
	if (!!owner) {
		mat = glm::inverse(owner->local_transform);
	}
}

void Camera::update()
{
	if (render_system.camera == this) {
		double xf, yf;
		input_system.getScrollOffset(xf, yf);
		if (yf != 0) {
			owner->local_transform = glm::translate(owner->local_transform, glm::vec3(0, 0, -5));
			owner->local_transform = glm::scale(owner->local_transform, glm::vec3(pow(0.9f, yf)));
			owner->local_transform = glm::translate(owner->local_transform, glm::vec3(0, 0, 5));
		}
		input_system.getMouseOffset(xf, yf);
		if (input_system.getMouseButtonState(0)) {
			float angle = sqrt(xf * xf + yf * yf);
			if (angle != 0) {
				owner->local_transform = glm::translate(owner->local_transform, glm::vec3(0, 0, -5));
				owner->local_transform = glm::rotate(owner->local_transform, angle * 0.01f, glm::vec3(-yf, -xf, 0));
				owner->local_transform = glm::translate(owner->local_transform, glm::vec3(0, 0, 5));
			}
		}
		if (input_system.getMouseButtonState(1)) {
			float dist = sqrt(xf * xf + yf * yf);
			if (dist != 0) {
				owner->local_transform = glm::translate(owner->local_transform, 0.01f * glm::vec3(-xf, yf, 0));
			}
		}
	}
}

void Mesh::update() {
}

void Mesh::loadFromFile(std::string filename)
{
	vertices.clear();
	faces.clear();

	std::string sline;
	std::stringstream ss;
	std::fstream infile1(filename, std::ios::in);

	std::string str;
	while (std::getline(infile1, sline)) {
		if (sline[0] == 'v') {
			if (sline[1] == ' ') {
				float vx, vy, vz;
				ss.clear();
				ss.str(sline);
				ss >> str >> vx >> vy >> vz;
				vertices.push_back({ vx,vy,vz });
			}
		}
		if (sline[0] == 'f') {
			int f[3];
			ss.clear();
			ss.str(sline);
			ss >> str >> f[0] >> f[1] >> f[2];
			faces.push_back({ f[0] - 1,f[1] - 1,f[2] - 1 });
		}
	}
	infile1.close();
}

Mesh::Mesh(std::string obj) {
	if (obj == "") {
		vertices = {
			{-1,-1,1},
			{1,-1,1},
			{-1,1,1},
			{1,1,1},
			{-1,-1,-1},
			{1,-1,-1},
			{-1,1,-1},
			{1,1,-1},
		};
		faces = {
			{0,1,3},
			{0,3,2},
			{4,7,5},
			{4,6,7},

			{0,6,4},
			{0,2,6},
			{1,5,7},
			{1,7,3},

			{0,5,1},
			{0,4,5},
			{2,3,7},
			{2,7,6},
		};
	}
}

void MeshRenderer::paint()
{
	if (!isDataUpdated) {
		updateBufferData();
		isDataUpdated = true;
	}
	shader.use();
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, (GLfloat*)&owner->world_transform);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, (GLfloat*)&owner->location->view);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "camera_transform"), 1, GL_FALSE, (GLfloat*)&render_system.camera->owner->local_transform);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, (GLfloat*)&owner->location->projection);
	if (owner)
		glDrawElements(GL_TRIANGLES, owner->getComponent<Mesh>()->faces.size() * 3, GL_UNSIGNED_INT, 0);
}

void MeshRenderer::updateBufferData()
{
	auto mesh = owner->getComponent<Mesh>();
	auto& vertices = mesh->vertices;
	auto& faces = mesh->faces;
	if (mesh) {
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(glm::ivec3), &faces[0], GL_STATIC_DRAW);
	}
}

MeshRenderer::MeshRenderer() :
	VBO(0),
	VAO(0),
	EBO(0),
	shader(shader_asset.asset["mesh"])
{
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

}

void BVHRenderer::paint()
{
	shader.use();
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, (GLfloat*)&owner->world_transform);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, (GLfloat*)&owner->location->view);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "camera_transform"), 1, GL_FALSE, (GLfloat*)&render_system.camera->owner->local_transform);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, (GLfloat*)&owner->location->projection);

	auto drawnode = [](auto&& fun,BVHNode* node) {
		if (node->left == NULL && node->right == NULL) {
			glm::vec3 verts[8];
			verts[0].x = verts[2].x = verts[4].x = verts[6].x = node->AA.x;
			verts[0].y = verts[1].y = verts[4].y = verts[5].y = node->AA.y;
			verts[0].z = verts[1].z = verts[2].z = verts[3].z = node->AA.z;
			verts[1].x = verts[3].x = verts[5].x = verts[7].x = node->BB.x;
			verts[2].y = verts[3].y = verts[6].y = verts[7].y = node->BB.y;
			verts[4].z = verts[5].z = verts[6].z = verts[7].z = node->BB.z;
			glBufferSubData(GL_ARRAY_BUFFER, 0, 24*4, verts);
			glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
			return;
		}
		fun(fun,node->left);
		fun(fun,node->right);
	};
	drawnode(drawnode,bvh);
}

BVHRenderer::BVHRenderer()
{
	unsigned indices[24]{
		0,1,
		2,3,
		4,5,
		6,7,
		0,2,
		1,3,
		4,6,
		5,7,
		0,4,
		1,5,
		2,6,
		3,7,
	};

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	shader=shader_asset.asset["default"];
}
