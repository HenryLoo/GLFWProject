#pragma once
#ifndef Vertex_H
#define Vertex_H

#include <glm/glm.hpp>

// Defines the structure of a mesh vertex.
// Each vertex has 8 values.
struct Vertex
{
	// The position of this vertex.
	// (x, y, z)
	glm::vec3 position;

	// The normal vector at this vertex.
	// (x, y, z)
	glm::vec3 normal;

	// The texture coordinates of this vertex.
	// (u, v)
	glm::vec2 texCoords;
};

#endif