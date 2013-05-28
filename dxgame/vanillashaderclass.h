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
        DirectX::XMFLOAT4X4 lightViewProjection[NUM_SPOTLIGHTS+1];
        unsigned int numLights; // number of lights casting shadows in scene; aka how much of above array is populated
        float padding[3];
    };

    struct CameraBufferType
    {
        DirectX::XMFLOAT4 cameraPosition;
        float time;
        unsigned effect;
        float padding[2];
    };

    struct MaterialBufferType
    {
        DirectX::XMFLOAT4 ambientColor;
        DirectX::XMFLOAT4 diffuseColor;
        DirectX::XMFLOAT4 specularColor;
        float specularPower;
        unsigned useNormalMap; /* bool */
		unsigned useSpecularMap; /* bool */
        float padding;
    };

    struct LightBufferType
    {
        DirectX::XMFLOAT3 lightDirection;
        float time;
        DirectX::XMFLOAT4 cameraPos;
        DirectX::XMFLOAT4 spotlightPos[NUM_SPOTLIGHTS];
        DirectX::XMFLOAT4 spotlightDir[NUM_SPOTLIGHTS];
        DirectX::XMFLOAT4 spotlightEtc[NUM_SPOTLIGHTS]; // arrays are packed into 4-float elements anyway so this is {Cos(angle), constant attenuation, linear attenuation, quadratic attenuation}
        unsigned int numLights;
    };


public:
	VanillaShaderClass();
	VanillaShaderClass(const VanillaShaderClass&);
	~VanillaShaderClass();

	void Shutdown();
	bool Render(ID3D11DeviceContext *deviceContext, int indexCount, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, ID3D11ShaderResourceView** normalMap, ID3D11ShaderResourceView** specularMap,DirectX::XMFLOAT4X4 *lightProjections, int numShadows, ID3D11ShaderResourceView** texture, unsigned resourceViewCount = 1, bool setSampler = true);
		bool InitializeShader(ID3D11Device* device, HWND hwnd, wchar_t *vsFilename, wchar_t *psFilename, bool multiStreaming = false);
		bool SetPSMaterial( ID3D11DeviceContext *deviceContext, DirectX::XMFLOAT4 &ambientColor, DirectX::XMFLOAT4 &diffuseColor, float specularPower, DirectX::XMFLOAT4 &specularColor, bool useNormalMap, bool useSpecularMap);
        bool SetPSLights( ID3D11DeviceContext *deviceContext, const DirectX::XMFLOAT3 &lightDirection, float time, DirectX::FXMVECTOR cameraPos, 
            DirectX::XMFLOAT4 *spotlightPos, DirectX::XMFLOAT3 *spotlightDir, DirectX::XMFLOAT4 *spotlightParams, int numSpotlights );
        bool setVSCameraBuffer( ID3D11DeviceContext* deviceContext, DirectX::CXMVECTOR cameraPos, float time, unsigned effect );

private:
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix,
		ID3D11ShaderResourceView **normalMap, ID3D11ShaderResourceView **specularMap, DirectX::XMFLOAT4X4* lightProjections, unsigned numLights, ID3D11ShaderResourceView **texture, unsigned numViews = 1)
		;


        void RenderShader(ID3D11DeviceContext *deviceContext, int indexCount, bool setSampler = true);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer *m_matrixBuffer, *m_cameraBuffer, *m_lightBuffer, *m_materialBuffer;
	ID3D11SamplerState* m_sampleState[3]; // [0]: anisotropic filtering for normal rendering, [1]: unfiltered for copying to screen, [2]: unused yet... something for shadows
};

#endif