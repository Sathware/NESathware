#include "Graphics.h"
#include "DesktopWindow.h"
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d11_3.h>
#include <d3d11_4.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3d11sdklayers.h>

#if defined(DEBUG) || defined(_DEBUG)
#define ThrowIfFailed(result, string) if(FAILED(result)) throw Exception(string);
#else
#define ThrowIfFailed(result, string) 
#endif

Graphics::Graphics(const DesktopWindow& window)
	: m_width(window.mClientWidth), m_height(window.mClientHeight), 
	frameImage(new Color[m_width*m_height])
{
	InitializeDeviceAndContext();
	InitializeSwapChain(window.m_windowHandle);
	InitializeRenderTarget();
	InitializeDepthStencil();
	InitializeShaders();
	InitializeInputLayout();
	InitializeVertexBuffer();
	InitializePixelShaderTexture();

	//Initialize View Port
	mViewPort.Width = (float)this->m_width;
	mViewPort.Height = (float)this->m_height;
	mViewPort.TopLeftX = 0;
	mViewPort.TopLeftY = 0;
	mViewPort.MinDepth = 0;
	mViewPort.MaxDepth = 0;

	//stride = size in bytes of 'elements' in vertex buffer 
	UINT strides[] = { sizeof(Vertex) };
	//offset = size in bytes to the first vertex element to be used from the start of the buffer
	UINT offsets[] = { 0 };
	mContext->IASetInputLayout(mInputLayout.Get());
	mContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), strides, offsets);
	mContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	mContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	mContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	mContext->PSSetShaderResources(0, 1, mShaderResourceView.GetAddressOf());
	mContext->PSSetSamplers(0, 1, mSampler.GetAddressOf());

	mContext->RSSetViewports(1, &mViewPort);

	//Useful Direct3D Debugging tools
#if defined(DEBUG) || defined(_DEBUG)

	ComPtr<IDXGIDevice3> dxgiDevice;
	HRESULT result = mDevice.As(&dxgiDevice);
	ThrowIfFailed(result, L"Failed to get DXGI Device from DirectX Device!");

	ComPtr<IDXGIAdapter> dxgiAdapter;
	result = dxgiDevice->GetAdapter(&dxgiAdapter);
	ThrowIfFailed(result, L"Failed to get Adapter/GPU from DXGI Device!");

	DXGI_ADAPTER_DESC GPU_Summary;
	dxgiAdapter->GetDesc(&GPU_Summary);
	OutputDebugStringW(L"GPU INFO ------------\n");
	OutputDebugStringW(GPU_Summary.Description);
	OutputDebugStringW((L"\nVRAM in MB:" + std::to_wstring(GPU_Summary.DedicatedVideoMemory / 1048576) + L"\n").c_str());

	ID3D11Debug* deviceDebug;
	ID3D11InfoQueue* debugInfo;

	result = mDevice->QueryInterface(&deviceDebug);
	ThrowIfFailed(result, L"Failed to get Debug Interface from D3D Device!");
	result = mDevice->QueryInterface(&debugInfo);
	ThrowIfFailed(result, L"Failed to get Debug Info Queue from D3D Device!");

	result = deviceDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
	ThrowIfFailed(result, L"Call to ReportLiveDeviceObjects Failed!");
	result = debugInfo->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	ThrowIfFailed(result, L"Failed call to SetBreakOnSeverity for Corruptions!");
	result = debugInfo->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	ThrowIfFailed(result, L"Failed call to SetBreakOnSeverity for Errors!");

	debugInfo->Release();
	deviceDebug->Release();
#endif
}

void Graphics::InitializeDeviceAndContext()
{
	UINT deviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL chosenFeatureLevel;

	HRESULT result = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		deviceFlags,
		levels,
		(UINT)std::size(levels),
		D3D11_SDK_VERSION,
		&mDevice,
		&chosenFeatureLevel,
		&mContext);
	ThrowIfFailed(result, L"Failed to create D3D11 Device!");

	mChosenFeatureLevel = chosenFeatureLevel;
}

void Graphics::InitializeSwapChain(HWND windowHandle)
{
	//Specify Swap Chain Parameters/Behavior
	DXGI_SWAP_CHAIN_DESC desc{};
	desc.BufferDesc.Width = (UINT)m_width;
	desc.BufferDesc.Height = (UINT)m_height;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.BufferCount = 2;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.OutputWindow = windowHandle;
	desc.Windowed = true;

	//Use the same factory that created the d3d device to also create the swap chain,
	//This avoids having to remake all resources if we wished to only remake the swap chain
	ComPtr<IDXGIDevice3> dxgiDevice;
	HRESULT result = mDevice.As(&dxgiDevice);
	ThrowIfFailed(result, L"Failed to get DXGI Device from DirectX Device!");

	ComPtr<IDXGIAdapter> dxgiAdapter;
	result = dxgiDevice->GetAdapter(&dxgiAdapter);
	ThrowIfFailed(result, L"Failed to get Adapter/GPU from DXGI Device!");

	ComPtr<IDXGIFactory> dxgiFactory;
	result = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	ThrowIfFailed(result, L"Failed to get DXGI Factory from DXGI Adapter/GPU!");

	result = dxgiFactory->CreateSwapChain(
		mDevice.Get(),
	    &desc,
		&mSwapChain);
	ThrowIfFailed(result, L"Failed to Create Swap Chain!");
}

void Graphics::InitializeRenderTarget()
{
	D3D11_RENDER_TARGET_VIEW_DESC desc{};
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	//Bind render target view to Swap Chain back buffer
	HRESULT result = mSwapChain->GetBuffer(0, IID_PPV_ARGS(&mSwapChainBackBuffer));
	ThrowIfFailed(result, L"Failed to get read/write buffer from swap chain!");

	result = mDevice->CreateRenderTargetView(
		mSwapChainBackBuffer.Get(),
		&desc,
		mRenderTargetView.GetAddressOf());
	ThrowIfFailed(result, L"Failed to bind Swap Chain Back Buffer to Render Target View!");
}

void Graphics::InitializeDepthStencil()
{
	D3D11_TEXTURE2D_DESC bufferDesc{};
	bufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	bufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	bufferDesc.Width = (UINT)m_width;
	bufferDesc.Height = (UINT)m_height;
	bufferDesc.MipLevels = 1;
	bufferDesc.ArraySize = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	HRESULT result = mDevice->CreateTexture2D(&bufferDesc, nullptr, &mDepthStencilBuffer);
	ThrowIfFailed(result, L"Failed to create depth stencil buffer!");

	D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
	viewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Flags = 0;
	viewDesc.Texture2D.MipSlice = 0;

	result = mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &viewDesc, &mDepthStencilView);
	ThrowIfFailed(result, L"Failed to create Depth Stencil View!");
}

void Graphics::InitializeShaders()
{
	namespace fs = std::filesystem;

	//Check that the shaders are present
	if (!fs::exists(fs::current_path() / L"VertexShader.cso"))
		throw Exception(L"Vertex Shader is not in the same directory as executable!");
	if (!fs::exists(fs::current_path() / L"PixelShader.cso"))
		throw Exception(L"Pixel Shader is not in the same directory as executable!");

	//Initialize input file stream object to throw exceptions
	std::ifstream shaderFile;
	shaderFile.exceptions(std::fstream::badbit | std::fstream::failbit);

	//Read vertex shader bytecode file and create vertex shader
	size_t numVertexShaderFileBytes = fs::file_size(fs::current_path() / L"VertexShader.cso");
	mVertexShaderBinary.resize(numVertexShaderFileBytes);

	shaderFile.open(L"VertexShader.cso", std::ios_base::binary);
	shaderFile.read(mVertexShaderBinary.data(), numVertexShaderFileBytes);
	shaderFile.close();

	HRESULT result = mDevice->CreateVertexShader(mVertexShaderBinary.data(), mVertexShaderBinary.size(), nullptr, &mVertexShader);
	ThrowIfFailed(result, L"Failed to create vertex shader!");

	//Read pixel shader bytecode file and create pixel shader
	size_t numPixelShaderFileBytes = fs::file_size(fs::current_path() / L"PixelShader.cso");
	mPixelShaderBinary.resize(numPixelShaderFileBytes);

	shaderFile.open(L"PixelShader.cso", std::ios_base::binary);
	shaderFile.read(mPixelShaderBinary.data(), numPixelShaderFileBytes);
	shaderFile.close();

	result = mDevice->CreatePixelShader(mPixelShaderBinary.data(), mPixelShaderBinary.size(), nullptr, &mPixelShader);
	ThrowIfFailed(result, L"Failed to create vertex shader!");
}

void Graphics::InitializeInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC descriptions[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HRESULT result = mDevice->CreateInputLayout(
		descriptions, 
		(UINT)std::size(descriptions),
		mVertexShaderBinary.data(), 
		mVertexShaderBinary.size(), 
		&mInputLayout);
	ThrowIfFailed(result, L"Failed to Initialize Input Layout!");
}

void Graphics::InitializeVertexBuffer()
{
	Vertex vertexData[] =
	{
		{-1.0f,-1.0f, 0.0f, 0.0f, 1.0f},
		{-1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
		{ 1.0f,-1.0f, 0.0f, 1.0f, 1.0f},
		{ 1.0f, 1.0f, 0.0f, 1.0f, 0.0f}
	};
	numVertices = (unsigned int)std::size(vertexData);

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = sizeof(vertexData);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = vertexData;
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	HRESULT result = mDevice->CreateBuffer(&desc, &initialData, &mVertexBuffer);
	ThrowIfFailed(result, L"Failed to create vertex buffer!");
}

void Graphics::InitializePixelShaderTexture()
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = (UINT)m_width;
	textureDesc.Height = (UINT)m_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA resource{};
	resource.pSysMem = frameImage;
	resource.SysMemPitch = (UINT)m_width * sizeof(Color);
	resource.SysMemSlicePitch = 0;

	HRESULT result = mDevice->CreateTexture2D(&textureDesc, &resource, &mShaderTexture);
	ThrowIfFailed(result, L"Failed to create Texture for shaders!");
	
	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc{};
	resourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Buffer.FirstElement = 0;
	resourceViewDesc.Buffer.NumElements = (UINT)(m_width * m_height);
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	resourceViewDesc.Texture2D.MipLevels = 1;

	result = mDevice->CreateShaderResourceView(mShaderTexture.Get(), &resourceViewDesc, &mShaderResourceView);
	ThrowIfFailed(result, L"Failed to create Shader Resource View!");

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;

	result = mDevice->CreateSamplerState(&samplerDesc, &mSampler);
	ThrowIfFailed(result, L"Failed to create Sampler for texture!");
}

void Graphics::Clear()
{
	for (size_t i = 0; i < m_width * m_height; ++i)
		frameImage[i].rgba = 0x00000000;
}

void Graphics::UpdateFrame()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	HRESULT result = mContext->Map(mShaderTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ThrowIfFailed(result, L"Mapping texture buffer for CPU write failed!");

	Color* d3dTexture = reinterpret_cast<Color*>(mappedResource.pData);
	size_t scanLineWidth = mappedResource.RowPitch / sizeof(Color);

	for (size_t y = 0; y < m_height; ++y)
	{
		memcpy(&d3dTexture[y * scanLineWidth], &frameImage[y * m_width], m_width * sizeof(Color));
	}

	mContext->Unmap(mShaderTexture.Get(), 0);
}

void Graphics::Render()
{
	const float clearColor[4] = { 0.1843f, 0.207f, 0.235f, 1.0f };
	mContext->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);
	mContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), nullptr);

	UpdateFrame();

	mContext->Draw(numVertices, 0);
	HRESULT result = mSwapChain->Present(1, 0);
	ThrowIfFailed(result, L"Presenting Fram failed!");
}