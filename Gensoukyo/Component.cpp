#include "pch.h"
#include "Component.h"
#include "Object.h"
#include "Shader.h"
#include "Window.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


void encodeTriangles(
	const vector<vec3>& vertices,
	const vector<ivec3>& faces,
	const vector<BoundingBox>& ordered_set,
	vector<float>& data)
{
	data.resize(faces.size() * 3 * 3);
	int i = 0;
	for (auto& bb : ordered_set) {
		ivec3 f = faces[bb.index];
		for (int m = 0; m < 3; m++)
			for (int n = 0; n < 3; n++)
				data[i++] = vertices[f[m]][n];
	}
}

void encodeTrianglesWithNormals(
	const vector<vec3>& vertices,
	const vector<ivec3>& faces,
	const vector<vec3>& normals,
	const vector<BoundingBox>& ordered_set,
	vector<float>& data)
{
	data.resize(faces.size() * 3 * 3 * 2);
	int i = 0;
	for (auto& bb : ordered_set) {
		ivec3 f = faces[bb.index];
		for (int m = 0; m < 3; m++) {
			for (int n = 0; n < 3; n++) {
				data[i] = vertices[f[m]][n];
				data[i + 3] = normals[f[m]][n];
				i++;
			}
			i += 3;
		}
	}
}

void encodeBVH(BVHNode* bvh, vector<float>& data) {
	int ptr = 0;
	data.clear();
	int used = 0;
	function<void(BVHNode*, int)> write;
	write = [&](BVHNode* node, int index) {
		int offset = index * 12;
		if (data.size() < offset + 12)
			data.resize(offset + 12);
		data[offset] = node->left == NULL ? -1 : (++used);
		data[offset + 1] = node->right == NULL ? -1 : (++used);
		data[offset + 2] = 0;
		data[offset + 3] = node->index;
		data[offset + 4] = node->n;
		data[offset + 5] = 0;
		memcpy(data.data() + offset + 6, &node->AA, 3 * sizeof(float));
		memcpy(data.data() + offset + 9, &node->BB, 3 * sizeof(float));
		if (node->left != NULL)
			write(node->left, data[offset]);
		if (node->right != NULL)
			write(node->right, data[offset + 1]);
	};
	write(bvh, 0);
}



Component::Component() :
	owner(NULL)
{

}

void Renderer::update()
{
	if (isVisible)
		render_system.render_buffer.push_back(this);
}

void GizmoRenderer::paint()
{
	//cout <<get_type_id()<< endl;
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

void Camera::getProjMat(mat4& mat) const
{
	mat = perspective(fov, aspect, near_z, far_z);
}

void Camera::getViewMat(mat4& mat) const
{
	if (!!owner) {
		mat = inverse(owner->local_transform);
	}
}

void Camera::update()
{
	if (render_system.camera == this) {
		double xf, yf;
		input_system.getScrollOffset(xf, yf);
		isChanged = false;
		if (yf != 0) {
			owner->local_transform = translate(owner->local_transform, vec3(0, 0, -5));
			owner->local_transform = scale(owner->local_transform, vec3(pow(0.9f, yf)));
			owner->local_transform = translate(owner->local_transform, vec3(0, 0, 5));
			isChanged = true;
		}
		input_system.getMouseOffset(xf, yf);
		if (input_system.getMouseButtonState(0)) {
			float angle = sqrt(xf * xf + yf * yf);
			if (angle != 0) {
				owner->local_transform = translate(owner->local_transform, vec3(0, 0, -5));
				owner->local_transform = rotate(owner->local_transform, angle * 0.01f, vec3(-yf, -xf, 0));
				owner->local_transform = translate(owner->local_transform, vec3(0, 0, 5));
				isChanged = true;
			}
		}
		if (input_system.getMouseButtonState(1)) {
			float dist = sqrt(xf * xf + yf * yf);
			if (dist != 0) {
				owner->local_transform = translate(owner->local_transform, 0.01f * vec3(-xf, yf, 0));
				isChanged = true;
			}
		}
	}
}

void Mesh::update() {
}

void Mesh::loadFromFile(string filename)
{
	vertices.clear();
	faces.clear();

	string sline;
	stringstream ss;
	fstream infile1(filename, ios::in);

	string str;
	while (getline(infile1, sline)) {
		if (sline[0] == 'v') {
			if (sline[1] == ' ') {
				float vx, vy, vz;
				ss.clear();
				ss.str(sline);
				ss >> str >> vx >> vy >> vz;
				vertices.push_back({ vx,vy,vz });
			}
			else if (sline[1] == 'n') {
				float nx, ny, nz;
				ss.clear();
				ss.str(sline);
				ss >> str >> nx >> ny >> nz;
				normals.push_back({ nx,ny,nz });
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

Mesh::Mesh(string obj) {
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
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(ivec3), &faces[0], GL_STATIC_DRAW);
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

	auto drawnode = [](auto&& fun, BVHNode* node, int depth) {
		if (depth > 5 || node->left == NULL && node->right == NULL) {
			vec3 verts[8];
			verts[0].x = verts[2].x = verts[4].x = verts[6].x = node->AA.x;
			verts[0].y = verts[1].y = verts[4].y = verts[5].y = node->AA.y;
			verts[0].z = verts[1].z = verts[2].z = verts[3].z = node->AA.z;
			verts[1].x = verts[3].x = verts[5].x = verts[7].x = node->BB.x;
			verts[2].y = verts[3].y = verts[6].y = verts[7].y = node->BB.y;
			verts[4].z = verts[5].z = verts[6].z = verts[7].z = node->BB.z;
			glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * 4, verts);
			glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
			return;
		}
		fun(fun, node->left, depth + 1);
		fun(fun, node->right, depth + 1);
	};
	if (bvh)
		drawnode(drawnode, bvh, 0);
}

BVHRenderer::BVHRenderer() :
	bvh(NULL)
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
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(vec3), NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	shader = shader_asset.asset["default"];
}

RayTracingRenderer::RayTracingRenderer() {


	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glGenBuffers(2, TBO);
	glGenTextures(2, TEX);
	for (int i = 0; i < 2; i++) {
		glBindBuffer(GL_TEXTURE_BUFFER, TBO[i]);
		glBindTexture(GL_TEXTURE_BUFFER, TEX[i]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, TBO[i]);
	}

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glGenTextures(1, &SUM_TEX);
	glBindTexture(GL_TEXTURE_2D, SUM_TEX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SUM_TEX, 0);

	glGenBuffers(1, &SUM_VBO);
	glGenBuffers(1, &SUM_EBO);

	glGenFramebuffers(1, &PRE_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, PRE_FBO);
	glGenTextures(1, &PRE_TEX);
	glBindTexture(GL_TEXTURE_2D, PRE_TEX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 100, 75, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PRE_TEX, 0);

	int w, h, comp;
	float* img = stbi_loadf("hdr/kay.hdr", &w, &h, &comp, 4);
	cout << w << h << endl;
	glGenTextures(1, &HDR);
	glBindTexture(GL_TEXTURE_2D, HDR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(img);

	shader = shader_asset.asset["ray_tracing"];
	shader_pass2 = shader_asset.asset["ray_tracing_pass2"];
}

void RayTracingRenderer::paint() {
	static bool first = true;

	if (watcher->isChanged) {
		render_proc = -1;
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	if (first) {
		auto mesh = owner->getComponent<Mesh>();
		if (mesh) {
			glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(vec3), &mesh->vertices[0], GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->faces.size() * sizeof(ivec3), &mesh->faces[0], GL_STATIC_DRAW);
		}

		vector<vec3> vert_2 = { {-1, -1, -1}, { 1,-1,-1 }, { -1,1,-1 }, { 1,1,-1 } };
		vector<ivec3> indice_2 = { {0, 1, 3}, { 0,3,2 } };
		glBindBuffer(GL_ARRAY_BUFFER, SUM_VBO);
		glBufferData(GL_ARRAY_BUFFER, vert_2.size() * sizeof(vec3), &vert_2[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SUM_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice_2.size() * sizeof(ivec3), &indice_2[0], GL_STATIC_DRAW);


		vector<float> data;
		if (rayTracingData) {
			auto rt_mesh = rayTracingData->getComponent<Mesh>();
			auto rt_bvh = rayTracingData->getComponent<BVHRenderer>();
			if (rt_mesh && rt_bvh) {
				encodeTrianglesWithNormals(rt_mesh->vertices, rt_mesh->faces, rt_mesh->normals, rt_bvh->boxs, data);
				glBindBuffer(GL_TEXTURE_BUFFER, TBO[0]);
				glBufferData(GL_TEXTURE_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
				encodeBVH(rt_bvh->bvh, data);
				glBindBuffer(GL_TEXTURE_BUFFER, TBO[1]);
				glBufferData(GL_TEXTURE_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
			}
		}


		first = false;
	}
	//pass 1:
	shader.use();
	if (render_proc >= 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, PRE_FBO);
		glViewport(0, 0, 100, 75);
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	if (render_proc <= 0)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, (GLfloat*)&owner->world_transform);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, (GLfloat*)&owner->location->view);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "camera_transform"), 1, GL_FALSE, (GLfloat*)&render_system.camera->owner->local_transform);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, (GLfloat*)&owner->location->projection);
	//glUniform1i(glGetUniformLocation(shader.ID, "rand_seed"), render_proc++);
	render_proc++;
	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_TEXTURE_BUFFER, TBO[0]);
	glBindTexture(GL_TEXTURE_BUFFER, TEX[0]);
	glUniform1i(glGetUniformLocation(shader.ID, "triangles"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindBuffer(GL_TEXTURE_BUFFER, TBO[1]);
	glBindTexture(GL_TEXTURE_BUFFER, TEX[1]);
	glUniform1i(glGetUniformLocation(shader.ID, "BVH_nodes"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,HDR);
	glUniform1i(glGetUniformLocation(shader.ID, "hdr"), 2);

	if (render_proc != 0) {
		for (int i = 0; i < 10; i++) {
			glUniform1i(glGetUniformLocation(shader.ID, "rand_seed"), (render_proc - 1) * 10 + i);
			glDrawElements(GL_TRIANGLES, owner->getComponent<Mesh>()->faces.size() * 3, GL_UNSIGNED_INT, 0);
		}
	}
	else {
		for (int i = 0; i < 100; i++) {
			glUniform1i(glGetUniformLocation(shader.ID, "rand_seed"), render_proc + i);
			glDrawElements(GL_TRIANGLES, owner->getComponent<Mesh>()->faces.size() * 3, GL_UNSIGNED_INT, 0);
		}
	}
	//pass 2:	
	shader_pass2.use();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, SUM_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SUM_EBO);
	glActiveTexture(GL_TEXTURE0);
	if (render_proc != 0) {
		glBindTexture(GL_TEXTURE_2D, SUM_TEX);
		glUniform1i(glGetUniformLocation(shader_pass2.ID, "process"), render_proc * 10);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, PRE_TEX);
		glUniform1i(glGetUniformLocation(shader_pass2.ID, "process"), 100);
	}
	glUniform1i(glGetUniformLocation(shader_pass2.ID, "light"), 0);
	glBlendFunc(GL_ONE, GL_ZERO);
	glViewport(0, 0, 800, 600);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}