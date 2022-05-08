
#include "pch.h"
#include "Shader.h"
#include "Window.h"

bool myframe()
{
	glClearColor(0, 0, 0, 0);

	bool my_tool_active;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// 创建一个带有菜单栏的名为“我的第一个工具”的窗口。
	ImGui::Begin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
			if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
			if (ImGui::MenuItem("Close", "Ctrl+W")) { my_tool_active = false; }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	// 编辑颜色 (stored as ~4 floats)
	float my_color[] = { 0.2f, 0.1f, 1.0f, 0.5f };
	ImGui::ColorEdit4("Color", my_color);

	// Plot some values
	const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
	ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

	// 在滚动区域中显示内容
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
	ImGui::BeginChild("Scrolling");
	for (int n = 0; n < 50; n++)
		ImGui::Text("%04d: Some text", n);
	ImGui::EndChild();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	return true;
}

int main() {
	
	Window my_window;
	my_window.initialize();
	Shader def("Shader/default.vert", "Shader/default.frag");
	def.use();
	my_window.setFrameCallback(myframe);
	my_window.run();
	
	return 0;
}