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
#include"Scene.h"
#include"utility.h"
#include<Animation.h>
#include<Animator.h>
#include"global_data.h"
#include"Window/Window.h"
#include"Window/TopWindow.h"
#include"Window/LeftWindow.h"
#include"Window/RenderWindow.h"
#include"Window/BottomWindow.h"
#include"ImFileDialog.h"
#include"global_data.h"
#include"model_s.h"
#include"LTC/ltc_matrix.h"
#include"PBR/Irradiance.h"
#include"PBR/prefilter.h"
#include"PBR/LUT.h"
#include"RTX/RTX.h"
GLfloat lerp(GLfloat a, GLfloat b, GLfloat f);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadHDRTexture(char const* path);
void renderCube();
void renderSphere(std::shared_ptr<Object3D> obj);
GLuint loadMTexture();
GLuint loadLUTTexture();


int main()
{
    Scene scene;
    scene.creatWindow();
    

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    Shader pbrShader("E:/LearnOpenGL/Project1/Project1/shader/IBL/iblPBR.vs", "E:/LearnOpenGL/Project1/Project1/shader/IBL/finalPBRshader.fs");
    Shader equirectangularMaptoCubeMapShader("E:/LearnOpenGL/Project1/Project1/shader/IBL/EquirectangularMaptoCubeMap.vs", "E:/LearnOpenGL/Project1/Project1/shader/IBL/EquirectangularMaptoCubeMap.fs");
    Shader skyboxShader("E:/LearnOpenGL/Project1/Project1/shader/skybox/skybox.vs","E:/LearnOpenGL/Project1/Project1/shader/skybox/skybox.fs");
    Shader ambientIBLShader("E:/LearnOpenGL/Project1/Project1/shader/IBL/ambientIBL.vs", "E:/LearnOpenGL/Project1/Project1/shader/IBL/ambientIBL.fs");
    Shader prefilterShader("E:/LearnOpenGL/Project1/Project1/shader/IBL/prefilterMap.vs", "E:/LearnOpenGL/Project1/Project1/shader/IBL/prefilterMap.fs");
    Shader brdfShader("E:/LearnOpenGL/Project1/Project1/shader/IBL/BRDFlut.vs", "E:/LearnOpenGL/Project1/Project1/shader/IBL/BRDFlut.fs");
    Shader animationShader("E:/LearnOpenGL/Project1/Project1/shader/Animation/anim_model.vs", "E:/LearnOpenGL/Project1/Project1/shader/Animation/anim_model.fs");
    Shader planeShader("E:/LearnOpenGL/Project1/Project1/shader/plane/plane.vs", "E:/LearnOpenGL/Project1/Project1/shader/plane/plane.fs");
    Shader ltcShader("E:/LearnOpenGL/Project1/Project1/shader/LTC/LTC.vs","E:/LearnOpenGL/Project1/Project1/shader/LTC/LTC.fs");
    Shader light_plane_Shader("E:/LearnOpenGL/Project1/Project1/shader/LTC/light_plane.vs","E:/LearnOpenGL/Project1/Project1/shader/LTC/light_plane.fs");
    Model ourModel("E:/LearnOpenGL/Project1/Project1/vampire/dancing_vampire.dae");
    Animation danceAnimation("E:/LearnOpenGL/Project1/Project1/vampire/dancing_vampire.dae", &ourModel);
    Animator animator(&danceAnimation);
    Shader rtxShaderPass1("E:/LearnOpenGL/Project1/Project1/shader/RTX/RayTracing.vs", "E:/LearnOpenGL/Project1/Project1/shader/RTX/pass1.fs");
    Shader rtxShaderPass2("E:/LearnOpenGL/Project1/Project1/shader/RTX/RayTracing.vs", "E:/LearnOpenGL/Project1/Project1/shader/RTX/pass2.fs");
    Shader rtxShaderPass1_5("E:/LearnOpenGL/Project1/Project1/shader/RTX/RayTracing.vs", "E:/LearnOpenGL/Project1/Project1/shader/RTX/pass1_5.fs");
    pbrShader.use();
    pbrShader.setInt("albedoMap", 0);
    pbrShader.setInt("normalMap", 1);
    pbrShader.setInt("metallicMap", 2);
    pbrShader.setInt("roughnessMap", 3);
    pbrShader.setInt("irradianceMap", 4);
    pbrShader.setInt("prefilterMap", 5);
    pbrShader.setInt("brdfLUT", 6);
    pbrShader.setInt("aoMap", 7);

    
    skyboxShader.use();
    skyboxShader.setInt("environmentMap", 0);
  
    glViewport(0, 0, screenWidth, screenHeight);

    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3(10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };
    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 2.5;

    
    auto hdrTexture = loadHDRTexture("E:/LearnOpenGL/Project1/Project1/Texture/pbrTexture/newport_loft.hdr");
    auto concreteTexture = loadHDRTexture("E:/LearnOpenGL/Project1/Project1/Texture/concreteTexture.png");
    
    unsigned int captureFBO = CreateFrameBuffer(512, 512);
    unsigned int captureRBO;
    glGenRenderbuffers(1, &captureRBO);
    
    unsigned int envCubemap = scene.back_ground(hdrTexture);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);




    ambientIBLShader.use();
    ambientIBLShader.setMat4("projection", captureProjection);
    ambientIBLShader.setInt("environmentMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; i++) {
        ambientIBLShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    std::shared_ptr<Irradiance> irradiance = make_shared<Irradiance>(envCubemap);
    std::shared_ptr<Prefilter> prefilter = std::make_shared<Prefilter>(envCubemap);
    std::shared_ptr<LUT> lut = std::make_shared<LUT>();

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; i++) {
            prefilterShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    unsigned int imGUIFBO;
    glGenFramebuffers(1, &imGUIFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, imGUIFBO);
    
    
    glGenTextures(1, &imGUItexture);
    glBindTexture(GL_TEXTURE_2D, imGUItexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, imGUItexture, 0);
    
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glm::vec3 LIGHT_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 areaLightTranslate;
    areaLightTranslate = glm::vec3(0.0f, 0.0f, 0.0f);

    ltcShader.use();
    ltcShader.setVec3("areaLight.points[0]", areaLightVertices[0].position);
    ltcShader.setVec3("areaLight.points[1]", areaLightVertices[1].position);
    ltcShader.setVec3("areaLight.points[2]", areaLightVertices[4].position);
    ltcShader.setVec3("areaLight.points[3]", areaLightVertices[5].position);
    ltcShader.setVec3("areaLight.color", LIGHT_COLOR);
    ltcShader.setInt("LTC1", 0);
    ltcShader.setInt("LTC2", 1);
    ltcShader.setInt("material.diffuse", 2);
    ltcShader.setVec4("material.albedoRoughness",glm::vec4(0.4f,0.4f,0.4f,0.5f));
    ltcShader.setFloat("areaLight.intensity", 4.0f);
    ltcShader.setBool("areaLight.twoSided", true);

    glm::mat4 planeModel(1.0f);
    light_plane_Shader.use();
    light_plane_Shader.setMat4("model", planeModel);
    light_plane_Shader.setVec3("lightColor", LIGHT_COLOR);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    pbrShader.use();
    pbrShader.setMat4("projection", projection);
    
    unsigned int m1 = loadMTexture();
    unsigned int m2 = loadLUTTexture();
    
    glm::vec2 widthAndheight  = scene.GetFrameBufferSize();
    glViewport(0, 0, widthAndheight.x, widthAndheight.y);

    std::shared_ptr<Cube> _cube = std::make_shared<Cube>();
    std::shared_ptr<Sphere> _sphere = std::make_shared<Sphere>();
    std::shared_ptr<Object3D> object;
    std::shared_ptr<TopWindow> TWindow = std::make_shared<TopWindow>();
    std::shared_ptr<LeftWindow> LWindow = std::make_shared<LeftWindow>();
    std::shared_ptr<RenderWindow> renderWindow = std::make_shared<RenderWindow>();
    std::shared_ptr<BottomWindow> BWindow = std::make_shared<BottomWindow>();
    std::shared_ptr<RayTracing> rtx = std::make_shared<RayTracing>(rtxShaderPass1,rtxShaderPass1_5,rtxShaderPass2);
    rtx->Init();
    //让他一直循环渲染
    while (!glfwWindowShouldClose(scene.window))//判断是否关闭，如果关闭了会返回true的
    {
        glfwPollEvents();//检查有没有触发什么事件,检测完才能进行下一步。
        

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        glPolygonMode(GL_FRONT_AND_BACK, renderMode);
        processInput(scene.window);
        animator.UpdateAnimation(deltaTime);
        //渲染指令

        if (object_index == sphere) {
            object = _sphere;
        }
        if (object_index == cube) {
            object = _cube;
        }
 

        glBindFramebuffer(GL_FRAMEBUFFER, imGUIFBO);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        pbrShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        if (scene_index == 0) {
            pbrShader.use();
        
        pbrShader.setMat4("view", view);
        pbrShader.setVec3("camPos", camera.Position);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Albedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Metallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, Roughness);
        glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance->GetIrradiance());
        glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter->Getprefilter());
        glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, lut->GetLUT());
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, AO);
            
        pbrShader.setBool("use_skybox", use_skybox);
        pbrShader.setBool("use_metallicMap", use_metallicMap);
        pbrShader.setBool("use_roughnessMap", use_roughnessMap);
        pbrShader.setBool("use_normalMap", use_normalMap);
        pbrShader.setBool("use_aoMap", use_aoMap);
        pbrShader.setBool("use_albedoMap", use_albedoMap);
        pbrShader.setVec3("_albedo", _albedo);
        pbrShader.setFloat("_metallic", _metallic);
            pbrShader.setFloat("_roughness", _roughness);
        pbrShader.setMat4("model", model);
        pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        object->Draw();
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        }
        }
        if (scene_index == 1) {
            ltcShader.use();
            ltcShader.setVec3("areaLight.points[0]", areaLightVertices[0].position);
            ltcShader.setVec3("areaLight.points[1]", areaLightVertices[1].position);
            ltcShader.setVec3("areaLight.points[2]", areaLightVertices[4].position);
            ltcShader.setVec3("areaLight.points[3]", areaLightVertices[5].position);
            ltcShader.setVec3("areaLight.color", LIGHT_COLOR);
            ltcShader.setInt("LTC1", 0);
            ltcShader.setInt("LTC2", 1);
            ltcShader.setInt("material.diffuse", 2);
            ltcShader.setVec4("material.albedoRoughness", glm::vec4(0.4f, 0.4f, 0.4f, 0.5f));
            ltcShader.setFloat("areaLight.intensity", 4.0f);
            ltcShader.setBool("areaLight.twoSided", true);
            glm::mat3 normalMatrix = glm::mat3(model);
            ltcShader.setMat4("model", model);
            ltcShader.setMat3("normalMatrix", normalMatrix);
            glm::mat4 view = camera.GetViewMatrix();
            ltcShader.setMat4("view", view);
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
            ltcShader.setMat4("projection", projection);
            ltcShader.setVec3("viewPosition", camera.Position);
            ltcShader.setVec3("areaLightTranslate", areaLightTranslate);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, concreteTexture);
            renderLTCplane();
            glm::mat4 planeModel(1.0f);
            light_plane_Shader.use();
            light_plane_Shader.setMat4("model", planeModel);
            light_plane_Shader.setVec3("lightColor", LIGHT_COLOR);
            light_plane_Shader.setMat4("view", view);
            light_plane_Shader.setMat4("projection", projection);
            renderAreaLight();
        }
        if(scene_index == 2) {
            rtx->render(imGUIFBO);
        }

        if (use_plane) {
            planeShader.use();
            scene.renderplane(planeShader);
        }
        if (use_skybox) {
            skyboxShader.use();
            skyboxShader.setMat4("projection", projection);
            skyboxShader.setMat4("view", view);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
            renderCube();
        }
      
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
      

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();
        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("This is some useful text.");
        ImGui::Checkbox("Style Editor", &show_app_style_editor);
        ImGui::Text("io.WantCaptureMouse: %d", io.WantCaptureMouse);
        ImGui::Text("io.WantCaptureMouseUnlessPopupClose: %d", io.WantCaptureMouseUnlessPopupClose);
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        
        renderWindow->render();
        LWindow->render();
        TWindow->render();
        BWindow->render();

        if (show_app_style_editor)
        {
            ImGui::Begin("Dear ImGui Style Editor", &show_app_style_editor);
            ImGui::Text("This is some useful text.");

            ImGui::ShowStyleEditor();
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(scene.window);//交换颜色缓冲

    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;

}

unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere(std::shared_ptr<Object3D> obj)
{
    
    obj->Draw();
}




void processInput(GLFWwindow* window)
{
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}
GLfloat lerp(GLfloat a, GLfloat b, GLfloat f) {
    return a + f * (b - a);
}
GLuint loadMTexture()
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
        0, GL_RGBA, GL_FLOAT, LTC1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

GLuint loadLUTTexture()
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
        0, GL_RGBA, GL_FLOAT, LTC2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}