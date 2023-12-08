#ifndef _HELP_H
#define _HELP_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<vector>
namespace rtx {
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    void renderQuad()
    {
        if (quadVAO == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
    struct Material {
        glm::vec3 emissive = glm::vec3(0, 0, 0); // 作为光源时的发光颜色
        glm::vec3 baseColor = glm::vec3(0, 0, 0);
        float subsurface = 0.0;
        float metallic = 0.0;
        float specular = 0.0;
        float specularTint = 0.0;
        float roughness = 0.0;
        float anisotropic = 0.0;
        float sheen = 0.0;
        float sheenTint = 0.0;
        float clearcoat = 0.0;
        float clearcoatGloss = 0.0;
        float IOR = 1.0;
        float transmission = 0.0;
    };
    // 三角形定义
    struct Triangle {
        glm::vec3 p1, p2, p3; // 顶点坐标
        glm::vec3 n1, n2, n3; // 顶点法线
        Material material; // 材质
    };
    struct Triangle_encoded {
        glm::vec3 p1, p2, p3; // 顶点坐标
        glm::vec3 n1, n2, n3; // 顶点法线
        glm::vec3 emissive; // 自发光参数
        glm::vec3 baseColor; // 颜色
        glm::vec3 param1; // (subsurface, metallic, specular)
        glm::vec3 param2; // (specularTint, roughness, anisotropic)
        glm::vec3 param3; // (sheen, sheenTint, clearcoat)
        glm::vec3 param4; // (clearcoatGloss, IOR, transmission)
    };
    void readObj(std::string filepath, std::vector<Triangle>& triangles, Material material, glm::mat4 trans, bool smoothNormal) {

        // 顶点位置，索引
        std::vector<glm::vec3> vertices;
        std::vector<GLuint> indices;

        // 打开文件流
        std::ifstream fin(filepath);
        std::string line;
        if (!fin.is_open()) {
            std::cout << "文件 " << filepath << " 打开失败" << std::endl;
            exit(-1);
        }

        // 计算 AABB 盒，归一化模型大小
        float maxx = -11451419.19;
        float maxy = -11451419.19;
        float maxz = -11451419.19;
        float minx = 11451419.19;
        float miny = 11451419.19;
        float minz = 11451419.19;

        // 按行读取
        while (std::getline(fin, line)) {
            std::istringstream sin(line);   // 以一行的数据作为 string stream 解析并且读取
            std::string type;
            GLfloat x, y, z;
            int v0, v1, v2;
            int vn0, vn1, vn2;
            int vt0, vt1, vt2;
            char slash;

            // 统计斜杆数目，用不同格式读取
            int slashCnt = 0;
            for (int i = 0; i < line.length(); i++) {
                if (line[i] == '/') slashCnt++;
            }

            // 读取obj文件
            sin >> type;
            if (type == "v") {
                sin >> x >> y >> z;
                vertices.push_back(glm::vec3(x, y, z));
                maxx = std::max(maxx, x); maxy = std::max(maxx, y); maxz = std::max(maxx, z);
                minx = std::min(minx, x); miny = std::min(minx, y); minz = std::min(minx, z);
            }
            if (type == "f") {
                if (slashCnt == 6) {
                    sin >> v0 >> slash >> vt0 >> slash >> vn0;
                    sin >> v1 >> slash >> vt1 >> slash >> vn1;
                    sin >> v2 >> slash >> vt2 >> slash >> vn2;
                }
                else if (slashCnt == 3) {
                    sin >> v0 >> slash >> vt0;
                    sin >> v1 >> slash >> vt1;
                    sin >> v2 >> slash >> vt2;
                }
                else {
                    sin >> v0 >> v1 >> v2;
                }
                indices.push_back(v0 - 1);
                indices.push_back(v1 - 1);
                indices.push_back(v2 - 1);
            }
        }

        // 模型大小归一化
        float lenx = maxx - minx;
        float leny = maxy - miny;
        float lenz = maxz - minz;
        float maxaxis = std::max(lenx, std::max(leny, lenz));
        for (auto& v : vertices) {
            v.x /= maxaxis;
            v.y /= maxaxis;
            v.z /= maxaxis;
        }

        // 通过矩阵进行坐标变换
        for (auto& v : vertices) {
            glm::vec4 vv = glm::vec4(v.x, v.y, v.z, 1);
            vv = trans * vv;
            v = glm::vec3(vv.x, vv.y, vv.z);
        }

        // 生成法线
        std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0, 0, 0));
        for (int i = 0; i < indices.size(); i += 3) {
            glm::vec3 p1 = vertices[indices[i]];
            glm::vec3 p2 = vertices[indices[i + 1]];
            glm::vec3 p3 = vertices[indices[i + 2]];
            glm::vec3 n = normalize(cross(p2 - p1, p3 - p1));
            normals[indices[i]] += n;
            normals[indices[i + 1]] += n;
            normals[indices[i + 2]] += n;
        }

        // 构建 Triangle 对象数组
        int offset = triangles.size();  // 增量更新
        triangles.resize(offset + indices.size() / 3);
        for (int i = 0; i < indices.size(); i += 3) {
            Triangle& t = triangles[offset + i / 3];
            // 传顶点属性
            t.p1 = vertices[indices[i]];
            t.p2 = vertices[indices[i + 1]];
            t.p3 = vertices[indices[i + 2]];
            if (!smoothNormal) {
                glm::vec3 n = normalize(cross(t.p2 - t.p1, t.p3 - t.p1));
                t.n1 = n; t.n2 = n; t.n3 = n;
            }
            else {
                t.n1 = normalize(normals[indices[i]]);
                t.n2 = normalize(normals[indices[i + 1]]);
                t.n3 = normalize(normals[indices[i + 2]]);
            }

            // 传材质
            t.material = material;
        }
    }
    unsigned int loadHDRTexture(char const* path)
    {

        stbi_set_flip_vertically_on_load(false);
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);
        if (data)
        {


            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load HDR at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
    
}
#endif // !_HELP_H