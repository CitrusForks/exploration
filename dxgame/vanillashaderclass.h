////////////////////////////////////////////////////////////////////////////////
// Filename: vanillashaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURESHADERCLASS_H_
#define _TEXTURESHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: TextureShaderClass
////////////////////////////////////////////////////////////////////////////////
class VanillaShaderClass
{
private:
    struct MatrixBufferType
    {
	    XMFLOAT4X4 world;
	    XMFLOAT4X4 view;
	    XMFLOAT4X4 projection;
    };

    struct CameraBufferType
    {
        XMFLOAT4 cameraPosition;
    };

    struct LightBufferType
    {
        XMFLOAT4 ambientColor;
        XMFLOAT4 diffuseColor;
        XMFLOAT4 specularColor;
        XMFLOAT3 lightDirection;
        float specularPower;
        float time;
        XMFLOAT3 cameraPos;
    };


public:
	VanillaShaderClass();
	VanillaShaderClass(const VanillaShaderClass&);
	~VanillaShaderClass();

	void Shutdown();
	bool Render(ID3D11DeviceContext*, int indexCount, CXMMATRIX worldMatrix, CXMMATRIX viewMatrix, CXMMATRIX projectionMatrix, ID3D11ShaderResourceView *texture, CXMVECTOR cameraPos);
        bool InitializeShader(ID3D11Device*, HWND, wchar_t *vsFilename, char *vsFunctionName, wchar_t *psFilename, char *psFunctionName, bool multiStreaming = false);
        bool SetPSConstants(ID3D11DeviceContext *deviceContext, XMFLOAT4 &ambientColor, XMFLOAT4 &diffuseColor, XMFLOAT3 &lightDirection, float specularPower, XMFLOAT4 &specularColor, float time, FXMVECTOR cameraPos);

private:
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, CXMMATRIX, CXMMATRIX, CXMMATRIX, CXMVECTOR, ID3D11ShaderResourceView*, unsigned numViews = 1);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer, *m_cameraBuffer, *m_lightBuffer;
	ID3D11SamplerState* m_sampleState;
};

#endif