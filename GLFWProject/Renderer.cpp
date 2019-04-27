#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Renderer::Renderer()
{
	// Render as wireframe.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Enable blending for transparency in textures.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Load the shader.
	m_defaultShader = std::make_unique<Shader>("default.vs", "default.fs");
}

Renderer::~Renderer()
{

}

void Renderer::render(Camera *camera, float aspectRatio)
{
	// Clear the colour buffer.
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Render each mesh.
	m_defaultShader->use();

	// Set the view matrix uniform.
	glm::mat4 viewMatrix = camera->getViewMatrix();
	m_defaultShader->setMat4("view", viewMatrix);

	// Set projection matrix uniform.
	glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f),
		aspectRatio, 0.1f, 100.0f);
	m_defaultShader->setMat4("projection", projMatrix);
	m_defaultShader->setVec3("viewPos", camera->getPosition());

	// TODO: replace placeholder lighting uniforms later.

	// Light properties.
	m_defaultShader->setVec3("dirLight.direction", 0.f, -1.f, -0.5f);
	m_defaultShader->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	m_defaultShader->setVec3("dirLight.diffuse", 1.f, 0.2f, 0.2f);
	m_defaultShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

	glm::vec3 lightColour = glm::vec3(0.2f, 0.2f, 1.0f);
	glm::vec3 diffuseColour = lightColour * glm::vec3(0.8f);
	glm::vec3 ambientColour = diffuseColour * glm::vec3(0.6f);
	m_defaultShader->setVec3("pointLight.position", camera->getPosition());
	m_defaultShader->setVec3("pointLight.ambient", ambientColour);
	m_defaultShader->setVec3("pointLight.diffuse", diffuseColour);
	m_defaultShader->setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
	m_defaultShader->setFloat("pointLight.constant", 1.0f);
	m_defaultShader->setFloat("pointLight.linear", 0.045f);
	m_defaultShader->setFloat("pointLight.quadratic", 0.0075f);

	// Material properties.
	m_defaultShader->setInt("material.diffuse", 0);
	m_defaultShader->setInt("material.specular", 1);
	m_defaultShader->setFloat("material.shininess", 32.0f);

	for (Mesh *mesh : m_meshes)
	{
		mesh->render(m_defaultShader.get());
	}
}

void Renderer::addMesh(Mesh *mesh)
{
	// Store the mesh to render later.
	m_meshes.push_back(mesh);
}

void Renderer::clearMeshes()
{
	m_meshes.clear();
}