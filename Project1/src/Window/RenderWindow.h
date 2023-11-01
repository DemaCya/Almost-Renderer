#pragma once
#include"Window.h"
#include"all_Include.h"
class RenderWindow :public Window {
public:
	void render() override {
        ImGui::Begin("Game render");
        ImGui::BeginChild("GameRender");
        ImVec2 wsize = ImGui::GetWindowSize();
        ImGui::Image((ImTextureID)imGUItexture, wsize, ImVec2(0, 1), ImVec2(1, 0));
        if (ImGui::IsItemHovered())
            ImGui::CaptureMouseFromApp(false);

        ImGui::EndChild();
        ImGui::End();
	}
};