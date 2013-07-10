#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_


// from http://www.rastertek.com/dx11tut03.html with lots of modifications, watch out

// I actually like this way of specifying input libraries but I can understand if most other people don't. 
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")


#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <directxmath.h>


class D3DClass
{
public:
	D3DClass();
	D3DClass(const D3DClass&);
	~D3DClass();

	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, 
						  float screenDepth, float screenNear);
	
	void Shutdown();
	
        void BeginScene(bool clear = true, float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f); // called before rendering
	void EndScene(); // ends rendering, stuff ends up on the screen

	ID3D11Device* GetDevice(); // needed in many places 
        ID3D11DeviceContext* GetDeviceContext(); // ""

        void GetVideoCardInfo(std::string, int&); // not really used...

        ID3D11DepthStencilView *GetDepthStencilView(); // used by off-screen renderer
        void setAsRenderTarget(bool depthEnable); // sets swap-chain render target as current

        void depthOn();  // useful when rendering a scene...
        void depthOff(); // this is for rendering post-processed image or overlays such as text

        void setDepthBias(bool setBias, bool highBias = false); // true for rendering to shadow maps

        HWND getWindow() { return m_window; }

private:
	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];
	IDXGISwapChain *m_swapChain;
	ID3D11Device *m_device;
	ID3D11DeviceContext *m_deviceContext;
	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11Texture2D *m_depthStencilBuffer;
	ID3D11DepthStencilState *m_depthStencilState;
        ID3D11DepthStencilState *m_depthStencilDisabledState;
	ID3D11DepthStencilView *m_depthStencilView;
	ID3D11RasterizerState *m_rasterState, *m_biasRasterState, *m_highBiasRasterState;
	DirectX::XMFLOAT4X4 m_projectionMatrix;
	DirectX::XMFLOAT4X4 m_orthoMatrix;

        HWND m_window;
};

#endif