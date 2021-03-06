#include "Graphics.h"

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;

	if (!InitializeDirectX(hwnd))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device.Get(), deviceContext.Get());
	ImGui::StyleColorsDark();

	return true;
}

void Graphics::RenderFrame()
{
	float bgcolor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), bgcolor);
	this->deviceContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	this->deviceContext->IASetInputLayout(this->vertexshader.GetInputLayout());
	this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->deviceContext->RSSetState(this->rasterizerState.Get());
	this->deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
	this->deviceContext->OMSetBlendState(blendState.Get(), nullptr, 0xFFFFFFF);
	this->deviceContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());
	this->deviceContext->VSSetShader(vertexshader.GetShader(), NULL, 0);
	this->deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);

	UINT offset = 0;
	static float objectScale[] = { 1.0f,1.0f,1.0f };
	static float objectScaling = 1.0f;
	static float objectAlpha = 1.0f;
	static float translationOffset[3] = { 0,0,-2.0f };
	static float objectCatScaling = 2.5f;
	
	{ //Object 2
		//Update Constant Buffer

		XMMATRIX world = XMMatrixScaling(objectCatScaling, objectCatScaling, objectCatScaling) * XMMatrixTranslation(0.0f, 0.0f, 4.0f);
		constantVSBuffer.data.mat = world * camera.GetViewMatrix() * camera.GetProjectionMatrix();
		constantVSBuffer.data.mat = DirectX::XMMatrixTranspose(constantVSBuffer.data.mat);

		if (!constantVSBuffer.ApplyChanges())
			return;
		this->deviceContext->VSSetConstantBuffers(0, 1, this->constantVSBuffer.GetAddressOf());
		;
		this->constantPSBuffer.data.alpha = 1.0f;
		this->constantPSBuffer.ApplyChanges();
		this->deviceContext->PSSetConstantBuffers(0, 1, this->constantPSBuffer.GetAddressOf());

		//Square
		this->deviceContext->PSSetShaderResources(0, 1, this->myAppleTexture.GetAddressOf());

		this->deviceContext->IASetVertexBuffers(0, 1, vertexBufferPlane.GetAddressOf(), vertexBuffer.StridePtr(), &offset);
		this->deviceContext->IASetIndexBuffer(indicesBufferPlane.Get(), DXGI_FORMAT_R32_UINT, 0);
		this->deviceContext->DrawIndexed(indicesBufferPlane.BufferSize(), 0, 0);
	}
	{ //Object
		//Update Constant Buffer

		XMMATRIX world = XMMatrixScaling(objectScale[0], objectScale[1], objectScale[2]) * XMMatrixTranslation(translationOffset[0], translationOffset[1], translationOffset[2]);
		constantVSBuffer.data.mat = world * camera.GetViewMatrix() * camera.GetProjectionMatrix();
		constantVSBuffer.data.mat = DirectX::XMMatrixTranspose(constantVSBuffer.data.mat);

		if (!constantVSBuffer.ApplyChanges())
			return;
		this->deviceContext->VSSetConstantBuffers(0, 1, this->constantVSBuffer.GetAddressOf());
		;
		this->constantPSBuffer.data.alpha = objectAlpha;
		this->constantPSBuffer.ApplyChanges();
		this->deviceContext->PSSetConstantBuffers(0, 1, this->constantPSBuffer.GetAddressOf());

		//Square
		this->deviceContext->PSSetShaderResources(0, 1, this->myTexture.GetAddressOf());

		this->deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), vertexBuffer.StridePtr(), &offset);
		this->deviceContext->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		this->deviceContext->DrawIndexed(indicesBuffer.BufferSize(), 0, 0);
	}
	{ //Picture	
	//Update Constant Buffer
		static float translationOffset[3] = { 0,0,1.0f };
		XMMATRIX world = XMMatrixScaling(8.0f, 8.0f, 8.0f) * XMMatrixTranslation(translationOffset[0], translationOffset[1], translationOffset[2]);
		constantVSBuffer.data.mat = world * camera.GetViewMatrix() * camera.GetProjectionMatrix();
		constantVSBuffer.data.mat = DirectX::XMMatrixTranspose(constantVSBuffer.data.mat);

		if (!constantVSBuffer.ApplyChanges())
			return;
		this->deviceContext->VSSetConstantBuffers(0, 1, this->constantVSBuffer.GetAddressOf());

		this->constantPSBuffer.data.alpha = 1.0f;
		this->constantPSBuffer.ApplyChanges();
		this->deviceContext->PSSetConstantBuffers(0, 1, this->constantPSBuffer.GetAddressOf());



		//Square
		this->deviceContext->PSSetShaderResources(0, 1, this->myPicture.GetAddressOf());

		this->deviceContext->IASetVertexBuffers(0, 1, vertexBufferPlane.GetAddressOf(), vertexBufferPlane.StridePtr(), &offset);
		this->deviceContext->IASetIndexBuffer(indicesBufferPlane.Get(), DXGI_FORMAT_R32_UINT, 0);
		this->deviceContext->DrawIndexed(indicesBufferPlane.BufferSize(), 0, 0);
	}
		
	
	
	//Draw Text
	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), L"", DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f,0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
	spriteBatch->End();


	//IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Object");
	ImGui::DragFloat("Alpha", &objectAlpha, 0.1f, 0.0f, 1.0f);
	ImGui::Text("Scale");
	ImGui::InputFloat3("Scale", objectScale);
	ImGui::DragFloat("X scale", &objectScale[0], 0.1f, 0.1f, 5.0f);
	ImGui::DragFloat("Y scale", &objectScale[1], 0.1f, 0.1f, 5.0f);
	ImGui::DragFloat("Z scale", &objectScale[2], 0.1f, 0.1f, 5.0f);
	if (ImGui::Button("Reset Scaling")) {
		objectScale[0] = 1.0f;
		objectScale[1] = 1.0f;
		objectScale[2] = 1.0f;
	}
	ImGui::Text("Position");

	ImGui::InputFloat3("", translationOffset);
	ImGui::DragFloat("X pos", &translationOffset[0], 0.1f, -5.0f, 5.0f);
	ImGui::DragFloat("Y pos", &translationOffset[1], 0.1f, -5.0f, 5.0f);
	ImGui::DragFloat("Z pos", &translationOffset[2], 0.1f, -5.0f, 5.0f);
	if (ImGui::Button("Reset Position")) {
		translationOffset[0] = 0.0f;
		translationOffset[1] = 0.0f;
		translationOffset[2] = -2.0f;
	}
	ImGui::End();
	ImGui::Begin("Cat");
	ImGui::DragFloat("ScalingCat", &objectCatScaling, 0.1f, 0.1f, 5.0f);
	
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	this->swapChain->Present(1, NULL);
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		ErrorLogger::Log("No IDXGI Adapters found.");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferDesc.Width = this->windowWidth;
	scd.BufferDesc.Height = this->windowHeight;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = hwnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(	adapters[0].pAdapter, //IDXGI Adapter
										D3D_DRIVER_TYPE_UNKNOWN,
										NULL, //FOR SOFTWARE DRIVER TYPE
										NULL, //FLAGS FOR RUNTIME LAYERS
										NULL, //FEATURE LEVELS ARRAY
										0, //# OF FEATURE LEVELS IN ARRAY
										D3D11_SDK_VERSION,
										&scd, //Swapchain description
										this->swapChain.GetAddressOf(), //Swapchain Address
										this->device.GetAddressOf(), //Device Address
										NULL, //Supported feature level
										this->deviceContext.GetAddressOf()); //Device Context Address

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create device and swapchain.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	hr = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "GetBuffer Failed.");
		return false;
	}

	hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->renderTargetView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create render target view.");
		return false;
	}

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = this->windowWidth;
	depthStencilDesc.Height = this->windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = this->device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil buffer.");
		return false;
	}

	hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil view.");
		return false;
	}

	this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil state.");
		return false;
	}

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = this->windowWidth;
	viewport.Height = this->windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Set the Viewport
	this->deviceContext->RSSetViewports(1, &viewport);

	//Create Rasterizer State
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create rasterizer state.");
		return false;
	}

	spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->deviceContext.Get());
	spriteFont = std::make_unique<DirectX::SpriteFont>(this->device.Get(), L"Data\\Fonts\\comic_sans_ms_16.spritefont");

	// Set Blend State
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; 
	
	device->CreateBlendState(&blendDesc, blendState.GetAddressOf());
	
	if (FAILED(hr)) {
		ErrorLogger::Log(hr, "Failed to create Blend State");
		return false;
	}
	//Create sampler description for sampler state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf()); //Create sampler state
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create sampler state.");
		return false;
	}

	return true;
}

bool Graphics::InitializeShaders()
{

	std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
	if (IsDebuggerPresent() == TRUE)
	{
#ifdef _DEBUG //Debug Mode
	#ifdef _WIN64 //x64
			shaderfolder = L"..\\x64\\Debug\\";
	#else  //x86 (Win32)
			shaderfolder = L"..\\Debug\\";
	#endif
	#else //Release Mode
	#ifdef _WIN64 //x64
			shaderfolder = L"..\\x64\\Release\\";
	#else  //x86 (Win32)
			shaderfolder = L"..\\Release\\";
	#endif
#endif
	}
	
	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements = ARRAYSIZE(layout);

	if (!vertexshader.Initialize(this->device, shaderfolder + L"vertexshader.cso", layout, numElements))
		return false;

	if (!pixelshader.Initialize(this->device, shaderfolder + L"pixelshader.cso"))
		return false;


	return true;
}

bool Graphics::InitializeScene()
{
	//Textured Figure	
	Vertex v[] =
	{
		/*Vertex(-0.5f, -0.5f, 0.0f,		0.0f, 1.0f), //Bottom Left   - [0]
		Vertex(-0.5f, 0.5f, 0.0f,			0.0f, 0.0f), //Top Left      - [1]
		Vertex(0.5f, 0.5f, 0.0f,			1.0f, 0.0f), //Top Right     - [2]
		Vertex(0.5f, -0.5f, 0.0f,		1.0f, 1.0f),*/ //Bottom Right   - [3]
		Vertex(0.0f,2.0f,0.0f	, 0.0f, 1.0f),
		Vertex(-1.0f,1.0f,-1.0f	, 0.0f,	0.8f	),
		Vertex(1.0f,1.0f,-1.0f	, 0.0f,	0.6f),
		Vertex(1.0f,1.0f,1.0f	, 0.0f,	0.4f),
		Vertex(-1.0f,1.0f,1.0f	, 0.0f,	0.2f),

		Vertex(-0.5f,0.0f,-0.5f	, 0.0f, 0.0f),
		Vertex(0.5f,0.0f,-0.5f	, 	0.2f, 0.0f),
		Vertex(0.5f,0.0f,0.5f	, 	0.4f, 0.0f),
		Vertex(-0.5f,0.0f,0.5f	, 	0.7f, 0.0f),

		Vertex(-1.0f,-1.0f,-1.0f, 	1.0f, 0.0f),
		Vertex(1.0f,-1.0f,-1.0f	, 	1.0f, 0.3f ),
		Vertex(1.0f,-1.0f,1.0f	, 	1.0f,  0.6f),
		Vertex(-1.0f,-1.0f,1.0f	, 1.0f, 1.0f),
	
	
	};
	Vertex vPlane[] =
	{
		Vertex(-0.5f, -0.5f, 0.0f,		0.0f, 1.0f), //Bottom Left   - [0]
		Vertex(-0.5f, 0.5f, 0.0f,		0.0f, 0.0f), //Top Left      - [1]
		Vertex(0.5f, 0.5f, 0.0f,		1.0f, 0.0f), //Top Right     - [2]
		Vertex(0.5f, -0.5f, 0.0f,		1.0f, 1.0f), //Bottom Right   - [3]
		


	};
	
	//Load Vertex Data
	HRESULT hr = this->vertexBuffer.Initialize(this->device.Get(), v, ARRAYSIZE(v));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create vertex buffer.");
		return false;
	}
	hr = this->vertexBufferPlane.Initialize(this->device.Get(), vPlane, ARRAYSIZE(v));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create vertex buffer.");
		return false;
	}

	DWORD indices[] =
	{
		0,2,1,
		3,2,0,
		0,4,3,
		4,0,1,
		5,1,2, 2,6,5,
		3,6,2, 3,7,6,
		3,4,8, 3,8,7,
		8,4,1, 5,8,1,
		6,9,5, 10,9,6,
		10,6,11, 6,7,11,
		7,8,11, 11,8,12,
		5,12,8, 9,12,5,
		11,12,9, 11,9,10,
	};
	DWORD indicesPlane[] =
	{
		0,1,2,
		0,2,3,
		
	};
	//Load Index Data
	
	hr = this->indicesBuffer.Initialize(this->device.Get(), indices, ARRAYSIZE(indices));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create indices buffer.");
		return hr;
	}
	hr = this->indicesBufferPlane.Initialize(this->device.Get(), indicesPlane, ARRAYSIZE(indices));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create indices buffer.");
		return hr;
	}

	//Load Texture
	hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\piano.png", nullptr, myTexture.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create wic texture from file.");
		return false;
	}
	hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\cat.png", nullptr, myAppleTexture.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create wic texture from file.");
		return false;
	}
	hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\Texture.bmp", nullptr, myPicture.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create wic texture from file.");
		return false;
	}
	//Initialize Constant Buffer(s)
	hr = this->constantVSBuffer.Initialize(this->device.Get(), this->deviceContext.Get());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to initialize constant buffer.");
		return false;
	}
	hr = this->constantPSBuffer.Initialize(this->device.Get(), this->deviceContext.Get());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to initialize constant buffer.");
		return false;
	}
	camera.SetPosition(0.0f, 0.0f, -5.0f);
	camera.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 1000.0f);

	return true;
}
