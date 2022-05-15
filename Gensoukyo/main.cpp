
#include "pch.h"
#include "Shader.h"
#include "Window.h"

bool myframe(Window* wind)
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
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Object Tree");

	// 在滚动区域中显示内容
	auto tree = wind->getObjectHierarchy();
	ImGui::BeginChild("Scrolling");
	for (size_t i = 0; i < tree.size(); i++) {
		int cur_hie = tree[i].hierarchy;
		if (i + 1 == tree.size() || tree[i + 1].hierarchy <= cur_hie) {
			ImGui::Text(std::string("   ").append(tree[i].obj->name).c_str());
		}
		else {
			bool is_open = ImGui::TreeNode(tree[i].obj->name.c_str());
			if (!is_open)
				while (!(i + 1 == tree.size() || tree[i + 1].hierarchy <= cur_hie))
					i++;
		}
		int end = 0;
		if (i + 1 != tree.size())
			end = tree[i + 1].hierarchy;
		int times = cur_hie - end;
		for (int j = 0; j < times; j++)
			ImGui::TreePop();
	}

	ImGui::EndChild();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	return true;
}

int main() {
	Window my_window;
	my_window.initialize();
	my_window.setFrameCallback(myframe);
	my_window.run();

	return 0;
}