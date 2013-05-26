////////////////////////////////////////////////////////////////////////////////
// Filename: vanillashaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURESHADERCLASS_H_
#define _TEXTURESHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <fstream>
#include <DirectXMath.h>
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
	    DirectX::XMFLOAT4X4 world;
	    DirectX::XMFLOAT4X4 view;
	    DirectX::XMFLOAT4X4 projection;
    };

    struct CameraBufferType
    {
        DirectX::XMFLOAT4 cameraPosition;
    };

    struct MaterialBufferType
    {
        DirectX::XMFLOAT4 ambientColor;
        DirectX::XMFLOAT4 diffuseColor;
        DirectX::XMFLOAT4 specularColor;
        float specularPower;
        bool useNormalMap;
        DirectX::XMFLOAT2 padding;
    };

    struct LightBufferType
    {
        DirectX::XMFLOAT3 lightDirection;
        float time;
        DirectX::XMFLOAT4 cameraPos;
        DirectX::XMFLOAT4 spotlightPos[NUM_SPOTLIGHTS];
        DirectX::XMFLOAT4 spotlightDir[NUM_SPOTLIGHTS];
        DirectX::XMFLOAT4 spotlightEtc[NUM_SPOTLIGHTS]; // arrays are packed into 4-float elements anyway so this is {Cos(angle), constant attenuation, linear attenuation, quadratic attenuation}
    };


public:
	VanillaShaderClass();
	VanillaShaderClass(const VanillaShaderClass&);
	~VanillaShaderClass();

	void Shutdown();
        bool Render(ID3D11DeviceContext *deviceContext, int indexCount, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, DirectX::CXMVECTOR cameraPos, ID3D11ShaderResourceView** normalMap, ID3D11ShaderResourceView** texture, unsigned resourceViewCount = 1, bool setSampler = true);
        bool InitializeShader(ID3D11Device*, HWND, wchar_t *vsFilename, char *vsFunctionName, wchar_t *psFilename, char *psFunctionName, bool multiStreaming = false);
        bool SetPSMaterial( ID3D11DeviceContext *deviceContext, DirectX::XMFLOAT4 &ambientColor, DirectX::XMFLOAT4 &diffuseColor, float specularPower, DirectX::XMFLOAT4 &specularColor, bool useNormalMap);
        bool SetPSLights( ID3D11DeviceContext *deviceContext, const DirectX::XMFLOAT3 &lightDirection, float time, DirectX::FXMVECTOR cameraPos, DirectX::XMFLOAT4 *spotlightPos, DirectX::XMFLOAT3 *spotlightDir, DirectX::XMFLOAT4 *spotlightParams, int numSpotlights );

private:
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, DirectX::CXMMATRIX, DirectX::CXMMATRIX, DirectX::CXMMATRIX, DirectX::CXMVECTOR, ID3D11ShaderResourceView **normalMap, ID3D11ShaderResourceView** textures, unsigned numViews = 1);
        void RenderShader(ID3D11DeviceContext *deviceContext, int indexCount, bool setSampler = true);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer *m_matrixBuffer, *m_cameraBuffer, *m_lightBuffer, *m_materialBuffer;
	ID3D11SamplerState* m_sampleState;
};

#endif