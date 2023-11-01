#pragma once
#include"Window.h"
#include<list>
#include"ImFileDialog.h"
#include"global_data.h"
class LeftWindow :public Window {
public:
	void render() override {
		ImGui::Begin("Operate World", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginTabBar("back ground", ImGuiTabBarFlags_None)) {
			if (ImGui::BeginTabItem("Global Settings")) {
				ImGui::Checkbox("use skybox", &use_skybox);
				ImGui::Checkbox("use plane", &use_plane);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Animation")) {
				enum animation_Mode
				{
					rotate,
					swing,
					rotate_jump,
					hover,
					load_animation
				};
				static int mode = 6;
				if (ImGui::RadioButton("rotate", mode == rotate)) {
					mode = rotate;
				}
				if (ImGui::RadioButton("swing", mode == swing)) {
					mode = swing;
				}
				if (ImGui::RadioButton("rotate_jump", mode == rotate_jump)) {
					mode = rotate_jump;
				}
				if (ImGui::RadioButton("hover", mode == hover)) {
					mode = hover;
				}
				if (ImGui::RadioButton("load_animation", mode == load_animation)) {
					mode = load_animation;
				}
				if (ImGui::RadioButton("NULL", mode == 6)) {
					mode = 6;
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();

		ImGui::Begin("Operate World2", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginTabBar("Settings", ImGuiTabBarFlags_None)) {
			if (ImGui::BeginTabItem("Model Settings")) {
				ImGui::ColorPicker3("Albedo", (float*)&_albedo, ImGuiColorEditFlags_None | ImGuiColorEditFlags_DisplayRGB);
				ImGui::SliderFloat("Metallic", &_metallic, 0.0f, 1.0f);
				ImGui::SliderFloat("Roughness", &_roughness, 0.0f, 1.0f);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Model Information")) {
				enum Mode {
					Vertex,
					triangle,
				};
				static int mode = 3;
				if (ImGui::RadioButton("Vertex",mode==Vertex)) {
					mode = Vertex;
					renderMode = GL_POINT;
				}
				if (ImGui::RadioButton("triangle", mode == triangle)) {
					mode = triangle;
					renderMode = GL_LINE;
				}
				if (ImGui::RadioButton("Null", mode == 3)) {
					mode = 3;
					renderMode = GL_FILL;
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();

	}
};
