////////////////////////////////////////////////////////////////////////////////
// Filename: vanillashaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef VANILLASHADERCLASS_H_
#define VANILLASHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <fstream>
#include <DirectXMath.h>
using namespace std;

#include "cpp_hlsl_defs.h"

#include "Light.h"

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
        DirectX::XMFLOAT4X4 lightViewProjection[NUM_SPOTLIGHTS+2];
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
        DirectX::XMFLOAT4 ambientLight; // for the whole scene
        DirectX::XMFLOAT4 diffuseLight;
        unsigned int numLights;
    };


public:
	VanillaShaderClass();
	VanillaShaderClass(const VanillaShaderClass&);
	~VanillaShaderClass();

	void Shutdown();
        bool Render(ID3D11DeviceContext *deviceContext, int indexCount, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, ID3D11ShaderResourceView** normalMap, ID3D11ShaderResourceView** specularMap,std::vector<Light> *lights, ID3D11ShaderResourceView** texture, unsigned resourceViewCount = 1, bool setSampler = true);
    	bool InitializeShader(ID3D11Device* device, HWND hwnd, wchar_t *vsFilename, wchar_t *psFilename, bool multiStreaming = false);
    	bool SetPSMaterial( ID3D11DeviceContext *deviceContext, DirectX::XMFLOAT4 &ambientColor, DirectX::XMFLOAT4 &diffuseColor, float specularPower, DirectX::XMFLOAT4 &specularColor, bool useNormalMap, bool useSpecularMap);
        bool SetPSLights( ID3D11DeviceContext *deviceContext, const DirectX::FXMVECTOR lightDirection, float time, const DirectX::FXMVECTOR cameraPos, std::vector<Light> &lights, const DirectX::XMFLOAT4 &ambientLight, const DirectX::XMFLOAT4 &diffuseLight );
        bool setVSCameraBuffer( ID3D11DeviceContext* deviceContext, DirectX::CXMVECTOR cameraPos, float time, unsigned effect );

private:
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

        bool SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, 
            ID3D11ShaderResourceView **normalMap, ID3D11ShaderResourceView **specularMap, std::vector<Light> *lights, ID3D11ShaderResourceView **texture, unsigned numViews = 1);


        void RenderShader(ID3D11DeviceContext *deviceContext, int indexCount, bool setSampler = true);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer *m_matrixBuffer, *m_cameraBuffer, *m_lightBuffer, *m_materialBuffer;
	ID3D11SamplerState* m_sampleState[3]; // [0]: anisotropic filtering for normal rendering, [1]: unfiltered for copying to screen, [2]: unused yet... something for shadows
};

#endif