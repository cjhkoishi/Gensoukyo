#include "pch.h"
#include "Component.h"
#include "Object.h"
#include "Shader.h"
#include "Window.h"

void Renderer::update()
{
	render_system.render_buffer.push_back(this);
}

void GizmoRenderer::paint()
{
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
			owner->local_transform = glm::scale(owner->local_transform, glm::vec3(pow(0.9, yf)));
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
				owner->local_transform = glm::translate(owner->local_transform, 0.01f*glm::vec3(-xf, yf, 0));
			}
		}
	}
}
