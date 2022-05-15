#include "pch.h"
#include "Object.h"

glm::vec3 Object::getPosition()
{
	return local_transform[3];
}

glm::quat Object::getRotation()
{
	const glm::mat3 rotMtx(
		glm::vec3(local_transform[0]) / glm::length(local_transform[0]),
		glm::vec3(local_transform[1]) / glm::length(local_transform[1]),
		glm::vec3(local_transform[2]) / glm::length(local_transform[2])
	);
	auto res=glm::quat_cast(rotMtx);
	return res;
}

glm::vec3 Object::getScale()
{
	return glm::vec3(glm::length(local_transform[0]), glm::length(local_transform[1]), glm::length(local_transform[2]));
}

void Object::setPosition(const glm::vec3& position)
{
	local_transform[3] = glm::vec4(position, 1);
}

void Object::setRotation(const glm::quat& rotation)
{
	auto rot = glm::mat3_cast(rotation);
	rot[0] *= glm::length(local_transform[0]);
	rot[1] *= glm::length(local_transform[1]);
	rot[2] *= glm::length(local_transform[2]);
	local_transform[0] = glm::vec4(rot[0], 0);
	local_transform[1] = glm::vec4(rot[1], 0);
	local_transform[2] = glm::vec4(rot[2], 0);
}

void Object::setScale(const glm::vec3& scale)
{
	local_transform[0] = glm::normalize(local_transform[0]) * scale[0];
	local_transform[1] = glm::normalize(local_transform[1]) * scale[1];
	local_transform[2] = glm::normalize(local_transform[2]) * scale[2];
}

void Object::update()
{
	for (auto it : components) {
		it->update();
	}
}
