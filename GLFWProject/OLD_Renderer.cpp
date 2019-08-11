//#include "Renderer.h"
//#include "Mesh.h"
//#include "Camera.h"
//
//#include <glm/gtc/matrix_transform.hpp>
//
//#include <iostream>
//
//Renderer::Renderer()
//{
//	// Render as wireframe.
//	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//	// Enable blending for transparency in textures.
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	// Load the shaders.
//	m_defaultShader = std::make_unique<Shader>("default.vs", "default.fs");
//	m_screenShader = std::make_unique<Shader>("screen.vs", "screen.fs");
//
//	// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
//	float screenVertices[] = {
//		// positions   // texCoords
//		-1.0f,  1.0f,  0.0f, 1.0f,
//		-1.0f, -1.0f,  0.0f, 0.0f,
//		 1.0f, -1.0f,  1.0f, 0.0f,
//
//		-1.0f,  1.0f,  0.0f, 1.0f,
//		 1.0f, -1.0f,  1.0f, 0.0f,
//		 1.0f,  1.0f,  1.0f, 1.0f
//	};
//
//	glGenVertexArrays(1, &m_screenVAO);
//	glGenBuffers(1, &m_screenVBO);
//	glBindVertexArray(m_screenVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, m_screenVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
//	glBindVertexArray(0);
//
//	glGenBuffers(1, &m_instanceVBO);
//}
//
//Renderer::~Renderer()
//{
//	glDeleteBuffers(1, &m_screenVAO);
//	glDeleteBuffers(1, &m_screenVBO);
//	glDeleteFramebuffers(1, &m_screenFBO);
//	glDeleteTextures(1, &m_screenColourBuffer);
//	glDeleteRenderbuffers(1, &m_screenRBO);
//}
//
//void Renderer::render(Camera *camera, float aspectRatio)
//{
//	glBindFramebuffer(GL_FRAMEBUFFER, m_screenFBO);
//
//	// Clear the colour buffer.
//	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	// Enable depth testing.
//	glEnable(GL_DEPTH_TEST);
//
//	// Render each mesh.
//	m_defaultShader->use();
//
//	// Set the view matrix uniform.
//	glm::mat4 viewMatrix{ camera->getViewMatrix() };
//	m_defaultShader->setMat4("view", viewMatrix);
//
//	// Set projection matrix uniform.
//	glm::mat4 projMatrix{ glm::perspective(glm::radians(45.0f),
//		aspectRatio, 0.1f, 100.0f) };
//	m_defaultShader->setMat4("projection", projMatrix);
//	m_defaultShader->setVec3("viewPos", camera->getPosition());
//
//	// TODO: replace placeholder lighting uniforms later.
//
//	// Light properties.
//	m_defaultShader->setVec3("dirLight.direction", 0.f, -1.f, -0.5f);
//	m_defaultShader->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
//	m_defaultShader->setVec3("dirLight.diffuse", 1.f, 0.2f, 0.2f);
//	m_defaultShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
//
//	glm::vec3 lightColour{ glm::vec3(0.2f, 0.2f, 1.0f) };
//	glm::vec3 diffuseColour{ lightColour * glm::vec3(0.8f) };
//	glm::vec3 ambientColour{ diffuseColour * glm::vec3(0.6f) };
//	m_defaultShader->setVec3("pointLight.position", camera->getPosition());
//	m_defaultShader->setVec3("pointLight.ambient", ambientColour);
//	m_defaultShader->setVec3("pointLight.diffuse", diffuseColour);
//	m_defaultShader->setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
//	m_defaultShader->setFloat("pointLight.constant", 1.0f);
//	m_defaultShader->setFloat("pointLight.linear", 0.045f);
//	m_defaultShader->setFloat("pointLight.quadratic", 0.0075f);
//
//	// Material properties.
//	m_defaultShader->setInt("material.diffuse", 0);
//	m_defaultShader->setInt("material.specular", 1);
//	m_defaultShader->setFloat("material.shininess", 32.0f);
//
//	//// TODO: clean up instancing implementation.
//	//m_meshes[0]->render(m_defaultShader.get());
//	//glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
//	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * models.size(), &models[0], GL_DYNAMIC_DRAW);
//
//	//glEnableVertexAttribArray(3);
//	//glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
//	//glEnableVertexAttribArray(4);
//	//glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
//	//glEnableVertexAttribArray(5);
//	//glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
//	//glEnableVertexAttribArray(6);
//	//glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
//
//	//glVertexAttribDivisor(3, 1);
//	//glVertexAttribDivisor(4, 1);
//	//glVertexAttribDivisor(5, 1);
//	//glVertexAttribDivisor(6, 1);
//
//	//glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_meshes.size());
//	//glBindVertexArray(0);
//	//// End of instancing implementation.
//
//	// Swap framebuffers for post-processing effects.
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	m_screenShader->use();
//	glBindVertexArray(m_screenVAO);
//	glDisable(GL_DEPTH_TEST);
//	glBindTexture(GL_TEXTURE_2D, m_screenColourBuffer);
//	glDrawArrays(GL_TRIANGLES, 0, 6);
//}
//
//void Renderer::addMesh(Mesh *mesh)
//{
//	// Store the mesh to render later.
//	m_meshes.push_back(mesh);
//}
//
//void Renderer::clearMeshes()
//{
//	m_meshes.clear();
//}
//
//void Renderer::createFramebuffer(int screenWidth, int screenHeight)
//{
//	// Delete any existing old buffers.
//	if (m_screenFBO) glDeleteFramebuffers(1, &m_screenFBO);
//	if (m_screenColourBuffer) glDeleteTextures(1, &m_screenColourBuffer);
//	if (m_screenRBO) glDeleteRenderbuffers(1, &m_screenRBO);
//
//	// Create the frame buffer.
//	glGenFramebuffers(1, &m_screenFBO);
//	glBindFramebuffer(GL_FRAMEBUFFER, m_screenFBO);
//
//	// Create the texture buffer for rendering the scene on a quad.
//	glGenTextures(1, &m_screenColourBuffer);
//	glBindTexture(GL_TEXTURE_2D, m_screenColourBuffer);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_screenColourBuffer, 0);
//
//	// Create the render buffer for depth and stencil testing.
//	glGenRenderbuffers(1, &m_screenRBO);
//	glBindRenderbuffer(GL_RENDERBUFFER, m_screenRBO);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_screenRBO);
//
//	// Check if the framebuffer is complete.
//	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//	{
//		std::cout << "Framebuffer not complete." << std::endl;
//	}
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}