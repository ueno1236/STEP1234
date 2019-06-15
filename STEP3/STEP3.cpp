#include <windows.h>
#include<d3dx9.h>
#include<d3d9.h>

#define WND_CLASS_NAME TEXT("My_Window")
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define SAFE_RELEASE(p) {if(p){(p)->Release();(p)=NULL;}}

LPDIRECT3D9 g_pD3d;
LPDIRECT3DDEVICE9 g_pDevice;
LPDIRECT3DTEXTURE9 g_pTexture;


LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3d(HWND hWnd);
void WallPeper();
void FreeRELEASES();



int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	INT nCmdShow) {

	HWND hWnd;
	WNDCLASSEX wcl;

	wcl.cbSize = sizeof(WNDCLASSEX);
	wcl.hInstance = hInstance;
	wcl.lpszClassName = WND_CLASS_NAME;
	wcl.lpfnWndProc = WindowProc;
	wcl.style = CS_HREDRAW | CS_VREDRAW;
	wcl.hIcon = NULL;
	wcl.hIconSm = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_CROSS);
	wcl.lpszMenuName = NULL;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (!RegisterClassEx(&wcl)) {
		return FALSE;
	}

	hWnd = CreateWindowEx(
		NULL,
		WND_CLASS_NAME,
		TEXT("STEP2"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1980,
		1020,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	MSG msg;
	timeBeginPeriod(1);

	DWORD previoustime = timeGetTime();
	DWORD currenttime;

	ZeroMemory(&msg, sizeof(msg));
	InitD3d(hWnd);
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			currenttime = timeGetTime();

			if (currenttime - previoustime >= 1000 / 60) {
				previoustime = currenttime;
				Sleep(1);
				WallPeper();
			}
		}
	}
	timeEndPeriod(1);

	FreeRELEASES();

	return msg.wParam;
}


LRESULT CALLBACK WindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg)
	{
	case WM_DESTROY://windowMessage
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch ((CHAR)wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT InitD3d(HWND hWnd) {
	//DirectXオブジェクト生成
	g_pD3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_pD3d == NULL) {
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	D3DDISPLAYMODE		  D3DdisplayMode;

	g_pD3d->GetAdapterDisplayMode(
		D3DADAPTER_DEFAULT,
		&D3DdisplayMode);

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferFormat = D3DdisplayMode.Format;
	d3dpp.BackBufferCount = 1;
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;


	g_pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pDevice);

	D3DVIEWPORT9 viewport{ 0,0,d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,0.0f,1.0f };
	g_pDevice->SetViewport(&viewport);


	//描画方法の設定--------------------------------------------------------------------わからん
	g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  //SRCの設定
	g_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_pDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);

	g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);



	D3DXCreateTextureFromFile(g_pDevice, "bullets.png", &g_pTexture);


	return S_OK;
}

void WallPeper() {

	g_pDevice->Clear(0, NULL,
		D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0x00, 0x00, 0x00),
		0, 0);

	g_pDevice->BeginScene();

	g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	//ポリゴンを作るための情報
	struct D3DCUSTOMVERTEX {
		float x, y, z, rhw;
		DWORD color;
		float tu, tv;
	};
	//zは１
	D3DCUSTOMVERTEX vertex[] = {
	{0,0,1,1,0x00ffffff,0,0},
	{640,0,1,1,0x00ffffff,1,0},
	{640,480,1,1,0xffffffff,1,1},
	{0,480,1,1,0xffffffff,0,1},
	};

	// テクスチャをステージに割り当てる
	g_pDevice->SetTexture(0, g_pTexture);
	// 描画
	g_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(D3DCUSTOMVERTEX));
	//D3DPT_TRIANGLEFANで右回りで頂点を指定する
	//2はサン角形２枚

	g_pDevice->BeginScene();
	g_pDevice->Present(NULL, NULL, NULL, NULL);

}

void FreeRELEASES() {
	SAFE_RELEASE(g_pTexture);
	SAFE_RELEASE(g_pDevice);
	SAFE_RELEASE(g_pD3d);
}