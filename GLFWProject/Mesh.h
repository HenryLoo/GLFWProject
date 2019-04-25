#pragma once
#ifndef Mesh_H
#define Mesh_H

#include "Vertex.h"

#include <glad/glad.h>
#include <vector>

class Texture;
class Shader;

class Mesh
{
public:
	Mesh(const std::vector<Vertex> &vertices, 
		const std::vector<GLuint> &indices, const std::vector<Texture *> &textures);
	~Mesh();

	void render(Shader *shader);

	// Transformation setters.
	void setTranslation(glm::vec3 translation);
	void setScale(glm::vec3 scale);
	void setRotation(glm::vec3 rotation);

private:
	// Get the transformation matrix.
	glm::mat4 getTransform() const;

	// Hold all the vertices for this mesh.
	// These will be passed into a vertex buffer object.
	std::vector<Vertex> m_vertices;

	// Hold all the indices for this mesh.
	// These will be passed into an element buffer object.
	std::vector<GLuint> m_indices;

	// Hold pointers to all textures for this mesh.
	std::vector<Texture *> m_textures;

	// Hold the ids for this mesh's vertex array object,
	// vertex buffer object, and element buffer object.
	GLuint m_VAO, m_VBO, m_EBO;

	// Transformation values. Rotations are in degrees.
	glm::vec3 m_translation{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_scale{ 1.0f, 1.0f, 1.0f };
	glm::vec3 m_rotation{ 0.0f, 0.0f, 0.0f };
};

#endif