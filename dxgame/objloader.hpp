#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include "vertex.h"

bool loadOBJ(
	const wchar_t * path, 
	std::vector<Vertex> & out_vertices
);

#endif