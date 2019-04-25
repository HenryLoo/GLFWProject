#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

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

		m_textures[i]->bind();
	}

	// Reset the active texture.
	glActiveTexture(GL_TEXTURE0);

	// Render the mesh.
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), 
		GL_UNSIGNED_INT, 0);

	// Unbind the VAO.
	glBindVertexArray(0);
}

const std::vector<Vertex> &Mesh::getVertices() const
{
	return m_vertices;
}

const std::vector<GLuint> &Mesh::getIndices() const
{
	return m_indices;
}

const std::vector<Texture *> &Mesh::getTextures() const
{
	return m_textures;
}