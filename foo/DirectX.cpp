#include "Main.h"
#include "DirectX.h"

//--------------------------------------------------------------------------------------
// DirectX11::DirectX11()関数：コンストラクタ
//--------------------------------------------------------------------------------------
DirectX11::DirectX11()
{
    m_matWorld = DirectX::XMMatrixIdentity();
    m_matView = DirectX::XMMatrixIdentity();
    m_matProjection = DirectX::XMMatrixIdentity();
}

//--------------------------------------------------------------------------------------
// DirectX11::~DirectX11関数：デストラクタ
//--------------------------------------------------------------------------------------
DirectX11::~DirectX11()
{

}

//--------------------------------------------------------------------------------------
// DirectX11::CompileShaderFromFile()：シェーダのコンパイル
//--------------------------------------------------------------------------------------
HRESULT DirectX11::CompileShaderFromFile(const WCHAR* wcFileName, LPCSTR lpEntryPoint, LPCSTR lpShaderModel, ID3DBlob** D3DBlob)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> D3DErrorBlob;
    hr = D3DCompileFromFile(wcFileName, nullptr, nullptr, lpEntryPoint, lpShaderModel,
        dwShaderFlags, 0, D3DBlob, &D3DErrorBlob);
    if (FAILED(hr))
    {
        if (D3DErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(D3DErrorBlob->GetBufferPointer()));
        }
        return hr;
    }

    return S_OK;
}

//--------------------------------------------------------------------------------------
// DirectX11::InitDevice()：DirectX関係の初期化
//--------------------------------------------------------------------------------------
HRESULT DirectX11::InitDevice()
{
    //------------------------------------------------------------
    // DirectX11とDirect2D 1.1の初期化
    //------------------------------------------------------------
    HRESULT hr = S_OK;

    UINT uiDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
        uiDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_DRIVER_TYPE drivertypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT uiDriverTypesNum = ARRAYSIZE(drivertypes);

    D3D_FEATURE_LEVEL featurelevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT uiFeatureLevelsNum = ARRAYSIZE(featurelevels);
    Microsoft::WRL::ComPtr<ID3D11Device> D3DDevice;
    D3D_DRIVER_TYPE drivertype = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL featurelevel = D3D_FEATURE_LEVEL_11_0;
    for (UINT uiDriverTypeIndex = 0;uiDriverTypeIndex < uiDriverTypesNum;uiDriverTypeIndex++) {
        drivertype = drivertypes[uiDriverTypeIndex];
        hr = D3D11CreateDevice(nullptr, drivertype, nullptr, uiDeviceFlags, featurelevels, uiFeatureLevelsNum, D3D11_SDK_VERSION, &D3DDevice, &featurelevel, &m_D3DDeviceContext);

        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDevice(nullptr, drivertype, nullptr, uiDeviceFlags, &featurelevels[1], uiFeatureLevelsNum - 1,
                D3D11_SDK_VERSION, &D3DDevice, &featurelevel, &m_D3DDeviceContext);//&D3DDevice &m_D3DDeviceContext 初期化
        }

        if (SUCCEEDED(hr)) {
            break;
        }
    }
    if (FAILED(hr)) {
        return hr;
    }

    Microsoft::WRL::ComPtr<IDXGIDevice2> DXGIDevice2;
    hr = D3DDevice.As(&DXGIDevice2);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID2D1Factory1> D2DFactory1;
    D2D1_FACTORY_OPTIONS factoryoptions = {  };
#ifdef _DEBUG
    factoryoptions.debugLevel= D2D1_DEBUG_LEVEL_INFORMATION;
#endif
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factoryoptions, D2DFactory1.GetAddressOf());//&D2DFactory1ではなくD2DFactory1.GetAddressOf()
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID2D1Device> D2D1Device;
    hr = D2DFactory1->CreateDevice(DXGIDevice2.Get(), &D2D1Device);

    hr = D2D1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_D2DDeviceContext);//&m_D2DDeviceContext 初期化
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDXGIAdapter> DXGIAdapter;
    hr = DXGIDevice2->GetAdapter(&DXGIAdapter);//&DXGIAdapter 初期化
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDXGIFactory2> DXGIFactory2;
    hr = DXGIAdapter->GetParent(IID_PPV_ARGS(&DXGIFactory2));//&DXGIFactory2 初期化
    if (FAILED(hr))
        return hr;

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = Window::GetClientWidth();
    desc.Height = Window::GetClientHeight();
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

    hr = DXGIFactory2->CreateSwapChainForHwnd(D3DDevice.Get(), Window::GethWnd(), &desc, nullptr, nullptr, &m_DXGISwapChain1);
    (void)DXGIDevice2->SetMaximumFrameLatency(1);
    DXGIFactory2->MakeWindowAssociation(Window::GethWnd(), DXGI_MWA_NO_ALT_ENTER);
    Microsoft::WRL::ComPtr<IDXGISurface2> DXGISurface2;
    hr = m_DXGISwapChain1->GetBuffer(0, IID_PPV_ARGS(&DXGISurface2));
    if (FAILED(hr))
        return hr;

    hr = m_D2DDeviceContext->CreateBitmapFromDxgiSurface(DXGISurface2.Get(), D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), &m_D2DBitmap1);
    if (FAILED(hr))
        return hr;

    m_D2DDeviceContext->SetTarget(m_D2DBitmap1.Get());

    Microsoft::WRL::ComPtr<ID3D11Texture2D> D3DTexture2D;
    hr = m_DXGISwapChain1->GetBuffer(0, IID_PPV_ARGS(&D3DTexture2D));
    if (FAILED(hr))
        return hr;

    hr = D3DDevice->CreateRenderTargetView(D3DTexture2D.Get(), nullptr, &m_D3DRenderTargetView);
    if (FAILED(hr))
        return hr;
    m_D3DDeviceContext->OMSetRenderTargets(1, m_D3DRenderTargetView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)Window::GetClientWidth();
    vp.Height = (FLOAT)Window::GetClientHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_D3DDeviceContext->RSSetViewports(1, &vp);

    //バーテックスシェーダのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> D3DBlobVS;
    hr = CompileShaderFromFile(L"VertexShader.hlsl", "main", "vs_5_0", &D3DBlobVS);
    if (FAILED(hr))
        return hr;

    hr = D3DDevice->CreateVertexShader(D3DBlobVS->GetBufferPointer(), D3DBlobVS->GetBufferSize(), nullptr, &m_D3DVertexShader);
    if (FAILED(hr))
        return hr;

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    UINT uiElements = ARRAYSIZE(layout);

    Microsoft::WRL::ComPtr<ID3D11InputLayout> D3DInputLayout;
    hr = D3DDevice->CreateInputLayout(layout, uiElements, D3DBlobVS->GetBufferPointer(), D3DBlobVS->GetBufferSize(), &D3DInputLayout);
    if (FAILED(hr))
        return hr;
    m_D3DDeviceContext->IASetInputLayout(D3DInputLayout.Get());

    Microsoft::WRL::ComPtr<ID3DBlob> D3DBlobPS;
    hr = CompileShaderFromFile(L"PixelShader.hlsl", "main", "ps_5_0", &D3DBlobPS);
    if (FAILED(hr))
        return hr;
    hr = D3DDevice->CreatePixelShader(D3DBlobPS->GetBufferPointer(), D3DBlobPS->GetBufferSize(), nullptr, &m_D3DPixelShader);
    if (FAILED(hr))
        return hr;
    SimpleVertex vertices[] = {
        { DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(0.5f, 0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-0.5f,-0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(0.5f,-0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
    };

    D3D11_BUFFER_DESC bd = {  };
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(SimpleVertex) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    D3D11_SUBRESOURCE_DATA sub = {};
    sub.pSysMem = vertices;
    hr = D3DDevice->CreateBuffer(&bd, &sub, &m_D3DVertexBuffer);
    if (FAILED(hr))
        return hr;
    UINT uiStride = sizeof(SimpleVertex);
    UINT uiOffeset = 0;
    m_D3DDeviceContext->IASetVertexBuffers(0, 1, m_D3DVertexBuffer.GetAddressOf(), &uiStride, &uiOffeset);
    Microsoft::WRL::ComPtr<ID3D11Buffer> D3DIndexBuffer;
    WORD indices[] = {
        0,1,2,
        2,1,3
    };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 6;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    sub.pSysMem = indices;

    hr = D3DDevice->CreateBuffer(&bd, &sub, &D3DIndexBuffer);
    if (FAILED(hr))
        return hr;
    m_D3DDeviceContext->IASetIndexBuffer(D3DIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_D3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    hr = D3DDevice->CreateBuffer(&bd, nullptr, &m_D3DConstantBuffer);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> D3DRasterizerState;
    D3D11_RASTERIZER_DESC ras = {};
    ras.FillMode = D3D11_FILL_SOLID;
    ras.CullMode = D3D11_CULL_NONE;
    hr = D3DDevice->CreateRasterizerState(&ras, &D3DRasterizerState);
    if (FAILED(hr))
        return hr;
    m_D3DDeviceContext->RSSetState(D3DRasterizerState.Get());

    m_matWorld = DirectX::XMMatrixIdentity();
    DirectX::XMVECTOR vecEye = DirectX::XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);
    DirectX::XMVECTOR vecFocus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR vecUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_matView = DirectX::XMMatrixLookAtLH(vecEye, vecFocus, vecUp);

    m_matProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f),
        static_cast<FLOAT>(Window::GetClientWidth()) / static_cast<FLOAT>(Window::GetClientHeight()),
        0.001f, 100.0f);

    m_D3DDeviceContext->VSSetShader(m_D3DVertexShader.Get(), nullptr, 0);
    m_D3DDeviceContext->VSSetConstantBuffers(0, 1, m_D3DConstantBuffer.GetAddressOf());
    m_D3DDeviceContext->PSSetShader(m_D3DPixelShader.Get(), nullptr, 0);


    //------------------------------------------------------------
    // DirectWriteの初期化
    //------------------------------------------------------------
    Microsoft::WRL::ComPtr<IDWriteFactory> DWriteFactory;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &DWriteFactory);//&DWriteFactory 初期化
    if (FAILED(hr))
        return hr;

    hr = DWriteFactory->CreateTextFormat(L"メイリオ", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"", &m_DWriteTextFormat);//&m_DWriteTextFormat 初期化
    if (FAILED(hr))
        return hr;

    hr = m_DWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    if (FAILED(hr))
        return hr;

    hr = m_D2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_D2DSolidBrush);//&m_D2DSolidBrush 初期化
    if (FAILED(hr))
        return hr;

    return S_OK;
}

//--------------------------------------------------------------------------------------
// DirectX11::Render()：DirectX関係の描画
//--------------------------------------------------------------------------------------
void DirectX11::Render()
{
    m_D3DDeviceContext->ClearRenderTargetView(m_D3DRenderTargetView.Get(), DirectX::Colors::Aquamarine);//m_D3DRenderTargetViewではなくm_D3DRenderTargetView.Get()

    //------------------------------------------------------------
    //初期設定
    //------------------------------------------------------------
    static double dElapsedTime = 0;//経過時間

    //------------------------------------------------------------
    //計算
    //------------------------------------------------------------
    dElapsedTime += Window::GetFrameTime();

    //--------------------------★追加↓--------------------------
    m_matWorld = DirectX::XMMatrixIdentity();
    static FLOAT fangleX = 0;
    fangleX+= static_cast<FLOAT>(Window::GetFrameTime() / 1000.0);
    m_matWorld = DirectX::XMMatrixRotationY(fangleX*2);

    DirectX::XMVECTOR lightDir = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.0f, 0.5f, -1.0f, 0.0f));

    static FLOAT fViewMoveZ = 0;


    static FLOAT fMoveX = 0;
    static int iMoveX = 0;
    if (iMoveX == 0)
    {
        fMoveX += static_cast<FLOAT>(Window::GetFrameTime() / 1000.0);
        fViewMoveZ += static_cast<FLOAT>(Window::GetFrameTime() / 1000.0);
        if (fMoveX > 1.5f*2)
        {
            iMoveX = 1;
        }
    }
    else
    {
        fMoveX -= static_cast<FLOAT>(Window::GetFrameTime() / 1000.0);
        fViewMoveZ -= static_cast<FLOAT>(Window::GetFrameTime() / 1000.0);
        if (fMoveX < -1.5f*2)
        {
            iMoveX = 0;
        }
    }
    DirectX::XMMATRIX matMove;
    matMove = DirectX::XMMatrixTranslation(fMoveX, 0, 0);//X方向移動
    //m_matWorld = m_matWorld * matMove;

    DirectX::XMMATRIX matViewMove;
    matViewMove = DirectX::XMMatrixTranslation(0, 0, fViewMoveZ);
    //m_matView = m_matView * matViewMove;
    //--------------------------★追加↑--------------------------

    //------------------------------------------------------------
    // 文字操作
    //------------------------------------------------------------
    //FPS表示用
    /*
    WCHAR wcText1[256] = { 0 };
    swprintf(wcText1, 256, L"FPS=%lf", Window::GetFps());
    */
    //------------------------------------------------------------
    // 3D描画
    //------------------------------------------------------------
    //頂点の書き替え
    D3D11_MAPPED_SUBRESOURCE msr;
    /*
    m_D3DDeviceContext->Map(m_D3DVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    SimpleVertex vertices[] =
    {
        { DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(0.5f, 0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, static_cast<FLOAT>(fabs(cos(dElapsedTime / 1000))), 0.0f, 1.0f) },
    };
    memcpy(msr.pData, vertices, sizeof(vertices));
    m_D3DDeviceContext->Unmap(m_D3DVertexBuffer.Get(), 0);
        */
    //更新
    ConstantBuffer cb;
    cb.world = DirectX::XMMatrixTranspose(m_matWorld);
    cb.view = DirectX::XMMatrixTranspose(m_matView);
    DirectX::XMStoreFloat4(&cb.light, lightDir);
    cb.projection = DirectX::XMMatrixTranspose(m_matProjection);
    m_D3DDeviceContext->Map(m_D3DConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, (void*)(&cb), sizeof(cb));
    m_D3DDeviceContext->Unmap(m_D3DConstantBuffer.Get(), 0);
    m_D3DDeviceContext->DrawIndexed(6, 0, 0);
    
    m_DXGISwapChain1->Present(0, 0);
    
}