#include<windows.h>
#include<d3dx9.h>
#include<d3d9.h>
#include<dinput.h>

#define WND_CLASS_NAME TEXT("My_Window")
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define SAFE_RELEASE(p) {if(p){(p)->Release();(p)=NULL;}}
#define WINDOWWIDTH 1052
#define WINDOWHEIGTE 592

LPDIRECT3D9 g_pD3d;
LPDIRECT3DDEVICE9 g_pDevice;
LPDIRECT3DTEXTURE9 g_pTexture;
LPDIRECT3DTEXTURE9 g_pTex_Enemy;
LPDIRECT3DTEXTURE9 g_pTex_Enemy2;
LPDIRECTINPUT8 g_pDinput = NULL;
LPDIRECTINPUTDEVICE8 g_pKeyDevice;
LPD3DXFONT m_pFONT;
FLOAT fWall_PosX = WINDOWWIDTH / 2, fWall_PosY = WINDOWHEIGTE / 2;
FLOAT fEnemy_PosX = 0, fEnemy_PosY = 0;
FLOAT fEnemy2_PosX=WINDOWWIDTH/2, fEnemy2_PosY = WINDOWHEIGTE/2;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3d(HWND hWnd);
HRESULT InitDinput(HWND hWnd, HINSTANCE hInstance);
void Enemy();
void Enemy2();
void WallPeper();
void FreeRELEASES();
void KeyBind();
void BeginScene();
void EndScene();


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_ INT nCmdShow) {

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
		TEXT("STEP4"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WINDOWWIDTH,
		WINDOWHEIGTE,
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
	InitDinput(hWnd, hInstance);

	while (msg.message != WM_QUIT) {
		Sleep(1);
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
				
				BeginScene();
				WallPeper();
				Enemy();
				Enemy2();
				KeyBind();
				EndScene();
			}
		}
	}
	timeEndPeriod(1);

	FreeRELEASES();

	return (INT)msg.wParam;
}


LRESULT CALLBACK WindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

HRESULT InitD3d(HWND hWnd) {
	//DirectXオブジェクト生成
	g_pD3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_pD3d == NULL) {
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	D3DDISPLAYMODE	D3DdisplayMode;

	g_pD3d->GetAdapterDisplayMode(
		D3DADAPTER_DEFAULT,
		&D3DdisplayMode);

	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.BackBufferCount = 1;
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;


	g_pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pDevice);

	D3DVIEWPORT9 viewport{ 0,0,d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,0.0f,1.0f };
	g_pDevice->SetViewport(&viewport);


	//描画方法の設定----------------------------------------------------------透過をちゃんと表示する処理
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


	D3DXCreateTextureFromFile(g_pDevice, "titlebackground.png", &g_pTexture);
	D3DXCreateTextureFromFile(g_pDevice, "enemys.png", &g_pTex_Enemy);
	
	//GOwanTAKERUのpath


	return S_OK;
}

void BeginScene() {

	g_pDevice->Clear(0, NULL,
		D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0xff, 0xff, 0xff),
		0, 0);

	g_pDevice->BeginScene();

}

void EndScene() {

	g_pDevice->EndScene();
	g_pDevice->Present(NULL, NULL, NULL, NULL);

}

void WallPeper() {

	g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	const int WINDOW_W = 1052;
	const int WINDOW_H = 592;

	//ポリゴンを作るための情報
	struct D3DCUSTOMVERTEX {
		float x, y, z, rhw;
		DWORD color;
		float tu, tv;
	};
	//zは１
	D3DCUSTOMVERTEX vertex[] = {
	{fWall_PosX - WINDOW_W / 2,fWall_PosY - WINDOW_H / 2,1,1,0xffffffff,0,0},
	{fWall_PosX + WINDOW_W / 2,fWall_PosY - WINDOW_H / 2,1,1,0xffffffff,1,0},
	{fWall_PosX + WINDOW_W / 2,fWall_PosY + WINDOW_H / 2,1,1,0xffffffff,1,1},
	{fWall_PosX - WINDOW_W / 2,fWall_PosY + WINDOW_H / 2,1,1,0xffffffff,0,1},
	};

	
	// テクスチャをステージに割り当てる
	g_pDevice->SetTexture(0, g_pTexture);
	// 描画
	g_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(D3DCUSTOMVERTEX));
	//D3DPT_TRIANGLEFANで右回りで頂点を指定する
	//2はサン角形２枚

}


void Enemy() {

	g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	//ポリゴンを作るための情報
	struct D3DCUSTOMVERTEX {
		float x, y, z, rhw;
		DWORD color;
		float tu, tv;
	};

	//zは１
	D3DCUSTOMVERTEX vertex[] = {
	{fEnemy_PosX,fEnemy_PosY,1,1,0xffffffff,0,0},
	{fEnemy_PosX+ 68 ,fEnemy_PosY,1,1,0xffffffff,68.0f/1024.0f,0},
	{fEnemy_PosX+ 68 ,fEnemy_PosY+112,1,1,0xffffffff,68.0f/1024.0f,112.0f/1024.0f},
	{fEnemy_PosX,fEnemy_PosY+112,1,1,0xffffffff,0,112.0f / 1024.0f},
	};
	//0fは割り算の答えを実数にする為にfloatのfを付与する
	//tu,tvは貼り付けたい画像の中のサイズを設定する
	//fEnemy_PosX+100は今の場所（０）から右に１００ポリゴン描画する
	//1024*1024の画像なので÷1024


	// テクスチャをステージに割り当てる
	g_pDevice->SetTexture(0, g_pTex_Enemy);
	// 描画
	g_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(D3DCUSTOMVERTEX));
	//D3DPT_TRIANGLEFANで右回りで頂点を指定する
	//2はサン角形２枚
}


void Enemy2() {

	g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	//ポリゴンを作るための情報
	struct D3DCUSTOMVERTEX {
		float x, y, z, rhw;
		DWORD color;
		float tu, tv;
	};

	//zは１
	D3DCUSTOMVERTEX vertex[] = {
	{fEnemy2_PosX,fEnemy2_PosY,1,1,0xffffffff,0,0},
	{fEnemy2_PosX + 68 ,fEnemy2_PosY,1,1,0xffffffff,68.0f / 1024.0f,0},
	{fEnemy2_PosX + 68 ,fEnemy2_PosY + 112,1,1,0xffffffff,68.0f / 1024.0f,112.0f / 1024.0f},
	{fEnemy2_PosX,fEnemy2_PosY + 112,1,1,0xffffffff,0,112.0f / 1024.0f},
	};
	//0fは割り算の答えを実数にする為にfloatのfを付与する
	//tu,tvは貼り付けたい画像の中のサイズを設定する
	//fEnemy_PosX+100は今の場所（０）から右に１００ポリゴン描画する


	// テクスチャをステージに割り当てる
	g_pDevice->SetTexture(0, g_pTex_Enemy);
	// 描画
	g_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(D3DCUSTOMVERTEX));
	//D3DPT_TRIANGLEFANで右回りで頂点を指定する
	//2はサン角形２枚
}

HRESULT InitDinput(HWND hWnd, HINSTANCE hInstance) {
	HRESULT hr;//hrとは？ハンドルリザルト

	//DirectInputオブジェクト作成
	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(VOID * *)& g_pDinput,
		NULL);

	//DirectInput Device オブジェクトの作成
	hr = g_pDinput->CreateDevice(GUID_SysKeyboard, &g_pKeyDevice, NULL);

	//Deviceをキーボードに設定
	hr = g_pKeyDevice->SetDataFormat(&c_dfDIKeyboard);

	//協調レベル設定　排他的バックグラウンド
	hr = g_pKeyDevice->SetCooperativeLevel(
		hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

	g_pKeyDevice->Acquire();

	return S_OK;
}

void KeyBind() {
	HRESULT hr = g_pKeyDevice->Acquire();
	if ((hr == DI_OK) || (hr == S_FALSE)) {
		BYTE diks[256];
		g_pKeyDevice->GetDeviceState(sizeof(diks), &diks);

		if (diks[DIK_LEFT] & 0x80)
		{
			fEnemy_PosX -= 4;
		}

		if (diks[DIK_RIGHT] & 0x80)
		{
			fEnemy_PosX += 4;
		}

		if (diks[DIK_UP] & 0x80)
		{
			fEnemy_PosY -= 4;
		}

		if (diks[DIK_DOWN] & 0x80)
		{
			fEnemy_PosY += 4;
		}

	}
}


void FreeRELEASES() {
	SAFE_RELEASE(g_pTexture);
	SAFE_RELEASE(g_pDevice);
	SAFE_RELEASE(g_pD3d);
	SAFE_RELEASE(g_pDinput);
	SAFE_RELEASE(g_pKeyDevice);
	SAFE_RELEASE(g_pTex_Enemy);
	SAFE_RELEASE(g_pTex_Enemy);
	SAFE_RELEASE(g_pTex_Enemy2);
}