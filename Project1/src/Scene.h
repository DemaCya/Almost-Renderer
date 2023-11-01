#pragma once
#include<string>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include"stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<vector>
#include <iostream>
#include "shader.h"
#include"Camera.h"
#include"Model.h"
#include<random>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include"Object3D.h"
#include"utility.h"
#include"ImFileDialog.h"
#include"global_data.h"

namespace
{
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    bool is_left_hold = false;
    bool firstMouse = true;
    float lastX = 1280.0f / 2.0;
    float lastY = 768.0 / 2.0;
    void camera_callback(GLFWwindow* window, double xpos, double ypos);

    void mouse_clickCallback(GLFWwindow* window, int button, int state, int mod) {
        ImGuiIO& io = ImGui::GetIO();

        if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
            is_left_hold = true;
            firstMouse = true;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_RELEASE) {
            is_left_hold = false;
        }

        if (is_left_hold && !io.WantCaptureMouse) {
            glfwSetCursorPosCallback(window, camera_callback);
            //camera_move_by_mouse(window);

        }
        if (!is_left_hold) {
            glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
        }

    }
    void camera_callback(GLFWwindow* window, double xpos, double ypos) {
        if (firstMouse) // 这个bool变量初始时是设定为true的
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);

    }
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }
}
class Scene {
public:
    GLFWwindow* window;
	const GLuint SCR_WIDTH = 1280,SCR_HEIGHT = 768;

	bool creatWindow();
    glm::vec2 GetFrameBufferSize();
    unsigned int back_ground(unsigned int texture);
    void renderplane(Shader shader);
};
void Scene::renderplane(Shader shader) {
    auto planeTexture = loadTexture("E:/LearnOpenGL/Project1/Project1/Texture/planeTexture/grid.png");
    shader.use();
    shader.setInt("planeTexture", 0);

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    model = glm::translate(model, glm::vec3(0.0, -1.0, 0.0));
    model = glm::rotate(model, -80.1f, glm::vec3(1.0, 0.0, 0.0));
    model = glm::scale(model, glm::vec3(10.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planeTexture);

    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    renderQuad();
}
unsigned int Scene::back_ground(unsigned int texture) {
    

    Shader equirectangularMapToCubeMapShader("E:/LearnOpenGL/Project1/Project1/shader/IBL/EquirectangularMaptoCubeMap.vs", "E:/LearnOpenGL/Project1/Project1/shader/IBL/EquirectangularMaptoCubeMap.fs");

    unsigned int captureFBO = CreateFrameBuffer(512, 512);
    unsigned int envCubeMap;
    glGenTextures(1, &envCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    equirectangularMapToCubeMapShader.use();
    equirectangularMapToCubeMapShader.setMat4("projection", captureProjection);
    equirectangularMapToCubeMapShader.setInt("equirectangularMap", 0);
    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D,texture);
    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    std::shared_ptr<Object3D> cube = make_shared<Cube>();
    for (GLuint i = 0; i < 6; i++) {
        equirectangularMapToCubeMapShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube->Draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return envCubeMap;
}
glm::vec2 Scene::GetFrameBufferSize(){
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    return glm::vec2(scrWidth, scrHeight);
}

bool Scene::creatWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    this->window = glfwCreateWindow(1280, 768, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//在调用任何OpenGL的函数之前我们需要初始化GLAD。
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    glfwSetMouseButtonCallback(window, mouse_clickCallback);
    glfwSetScrollCallback(window, scroll_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    // Setup Dear ImGui style

    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
        GLuint tex;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return (void*)tex;
    };
    ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
        GLuint texID = (GLuint)tex;
        glDeleteTextures(1, &texID);
    };

}
