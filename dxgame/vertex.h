#ifndef VERTEX_H
#define VERTEX_H

#include <xnamath.h>
#include <string.h> // for memcmp()

struct Vertex
{ 
	XMFLOAT3 pos; 
	XMFLOAT3 normal; 
	XMFLOAT2 tex0; 
	XMFLOAT2 tex1; 

        Vertex() : pos(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), tex0(0.0f, 0.0f), tex1(0.0f, 0.0f)
        {
        }

        // the following operator is meant only to allow the use of Vertex as a key to std::map() and such
	bool operator<(const Vertex that) const{
		return memcmp((void*)this, (void*)&that, sizeof(Vertex))>0;
	};
};


#endif
