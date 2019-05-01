#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <sstream>

Mesh::Mesh(const std::vector<Vertex> &vertices,
	const std::vector<GLuint> &indices, const std::vector<Texture *> &textures) :
	m_vertices(vertices), m_indices(indices), m_textures(textures)
{
	// Create the mesh's VAO, VBO, and EBO.
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	// Bind to the vertex array object. All subsequent VBO/EBO configurations
	// will be stored for this VAO.
	glBindVertexArray(m_VAO);

	// Copy vertices to buffer.
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), 
		&m_vertices[0], GL_STATIC_DRAW);

	// Copy indices to buffer.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint),
		&m_indices[0], GL_STATIC_DRAW);

	// Set position attribute.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

	// Set normal attribute.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
		(void *)offsetof(Vertex, normal));

	// Set texture coordinate attribute.
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
		(void *)offsetof(Vertex, texCoords));

	// Unbind the VAO.
	glBindVertexArray(0);
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
}

void Mesh::render(Shader *shader)
{
	for (unsigned int i = 0; i < m_textures.size(); i++)
	{
		// Bind to the appropriate texture unit.
		glActiveTexture(GL_TEXTURE0 + i);

		// TODO: Do other texture configurations here.
		// For now, GL_TEXTURE0 is used for diffuse map and 
		// GLTEXTURE1 is used for specular map.

		m_textures[i]->bind();
	}

	// Reset the active texture.
	glActiveTexture(GL_TEXTURE0);

	// TODO: fix below for instancing.

	// Set uniforms for the shader.
	//shader->setMat4("model", getModelMatrix());

	// Render the mesh.
	glBindVertexArray(m_VAO);
	/*glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), 
		GL_UNSIGNED_INT, 0);*/

	// Unbind the VAO.
	//glBindVertexArray(0);
}

void Mesh::setTranslation(glm::vec3 translation)
{
	m_translation = translation;
}
void Mesh::setScale(glm::vec3 scale)
{
	m_scale = scale;
}

void Mesh::setRotation(glm::vec3 rotation)
{
	m_rotation = rotation;
}

glm::mat4 Mesh::getModelMatrix() const
{
	// Get the translation matrix.
	glm::mat4 transMatrix = glm::translate(glm::mat4(1.0), m_translation);

	// Get the rotation matrix.
	// Use quaternions to avoid Gimbal lock.
	glm::quat quaternion = glm::quat(glm::radians(m_rotation));
	glm::mat4 rotMatrix = glm::toMat4(quaternion);

	// Get the scale matrix.
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0), m_scale);

	// Apply the transformations in order: scale > rotation > translation.
	glm::mat4 modelMatrix = transMatrix * rotMatrix * scaleMatrix * glm::mat4(1.0f);
	return modelMatrix;
}