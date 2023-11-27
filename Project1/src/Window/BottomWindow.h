#include<all_Include.h>
#include"Window.h"
class BottomWindow :public Window {
public:
	string metallic_dir;
	string roughness_dir;
	string normal_dir;
	string ao_dir;
	string albedo_dir;
	void render() override {
		ImGui::Begin("Operate World3", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar| ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		if (ImGui::Button("   change metallic   ")) {
			ifd::FileDialog::Instance().Open("change metallic", "Open a texture", "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
		}
		
		ImGui::SameLine(179.0f, -1.0f);
		if (ImGui::Button("   change roughness  ")) {
			ifd::FileDialog::Instance().Open("change roughness", "Open a texture", "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
		}
		ImGui::SameLine(179.0f*2, -1.0f);
		if (ImGui::Button("    change Normal    ")) {
			ifd::FileDialog::Instance().Open("change Normal", "Open a texture", "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
		}
		ImGui::SameLine(179.0f * 3, -1.0f);
		if (ImGui::Button("      change AO      ")) {
			ifd::FileDialog::Instance().Open("change AO", "Open a texture", "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
		}
		ImGui::SameLine(179.0f * 4, -1.0f);
		if (ImGui::Button("    change Albedo    ")) {
			ifd::FileDialog::Instance().Open("change albedo", "Open a texture", "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
		}
		ImGui::Image(use_metallicMap ? (void*)(intptr_t)Metallic : nullptr, ImVec2(155.2f, 155.2f));
		ImGui::SameLine(179.0f, -1.0f);
		ImGui::Image(use_roughnessMap ? (void*)(intptr_t)Roughness : nullptr, ImVec2(155.2f, 155.2f));
		ImGui::SameLine(179.0f * 2, -1.0f);
		ImGui::Image(use_normalMap ? (void*)(intptr_t)Normal : nullptr, ImVec2(155.2f, 155.2f));
		ImGui::SameLine(179.0f * 3, -1.0f);
		ImGui::Image(use_aoMap ? (void*)(intptr_t)AO : nullptr, ImVec2(155.2f, 155.2f));
		ImGui::SameLine(179.0f * 4, -1.0f);
		ImGui::Image(use_albedoMap ? (void*)(intptr_t)Albedo : nullptr, ImVec2(155.2f, 155.2f));
		if (ifd::FileDialog::Instance().IsDone("change metallic")) {
			if (ifd::FileDialog::Instance().HasResult()) {
				metallic_dir = ifd::FileDialog::Instance().GetResult().u8string();
				Metallic = loadTexture(metallic_dir.c_str());
				use_metallicMap = true;
			}
			ifd::FileDialog::Instance().Close();
		}
		if (ifd::FileDialog::Instance().IsDone("change roughness")) {
			if (ifd::FileDialog::Instance().HasResult()) {
				roughness_dir = ifd::FileDialog::Instance().GetResult().u8string();
				Roughness = loadTexture(roughness_dir.c_str());
				use_roughnessMap = true;
			}
			ifd::FileDialog::Instance().Close();
		}
		if (ifd::FileDialog::Instance().IsDone("change Normal")) {
			if (ifd::FileDialog::Instance().HasResult()) {
				normal_dir = ifd::FileDialog::Instance().GetResult().u8string();
				Normal = loadTexture(roughness_dir.c_str());
				use_normalMap = true;
			}
			ifd::FileDialog::Instance().Close();
		}
		if (ifd::FileDialog::Instance().IsDone("change AO")) {
			if (ifd::FileDialog::Instance().HasResult()) {
				ao_dir = ifd::FileDialog::Instance().GetResult().u8string();
				AO = loadTexture(ao_dir.c_str());
				use_aoMap = true;
			}
			ifd::FileDialog::Instance().Close();
		}
		if (ifd::FileDialog::Instance().IsDone("change albedo")) {
			if (ifd::FileDialog::Instance().HasResult()) {
				albedo_dir = ifd::FileDialog::Instance().GetResult().u8string();
				 Albedo = loadTexture(albedo_dir.c_str());
				use_albedoMap = true;
			}
			ifd::FileDialog::Instance().Close();
		}
		ImGui::End();
	}
};