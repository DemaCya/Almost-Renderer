#pragma once
#include<memory>
#include"global_data.h"
#include"utility.h"
class LUT
{
public:
	LUT();
	~LUT();
	unsigned int GetLUT();

private:
	unsigned int textureID;
	unsigned int captureFBO;
	unsigned int captureRBO;
	shared_ptr<Shader> shader;
};

LUT::LUT()
{
	shader = std::make_shared<Shader>("E:/LearnOpenGL/Project1/Project1/shader/IBL/BRDFlut.vs", "E:/LearnOpenGL/Project1/Project1/shader/IBL/BRDFlut.fs");
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
	glViewport(0, 0, 512, 512);
	shader->use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

LUT::~LUT()
{
	glDeleteTextures(1, &textureID);
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);
}
unsigned int LUT::GetLUT() {
	return textureID;
}