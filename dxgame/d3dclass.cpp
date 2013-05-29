////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "d3dclass.h"

#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <iomanip>
#include <vector>


// from http://www.rastertek.com/dx11tut03.html

using namespace std;
using namespace DirectX;

D3DClass::D3DClass()
{
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
}


D3DClass::D3DClass(const D3DClass& other)
{
}


D3DClass::~D3DClass()
{
}


bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, 
						  float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;

	unsigned int numModes, numerator, denominator;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	ID3D11Texture2D* backBufferPtr;
	//float fieldOfView, screenAspect;


	// Store the vsync setting.
	m_vsync_enabled = vsync;

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if(FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).

	IDXGIAdapter* adapter;
	vector<IDXGIAdapter*> adapters;

	unsigned int i = 0;
	while (factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		++i;
		adapters.push_back(adapter);

		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);
		wstring_convert<codecvt_utf8<wchar_t>> conv1;
		string u8str = conv1.to_bytes(desc.Description);

		if (i == 0) strncpy_s(m_videoCardDescription, u8str.size(), u8str.c_str(), 127);

		cout << u8str << endl;
	}

	if (i == 0)
	{
		return false;
	}

	adapter = adapters[0];

	// Enumerate the primary adapter output (monitor).

	IDXGIOutput* adapterOutput;
	vector<IDXGIOutput*> adapterOutputs;

	i = 0;
	while (adapter->EnumOutputs(i, &adapterOutput) != DXGI_ERROR_NOT_FOUND)
	{
		++i;
		adapterOutputs.push_back(adapterOutput);

		DXGI_OUTPUT_DESC desc;
		adapterOutput->GetDesc(&desc);
		wstring_convert<codecvt_utf8<wchar_t>> conv1;
		string u8str = conv1.to_bytes(desc.DeviceName);
	
		cout << u8str << endl;
	}

	if(i == 0)
	{
		return false;
	}

	adapterOutput = adapterOutputs[0];

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
        for(i = 0; i < numModes; ++i)
        {
            cout << "Mode " << i << ": " << displayModeList[i].Width << "x" << displayModeList[i].Height << endl;
        }

	for(i=0; i<numModes; i++)
	{
		if(displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if(displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
                                break;
			}
		}
	}

	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if(FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / (1024 * 1024));

	// Release the display mode list.
	delete [] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	for (auto i = adapterOutputs.cbegin(); i != adapterOutputs.cend(); ++i) (*i)->Release();
	adapterOutputs.clear();
	adapterOutput = 0;

	// Release the adapter.
	for (auto i = adapters.cbegin(); i != adapters.cend(); ++i) (*i)->Release();
	adapters.clear();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	// Initialize the swap chain description.
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
        swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
        swapChainDesc.BufferDesc.Width = screenWidth;
        swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for the back buffer.
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if(m_vsync_enabled)
	{
	    swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
            swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
	    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
        swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off for swap chaing; we're doign it when rendering to off-screen target. 
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if(fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 10+.
	D3D_FEATURE_LEVEL featureLevels[3] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

        unsigned creationFlags = 0;
#if defined(_DEBUG)
        // If the project is in a debug build, enable the debug layer.
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, featureLevels, 3, 
						D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if(FAILED(result))
	{   
            Errors::Cry("Could not initialize device! Crashing now?");
            return false;
	}

	// Get the pointer to the back buffer.
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if(FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if(FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	// Initialize the description of the depth buffer.
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	const DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; // for more z precision, switch to DXGI_FORMAT_D32_FLOAT (w/o stencil) or DXGI_FORMAT_D32_FLOAT_S8X24_UINT (w/ 8-bit stencil and wasted 24 bits??)
	unsigned int maxQL = 0;
	m_device->CheckMultisampleQualityLevels(depthStencilFormat, Options::intOptions["MSAACount"], &maxQL);
	if (!maxQL)
	{
		cerr << "Warning, " << Options::intOptions["MSAACount"] << "x MSAA unsupported by GPU. :(";
		Options::intOptions["MSAACount"] = 1;
		Options::intOptions["MSAAQuality"] = 0;
	} else
	{
		Options::intOptions["MSAAQuality"] = maxQL-1;
	}

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth; // * 2; // x 2 for supersampling test
	depthBufferDesc.Height = screenHeight; // * 2;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = depthStencilFormat;
	depthBufferDesc.SampleDesc.Count = Options::intOptions["MSAACount"];
	depthBufferDesc.SampleDesc.Quality = Options::intOptions["MSAAQuality"];
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // effectively doesn't use stencil buffer

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // same as above

        D3D11_DEPTH_STENCIL_DESC depthStencilDisabledDesc = depthStencilDesc;
        depthStencilDisabledDesc.DepthEnable = false;
        depthStencilDisabledDesc.StencilEnable = false;

	// Create the depth stencil state.
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if(FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

        // Create the non-depth non-stencil state?
        result = m_device->CreateDepthStencilState(&depthStencilDisabledDesc, &m_depthStencilDisabledState);
        if(FAILED(result))
        {
            return false;
        }

	// Initialize the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = depthStencilFormat;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS; // D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if(FAILED(result))
	{
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
        // Update: don't bother binding the depth buffer here; we're going to use it with the off-screen buffer
        // NOTE Maybe I should put it in the IntermediateRenderTarget class then? Who knows.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr /*m_depthStencilView*/);

	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if(FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState);

        // Create a rasterizer state with depth bias for drawing shadow maps
        rasterDesc.AntialiasedLineEnable = false;
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.DepthBias = 1; // XXX need a good value here
        rasterDesc.DepthBiasClamp = 10000.0f; // here too
        rasterDesc.DepthClipEnable = false;
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.FrontCounterClockwise = false;
        rasterDesc.MultisampleEnable = true;
        rasterDesc.ScissorEnable = false;
        rasterDesc.SlopeScaledDepthBias = 1.0f; // also here!

        // really create it
        result = m_device->CreateRasterizerState(&rasterDesc, &m_biasRasterState);
        if(FAILED(result))
        {
            return false;
        }
        


	// Setup the viewport for rendering.
	D3D11_VIEWPORT viewport;
        viewport.Width = (float)1024; //screenWidth; // * 2; // x2 for supersampling test
        viewport.Height = (float)1024; //screenHeight; // * 2;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;

	// Create the viewport.
        m_deviceContext->RSSetViewports(1, &viewport);

        // d3dmatrix stuff that I'm not using:
#if 0
	// Setup the projection matrix.
	fieldOfView = (float)D3DX_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

        // Initialize the world matrix to the identity matrix.
        D3DXMatrixIdentity(&m_worldMatrix);

	// Create an orthographic projection matrix for 2D rendering.
	D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);
#endif

    return true;
}


void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if(m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if(m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if(m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if(m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if(m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if(m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if(m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if(m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if(m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}


void D3DClass::BeginScene( bool clear /*= true*/, float red /*= 0.0f*/, float green /*= 0.0f*/, float blue /*= 0.0f*/, float alpha /*= 0.0f*/ )
{
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	if (clear) m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
    
	// Clear the depth buffer. (always for now)
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


void D3DClass::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if(m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	return;
}


ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}


void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = XMLoadFloat4x4(&m_projectionMatrix);
	return;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = XMLoadFloat4x4(&m_orthoMatrix);
	return;
}


void D3DClass::GetVideoCardInfo( std::string cardName, int &memory )
{
	cardName = string(m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}


// we need this to pass on to IntermediateRenderTarget
ID3D11DepthStencilView * D3DClass::GetDepthStencilView()
{
    return m_depthStencilView;
}


void D3DClass::setAsRenderTarget( bool depthEnable )
{
    // Bind the render target view and depth stencil buffer to the output render pipeline.
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, depthEnable ? m_depthStencilView : nullptr);
}


// turn on depth and stencil checks by setting the appropriate state
void D3DClass::depthOn()
{
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
}


// turn off depth and stencil checks
void D3DClass::depthOff()
{
    m_deviceContext->OMSetDepthStencilState(m_depthStencilDisabledState, 0);
}


// set rasterizer state to one of two choices based on parameter
void D3DClass::setDepthBias( bool onOrOffOrWhatever )
{
    m_deviceContext->RSSetState(onOrOffOrWhatever ? m_biasRasterState : m_rasterState);
}


