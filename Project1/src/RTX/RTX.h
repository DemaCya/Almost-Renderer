#ifndef _RTX_H
#define _RTX_H

#include"shader.h"
#include"BVH.h"
#include"global_data.h"

class RayTracing
{
public:
	RayTracing(Shader shaderPass1, Shader pass1_5,Shader shaderPass2);
	~RayTracing();
	void render(unsigned int frameBuffer);
	void Init();
private:
	Shader shaderPass1;
	Shader shaderPass1_5;
	Shader shaderPass2;
	int nTriangles;
	int nnodes;
	GLuint trianglesTextureBuffer;
	GLuint bvhTextureBuffer;
	int frameCounter = 0;
	GLuint last_Frame;
	GLuint gBuffer;
	GLuint currentFrame;
	GLuint lfFBO;
	unsigned int hdrMap;
	
	
};
RayTracing::RayTracing(Shader shaderPass1, Shader shaderPass1_5, Shader shaderPass2) :shaderPass1(shaderPass1), shaderPass2(shaderPass2),shaderPass1_5(shaderPass1_5)
{

}

RayTracing::~RayTracing()
{
}
void RayTracing::Init() {
	std::vector<rtx::Triangle> triangles;
	
	rtx::Material m;
	m.baseColor = glm::vec3(0.5, 0.8, 0.5);
	m.roughness = 0.15;
	m.subsurface = 1.0;
	m.specular = 1.0;
	m.clearcoat = 1.0;
	m.anisotropic = 0.0;
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1.2*0.5, 2.0*0.5, 1.0));
	model = glm::translate(model, glm::vec3(0.0, -0.85, 0));
	rtx::readObj("E:/LearnOpenGL/Project1/Project1/models/Stanford Bunny.obj", triangles, m, model, true);

	//底部
	
	m.baseColor = glm::vec3(0.6, 0.6, 0.6);
	m.roughness = 0.1;
	m.subsurface = 0.5;
	m.specular = 1.0;
	m.clearcoat = 0.0;
	m.anisotropic = 0.0;
	m.metallic = 0.8;
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1.2*1.2, 0.01, 2.0));
	model = glm::translate(model, glm::vec3(0.0, -15.0*5, 0.0));
	rtx::readObj("E:/LearnOpenGL/Project1/Project1/models/quad.obj", triangles, m, model, false);

	//左边
	/*m.baseColor = glm::vec3(0.9, 0.5, 0.5);
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.01,2.0, 2.0));
	model = glm::translate(model, glm::vec3(-72.0, 0.123, 0.0));
	rtx::readObj("E:/LearnOpenGL/Project1/Project1/models/quad.obj", triangles, m, model, false);*/
	//
	////右边
	/*m.baseColor = glm::vec3(0.5, 0.9, 0.5);
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.01, 2.0, 2.0));
	model = glm::translate(model, glm::vec3(72.0, 0.123, 0.0));
	rtx::readObj("E:/LearnOpenGL/Project1/Project1/models/quad.obj", triangles, m, model, false);*/

	//上边
	/*m.baseColor = glm::vec3(0.6, 0.6, 0.6);
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1.2 * 1.2, 0.01, 2.0));
	model = glm::translate(model, glm::vec3(0.0, 125, 0.0));
	rtx::readObj("E:/LearnOpenGL/Project1/Project1/models/quad.obj", triangles, m, model, false);*/

	//后面
	//m.baseColor = glm::vec3(0.6, 0.6, 0.6);
	////m.emissive = glm::vec3(0.7, 0.7, 0);
	//model = glm::mat4(1.0f);
	//model = glm::scale(model, glm::vec3(1.2 * 1.2, 2.5, 0.01));
	//model = glm::translate(model, glm::vec3(0.0, 0.2, -90.0));
	//rtx::readObj("E:/LearnOpenGL/Project1/Project1/models/quad.obj", triangles, m, model, false);

	//灯
	m.baseColor = glm::vec3(1, 1, 1);
	m.emissive = glm::vec3(20, 20, 20);
	model = glm::mat4(1.0f);
	/*model = glm::scale(model, glm::vec3(1.0*0.6, 1.0, 0.6));
	model = glm::translate(model, glm::vec3(0.0, 1.23, -0.0));
	rtx::readObj("E:/LearnOpenGL/Project1/Project1/models/plane.obj", triangles, m, model, false);*/
	model = glm::scale(model, glm::vec3(1.0 * 0.6, 0.01, 0.6));
	model = glm::translate(model, glm::vec3(0.0, 123, -0.0));
	rtx::readObj("E:/LearnOpenGL/Project1/Project1/models/quad.obj", triangles, m, model, false);

	nTriangles = triangles.size();

	BVHNode testNode;
	
	std::vector<BVHNode> nodes;
	buildBVH(triangles, nodes, 0, triangles.size() - 1, 8);
	nnodes = nodes.size();

	std::cout << "BVH 建立完成: 共 " << nnodes << " 个节点" << std::endl;
	std::vector<rtx::Triangle_encoded> triangle_encoded(nTriangles);
	for (int i = 0; i < nTriangles; i++) {
		rtx::Triangle t = triangles[i];
		rtx::Material m = t.material;

		//顶点位置
		triangle_encoded[i].p1 = t.p1;
		triangle_encoded[i].p2 = t.p2;
		triangle_encoded[i].p3 = t.p3;

		//顶点法线
		triangle_encoded[i].n1 = t.n1;
		triangle_encoded[i].n2 = t.n2;
		triangle_encoded[i].n3 = t.n3;

		//材质
		triangle_encoded[i].baseColor = m.baseColor;
		triangle_encoded[i].emissive = m.emissive;
		triangle_encoded[i].param1 = glm::vec3(m.subsurface, m.metallic, m.specular);//(subsurface, metallic, specular)
		triangle_encoded[i].param2 = glm::vec3(m.specularTint, m.roughness, m.anisotropic);//(specularTint, roughness, anisotropic)
		triangle_encoded[i].param3 = glm::vec3(m.sheen, m.sheenTint, m.clearcoat);//(sheen, sheenTint, clearcoat)
		triangle_encoded[i].param4 = glm::vec3(m.clearcoatGloss, m.IOR, m.transmission);//(clearcoatGloss, IOR, transmission)

	}

	GLuint tbo;
	
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, triangle_encoded.size() * sizeof(rtx::Triangle_encoded), &triangle_encoded[0], GL_STATIC_DRAW);
	glGenTextures(1, &trianglesTextureBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, trianglesTextureBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);

	

	vector<BVHNode_encoded> bvhnode_encoded(nnodes);
	for (int i = 0; i < nnodes; i++) {
		bvhnode_encoded[i].childs = glm::vec3(nodes[i].left, nodes[i].right, 0);
		bvhnode_encoded[i].leafInfo = glm::vec3(nodes[i].n, nodes[i].index, 0);
		bvhnode_encoded[i].AA = nodes[i].AA;
		bvhnode_encoded[i].BB = nodes[i].BB;
	}
	GLuint tbo1;
	
	glGenBuffers(1, &tbo1);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo1);
	glBufferData(GL_TEXTURE_BUFFER, bvhnode_encoded.size() * sizeof(BVHNode_encoded), &bvhnode_encoded[0], GL_STATIC_DRAW);
	glGenTextures(1, &bvhTextureBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, bvhTextureBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo1);
	std::cout << "              三角形节点数" << nTriangles<<std::endl;

	hdrMap = rtx::loadHDRTexture("E:/LearnOpenGL/Project1/Project1/Texture/pbrTexture/peppermint_powerplant_4k.hdr");


	//last_Frame = loadTexture("E:/LearnOpenGL/Project1/Project1/Texture/wood.png");


	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	
	glGenTextures(1, &currentFrame);
	glBindTexture(GL_TEXTURE_2D, currentFrame);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentFrame, 0);

	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// - Finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete! for currentFrame" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	glGenFramebuffers(1, &lfFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, lfFBO);
	
	glGenTextures(1, &last_Frame);
	glBindTexture(GL_TEXTURE_2D, last_Frame);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, last_Frame, 0);
	GLuint attachment[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachment);

	GLuint rboDepth2;
	glGenRenderbuffers(1, &rboDepth2);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth2);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete! for lastFrame" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}
void RayTracing::render(unsigned int imGUIframeBuffer) {

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderPass1.use();
	shaderPass1.setInt("triangles", 0);
	shaderPass1.setInt("nodes", 1);
	shaderPass1.setInt("lastFrame", 2);
	shaderPass1.setInt("hdrMap", 3);
	shaderPass1.setInt("nTriangles", nTriangles);
	shaderPass1.setInt("nNodes", nnodes);
	shaderPass1.setInt("width", 1280);
	shaderPass1.setInt("height", 768);
	shaderPass1.setInt("frameCounter", frameCounter++);
	std::cout << frameCounter << std::endl;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, trianglesTextureBuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, bvhTextureBuffer);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, last_Frame);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, hdrMap);
	rtx::renderQuad();

	
	//last_Frame = currentFrame;
	glBindFramebuffer(GL_FRAMEBUFFER, lfFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderPass1_5.use();
	shaderPass1_5.setInt("frame", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentFrame);
	rtx::renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER,imGUIframeBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderPass2.use();
	shaderPass2.setInt("frame",0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, last_Frame);
	rtx::renderQuad();
	
	
}
#endif // !_RTX_H