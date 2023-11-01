
#include"Window.h"
#include<list>
#include"ImFileDialog.h"
class TopWindow :public Window {
public:
	void render() override {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open File")) {
                    ifd::FileDialog::Instance().Open("TextureOpenDialog", "Open a texture", "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
                }
                if (ImGui::BeginMenu("Open history")) {
                    for (auto& dir : directory) {
                        if (ImGui::MenuItem(dir.c_str())) {
                            std::cout << "Opening..." << std::endl;
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Object")) {
                if(ImGui::BeginMenu("model")) {
                    
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("basic object")) {
                    if (ImGui::MenuItem("Sphere")) {
                        object_index = sphere;
                    }
                    if (ImGui::MenuItem("Plane")) {
                        object_index = plane;
                    }
                    if (ImGui::MenuItem("Cube")) {
                        object_index = cube;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Scene")) {
                if (ImGui::MenuItem("IBL")) {
                    scene_index = IBL;
                }
                if (ImGui::MenuItem("LTC")) {
                    scene_index = LTC;
                }
                if (ImGui::MenuItem("RTX")) {
                    scene_index = RTX;
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMainMenuBar();
        }
        if (ifd::FileDialog::Instance().IsDone("TextureOpenDialog")) {
            if (ifd::FileDialog::Instance().HasResult()) {
                directory.push_back(ifd::FileDialog::Instance().GetResult().u8string());
                if (directory.size() >= 6)
                    directory.pop_front();
            }
            ifd::FileDialog::Instance().Close();
        }

	}
private:
	std::list<std::string> directory;
};