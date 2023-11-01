#pragma once
#include<glm/glm.hpp>
struct BoneInfo {
	int id;

	glm::mat4 offset;//把骨骼从模型空间转到骨骼空间
};