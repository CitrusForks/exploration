////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "vanillashaderclass.h"
#include <xnamath.h>


#include <iostream>

VanillaShaderClass::VanillaShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
        m_lightBuffer = 0;
        m_cameraBuffer = 0;
	m_sampleState = 0;
}


VanillaShaderClass::VanillaShaderClass(const VanillaShaderClass& other)
{
}


VanillaShaderClass::~VanillaShaderClass()
{
}


void VanillaShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}


bool VanillaShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, CXMMATRIX worldMatrix, CXMMATRIX viewMatrix, 
								CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, CXMVECTOR cameraPos)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, cameraPos);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


bool VanillaShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, wchar_t *vsFilename, char *vsFunctionName, wchar_t *psFilename, char *psFunctionName)
{
    HRESULT result;
    ID3D10Blob* errorMessage = 0;


    ID3D10Blob* vertexShaderBuffer = nullptr;

    // Compile the vertex shader code.
    result = D3DX11CompileFromFile(vsFilename, NULL, NULL, vsFunctionName, "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								&vertexShaderBuffer, &errorMessage, NULL); // XXX D3DX11 is deprecated, replace this with CompileFromFile()
    if(FAILED(result))
    {
	    // If the shader failed to compile it should have writen something to the error message.
	    if(errorMessage)
	    {
		    OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
	    }
	    // If there was nothing in the error message then it simply could not find the shader file itself.
	    else
	    {
		    MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
	    }

	    return false;
    }


    ID3D10Blob* pixelShaderBuffer = nullptr;

    // Compile the pixel shader code.
    result = D3DX11CompileFromFile(psFilename, NULL, NULL, psFunctionName, "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								&pixelShaderBuffer, &errorMessage, NULL); // XXX replace with CompileFromFile()
    if(FAILED(result))
    {
	    // If the shader failed to compile it should have writen something to the error message.
	    if(errorMessage)
	    {
		    OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
	    }
	    // If there was  nothing in the error message then it simply could not find the file itself.
	    else
	    {
		    MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
	    }

	    return false;
    }

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if(FAILED(result))
    {
	    return false;
    }

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if(FAILED(result))
    {
	    return false;
    }

    // Create the vertex input layout description.
    // This setup needs to match the VertexType stucture in the ModelClass and in the shader.
    // XXX Is this really a good place to define it then?
    D3D11_INPUT_ELEMENT_DESC polygonLayout[4];
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "NORMAL";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    polygonLayout[2].SemanticName = "TEXCOORD";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    polygonLayout[3].SemanticName = "TEXCOORD";
    polygonLayout[3].SemanticIndex = 1;
    polygonLayout[3].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[3].InputSlot = 0;
    polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[3].InstanceDataStepRate = 0;


    // Get a count of the elements in the layout.
    unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                            &m_layout);
    if(FAILED(result))
    {
	    return false;
    }

    // Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    D3D11_BUFFER_DESC cameraBufferDesc = matrixBufferDesc; // in the interest of brevity, copy all the identical values from a previous buffer_desc
    D3D11_BUFFER_DESC lightBufferDesc = cameraBufferDesc;  // also these need to be copied before the first call to CreateBuffer() because it may do odd things to the data; no, really, this actually happened.


    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if(FAILED(result))
    {
        std::cerr << "Could not create constant buffer!" << std::endl;
	return false;
    }

    // another buffer for camera data

    cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);

    result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
    if(FAILED(result))
    {
        std::cerr << "Could not create constant buffer!" << std::endl;
	return false;
    }

    // another buffer, for the light data in the pixel shader
    cameraBufferDesc.ByteWidth = sizeof(LightBufferType);

    result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
    if(FAILED(result))
    {
        std::cerr << "Could not create constant buffer!" << std::endl;
	return false;
    }

    // Create a texture sampler state description.
    D3D11_SAMPLER_DESC samplerDesc;
    //samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 8;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if(FAILED(result))
    {
	    return false;
    }

    std::cout << "initialized some shaders";

    return true;
}


void VanillaShaderClass::ShutdownShader()
{
    // Release the sampler state.
    if(m_sampleState)
    {
	    m_sampleState->Release();
	    m_sampleState = 0;
    }

    // Release the matrix constant buffer.
    if(m_matrixBuffer)
    {
	    m_matrixBuffer->Release();
	    m_matrixBuffer = 0;
    }

    // Release the layout.
    if(m_layout)
    {
	    m_layout->Release();
	    m_layout = 0;
    }

    // Release the pixel shader.
    if(m_pixelShader)
    {
	    m_pixelShader->Release();
	    m_pixelShader = 0;
    }

    // Release the vertex shader.
    if(m_vertexShader)
    {
	    m_vertexShader->Release();
	    m_vertexShader = 0;
    }

    return;
}


void VanillaShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	std::cerr << "Error compiling shader:";

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
		std::cerr << compileErrors[i];
	}

        std::cerr << std::endl;

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


bool VanillaShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, CXMMATRIX worldMatrix, CXMMATRIX viewMatrix, 
											 CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, CXMVECTOR cameraPos)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    CameraBufferType* cDataPtr;
    unsigned int bufferNumber;
    //XMFLOAT4X4 worldF4X4, viewF4X4, projectionF4X4;    

    // Transpose the matrices to prepare them for the shader.

    //D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
    //D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
    //D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

    deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);


    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result))
    {
	    return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // Copy the matrices into the constant buffer.
    XMStoreFloat4x4(&dataPtr->world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&dataPtr->view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&dataPtr->projection, XMMatrixTranspose(projectionMatrix));

    //dataPtr->world = worldMatrix;
    //dataPtr->view = viewMatrix;
    //dataPtr->projection = projectionMatrix;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    //
    // camera constant buffer!
    //
    result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result))
    {
	 return false;
    }

    // Get a pointer to the data in the constant buffer.
    cDataPtr = (CameraBufferType*)mappedResource.pData;
    
    XMStoreFloat4(&cDataPtr->cameraPosition, cameraPos);
    //cDataPtr->padding = 0.0f;

    deviceContext->Unmap(m_cameraBuffer, 0);

    ++bufferNumber;
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);


    // Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);

    return true;
}

// set light values and time
bool VanillaShaderClass::SetPSConstants( ID3D11DeviceContext *deviceContext, XMFLOAT4 &ambientColor, XMFLOAT4 &diffuseColor, XMFLOAT3 &lightDirection, float specularPower, XMFLOAT4 &specularColor, float time, FXMVECTOR cameraPos)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    LightBufferType* dataPtr;

    deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);


    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    dataPtr = (LightBufferType*)mappedResource.pData;

    dataPtr->ambientColor = ambientColor;
    dataPtr->diffuseColor = diffuseColor;
    dataPtr->lightDirection = lightDirection;
    dataPtr->specularPower = specularPower;
    dataPtr->specularColor = specularColor;
    dataPtr->time = time;
    //dataPtr->padding[0] = dataPtr->padding[1] = dataPtr->padding[2] = 0.0f;
    XMStoreFloat3(&dataPtr->cameraPos, cameraPos);



    deviceContext->Unmap(m_lightBuffer, 0);

    deviceContext->PSSetConstantBuffers(0, 1, &m_lightBuffer);

    return true;
}



void VanillaShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

        deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
        deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangles
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}