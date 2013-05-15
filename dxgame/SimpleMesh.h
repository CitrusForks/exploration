#ifndef SIMPLEMESH_H
#define SIMPLEMESH_H

#include <xnamath.h>
#include <vector>
#include <d3d11.h>
#include "vertex.h"
#include "textureclass.h"
#include "textureshaderclass.h"

class SimpleMesh
{
private:
	bool loaded;
        ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	TextureClass *m_Texture;
        TextureShaderClass *m_Shaders;


public:
	void render(CXMMATRIX model_to_world_space);
        bool load(wchar_t *objFileName, ID3D11Device* device, XMFLOAT2 texture_scaler = XMFLOAT2(1.0f, 1.0f));


	SimpleMesh();
	SimpleMesh(wchar_t *objFileName);
        SimpleMesh(wchar_t *objFileName, TextureClass *t, TextureShaderClass *ts) : m_Texture(t), m_Shaders(ts)
        {
            SimpleMesh(objFileName);
        }
	~SimpleMesh(void);
};

#endif