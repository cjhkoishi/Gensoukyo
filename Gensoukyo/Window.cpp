#include "pch.h"
#include "Window.h"
#include "BVH.h"

ShaderAsset shader_asset;
RenderSystem render_system;
InputSystem input_system;

bool Window::initialize()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_SAMPLES, 16);
	window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	auto resize = [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	};
	auto mouse = [](GLFWwindow* window, int button, int action, int mods) {
		input_system.mouse[button] = action;
	};
	auto move = [](GLFWwindow* window, double xpos, double ypos) {
		input_system.last_mouse_pos[0] = input_system.mouse_pos[0];
		input_system.last_mouse_pos[1] = input_system.mouse_pos[1];
		input_system.mouse_pos[0] = xpos;
		input_system.mouse_pos[1] = ypos;
	};
	auto scroll = [](GLFWwindow* window, double xoffset, double yoffset) {
		input_system.scroll_offset[0] += xoffset;
		input_system.scroll_offset[1] += yoffset;
	};
	//std::cout << get(window) << std::endl;
	glfwSetFramebufferSizeCallback(window, resize);
	glfwSetMouseButtonCallback(window, mouse);
	glfwSetCursorPosCallback(window, move);
	glfwSetScrollCallback(window, scroll);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	//glEnable(GL_MULTISAMPLE);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	shader_asset.asset["default"].compile("shader/default.vert", "shader/default.frag");
	shader_asset.asset["mesh"].compile("shader/mesh.vert", "shader/mesh.frag");
	shader_asset.asset["ray_tracing"].compile("shader/ray_tracing.vert", "shader/ray_tracing.frag");
	shader_asset.asset["ray_tracing_pass2"].compile("shader/ray_tracing_pass2.vert", "shader/ray_tracing_pass2.frag");
	current_scene.wind = this;

	return true;
}

void Window::run()
{

	auto camera_obj = current_scene.createObject("camera");
	auto camera = camera_obj->addComponent<Camera>();

	auto mask = current_scene.createObject("ray_tracing_mask");
	current_scene.container[1].hierarchy = 1;
	auto mask_mesh = mask->addComponent<Mesh>();
	mask_mesh->vertices = { {0,-1,-1},{1,-1,-1},{0,1,-1},{1,1,-1} };
	mask_mesh->faces = { {0,1,3},{0,3,2} };
	auto rt=mask->addComponent<RayTracingRenderer>();
	rt->watcher = camera;
	

	render_system.camera = camera;
	camera->fov = glm::radians(45.f);
	camera->aspect = 800 / 600.f;
	camera->near_z = 0.01f;
	camera->far_z = 1000;
	camera_obj->setPosition(glm::vec3(0, 0, 5));

	auto my_f = current_scene.createObject("bunny");
	auto mesh = my_f->addComponent<Mesh>();
	mesh->loadFromFile("models/bunny.obj");
	my_f->addComponent<MeshRenderer>()->isVisible=true;
	std::vector<BoundingBox> boxs(mesh->faces.size());
	auto& F = mesh->faces;
	auto& V = mesh->vertices;
	for (int i = 0; i < boxs.size(); i++) {
		boxs[i].AA.x = std::min(std::min(V[F[i][0]].x, V[F[i][1]].x), V[F[i][2]].x);
		boxs[i].AA.y = std::min(std::min(V[F[i][0]].y, V[F[i][1]].y), V[F[i][2]].y);
		boxs[i].AA.z = std::min(std::min(V[F[i][0]].z, V[F[i][1]].z), V[F[i][2]].z);
		boxs[i].BB.x = std::max(std::max(V[F[i][0]].x, V[F[i][1]].x), V[F[i][2]].x);
		boxs[i].BB.y = std::max(std::max(V[F[i][0]].y, V[F[i][1]].y), V[F[i][2]].y);
		boxs[i].BB.z = std::max(std::max(V[F[i][0]].z, V[F[i][1]].z), V[F[i][2]].z);
		boxs[i].center = (V[F[i][0]] + V[F[i][1]] + V[F[i][2]]) / 3.f;
		boxs[i].index = i;
	}
	BVHNode* bunny = constructBVH(boxs, 0, boxs.size() - 1, 2);
	auto bvh_data = my_f->addComponent<BVHRenderer>();
	bvh_data->bvh = bunny;
	bvh_data->boxs = boxs;
	//bvh_data->isVisible = false;
	
	rt->rayTracingData = my_f;


	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		current_scene.update();

		//render system
		render_system.run();

		//GUI system
		if (frame != NULL)
		{
			bool res = frame(this);
			if (!res)
				break;
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

std::vector<Scene::DAGNode> Window::getObjectHierarchy()
{
	return current_scene.container;
}

Window::Window() :
	window(NULL),
	frame(NULL)
{
}

Window::~Window()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}

void RenderSystem::run()
{

	for (auto it : render_buffer) {
		it->paint();
	}
	render_buffer.clear();
}
