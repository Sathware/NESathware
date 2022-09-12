#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "SathwareException.h"
#include <fstream>
#include <filesystem>
#include "Color.h"
#include <vector>
#include "SathwareEngine.h"

class SathwareAPI Graphics
{
public:
	Graphics(const class DesktopWindow& window);
	void ClearBuffer();
	void Render();

	void PutPixel(unsigned __int32 x, unsigned __int32 y, Color c)
	{
		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y < m_height);

		frameImage[m_width * y + x] = c;
	}

	size_t m_width;
	size_t m_height;

	Graphics() = delete;
	Graphics(const Graphics& other) = delete;
	Graphics(const Graphics&& other) = delete;
	Graphics& operator=(const Graphics& other) = delete;
private:
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	void InitializeDeviceAndContext();
	void InitializeSwapChain(HWND windowHandle);
	void InitializeRenderTarget();
	void InitializeDepthStencil();
	void InitializeShaders();
	void InitializeInputLayout();
	void InitializeVertexBuffer();
	void InitializePixelShaderTexture();
	void UpdateFrame();

	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mContext;

	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D11Texture2D> mSwapChainBackBuffer;
	ComPtr<ID3D11RenderTargetView> mRenderTargetView;

	//Future proofing, Depth Stencil buffer and view 
	//are not being used currently
	ComPtr<ID3D11Texture2D> mDepthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> mDepthStencilView;

	ComPtr<ID3D11VertexShader> mVertexShader;
	std::vector<__int8> mVertexShaderBinary;

	ComPtr<ID3D11InputLayout> mInputLayout;
	ComPtr<ID3D11Buffer> mVertexBuffer;

	ComPtr<ID3D11PixelShader> mPixelShader;
	std::vector<__int8> mPixelShaderBinary;

	ComPtr<ID3D11Texture2D> mShaderTexture;
	ComPtr<ID3D11ShaderResourceView> mShaderResourceView;
	ComPtr<ID3D11SamplerState> mSampler;

	D3D11_VIEWPORT mViewPort{};

	//Currently not used, needed to support older/lower end hardware
	D3D_FEATURE_LEVEL mChosenFeatureLevel;

	unsigned int numVertices;

	Color* frameImage;

	struct Vertex
	{
		float x, y, z, tU, tV;
	};
};