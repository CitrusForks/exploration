#ifndef VBOINDEXER_HPP
#define VBOINDEXER_HPP

#include <xnamath.h>
#include "vertex.h"

void indexVBO(
	std::vector<Vertex> & in_vertices,

	std::vector<unsigned short> & out_indices,
	std::vector<Vertex> & out_vertices);

#if 0
void indexVBO_TBN(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,
	std::vector<glm::vec3> & in_tangents,
	std::vector<glm::vec3> & in_bitangents,

	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	std::vector<glm::vec3> & out_tangents,
	std::vector<glm::vec3> & out_bitangents
);
#endif

#endif