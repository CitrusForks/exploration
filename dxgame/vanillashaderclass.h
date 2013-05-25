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

#define NUM_SPOTLIGHTS 4

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

    struct MaterialBufferType
    {
        XMFLOAT4 ambientColor;
        XMFLOAT4 diffuseColor;
        XMFLOAT4 specularColor;
        float specularPower;
        bool useNormalMap;
        XMFLOAT2 padding;
    };

    struct LightBufferType
    {
        XMFLOAT3 lightDirection;
        float time;
        XMFLOAT4 cameraPos;
        XMFLOAT4 spotlightPos[NUM_SPOTLIGHTS];
        XMFLOAT4 spotlightDir[NUM_SPOTLIGHTS];
        XMFLOAT4 spotlightEtc[NUM_SPOTLIGHTS]; // arrays are packed into 4-float elements anyway so this is {Cos(angle), constant attenuation, linear attenuation, quadratic attenuation}
    };


public:
	VanillaShaderClass();
	VanillaShaderClass(const VanillaShaderClass&);
	~VanillaShaderClass();

	void Shutdown();
        bool Render(ID3D11DeviceContext *deviceContext, int indexCount, CXMMATRIX worldMatrix, CXMMATRIX viewMatrix, CXMMATRIX projectionMatrix, CXMVECTOR cameraPos, ID3D11ShaderResourceView** normalMap, ID3D11ShaderResourceView** texture, unsigned resourceViewCount = 1, bool setSampler = true);
        bool InitializeShader(ID3D11Device*, HWND, wchar_t *vsFilename, char *vsFunctionName, wchar_t *psFilename, char *psFunctionName, bool multiStreaming = false);
        bool SetPSMaterial( ID3D11DeviceContext *deviceContext, XMFLOAT4 &ambientColor, XMFLOAT4 &diffuseColor, float specularPower, XMFLOAT4 &specularColor, bool useNormalMap);
        bool SetPSLights( ID3D11DeviceContext *deviceContext, const XMFLOAT3 &lightDirection, float time, FXMVECTOR cameraPos, XMFLOAT4 *spotlightPos, XMFLOAT3 *spotlightDir, XMFLOAT4 *spotlightParams, int numSpotlights );

private:
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, CXMMATRIX, CXMMATRIX, CXMMATRIX, CXMVECTOR, ID3D11ShaderResourceView **normalMap, ID3D11ShaderResourceView** textures, unsigned numViews = 1);
        void RenderShader(ID3D11DeviceContext *deviceContext, int indexCount, bool setSampler = true);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer *m_matrixBuffer, *m_cameraBuffer, *m_lightBuffer, *m_materialBuffer;
	ID3D11SamplerState* m_sampleState;
};

#endif