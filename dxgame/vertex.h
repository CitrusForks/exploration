#ifndef VERTEX_H
#define VERTEX_H

#include <xnamath.h>
#include <string.h> // for memcmp()

struct Vertex
{ 
	XMFLOAT3 pos;    // vertex position in model space
	XMFLOAT3 normal; // yep, normal
	XMFLOAT2 tex0;   // texture sampler, possibly an array
	XMFLOAT2 tex1;   // not used, just experimenting
        unsigned int texArray;  // texture to choose from array if using an array of textures

        Vertex() : pos(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), tex0(0.0f, 0.0f), tex1(0.0f, 0.0f), texArray(0)
        {
        }

        // the following operator is meant only to allow the use of Vertex as a key to std::map() and such
	bool operator<(const Vertex that) const{
		return memcmp((void*)this, (void*)&that, sizeof(Vertex))>0;
	};
};


#endif
