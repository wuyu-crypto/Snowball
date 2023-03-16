//=============================================================================
//
// フォント処理
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "font.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	TEXTURE_MAX	128

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static bool g_Load = false;

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitFont(void) {

	// フォントハンドルの設定
	int fontSize = 64;
	int fontWeight = 1000;
	LOGFONTW lf =
	{
		fontSize, 0, 0, 0, 
		fontWeight, 0, 0, 0,
		SHIFTJIS_CHARSET, 
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		DEFAULT_PITCH | FF_MODERN,
		(WCHAR)"ＭＳ Ｐ明朝"
	};
	// フォントハンドルの生成
	HFONT hFont = CreateFontIndirectW(&lf);

	// 現在のウィンドウに適用
	// デバイスにフォントを持たせないとGetGlyphOutline関数はエラーとなる
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

	// フォントビットマップ取得
	const wchar_t* c = L"S";
	UINT code = (UINT)*c;

	// 階調を設定
	const int gradFlag = GGO_GRAY4_BITMAP;

	// ビットマップを設定
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	GLYPHMETRICS gm;	// ビットマップデータ
	CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };

	// フォントビットマップを取得
	DWORD size = GetGlyphOutlineW(hdc, code, gradFlag, &gm, 0, NULL, &mat);
	BYTE* pMono = new BYTE[size];
	GetGlyphOutlineW(hdc, code, gradFlag, &gm, size, pMono, &mat);

	// ハンドルとコンテキスト解放
	SelectObject(hdc, oldFont);
	ReleaseDC(NULL, hdc);

	// フォントの幅と高さ
	INT fontWidth = (gm.gmBlackBoxX + 3) / 4 * 4;
	INT fontHeight = gm.gmBlackBoxY;

	// レンダーターゲットの設定
	D3D11_TEXTURE2D_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));
	rtDesc.Width = fontWidth;
	rtDesc.Height = fontHeight;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.SampleDesc.Quality = 0;
	rtDesc.Usage = D3D11_USAGE_DYNAMIC;
	rtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	rtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	rtDesc.MiscFlags = 0;

	// フォント用テクスチャ作成
	ID3D11Texture2D* layerBuffer = 0;
	ID3D11Device* pDevice = GetDevice();
	if (FAILED(pDevice->CreateTexture2D(&rtDesc, nullptr, &layerBuffer))) {
		return E_FAIL;
	}

	// デバイスコンテキストを取得
	auto deviceContext = GetDeviceContext();

	// フォント用テクスチャリソースにテクスチャ情報をコピー
	D3D11_MAPPED_SUBRESOURCE mappedSubrsrc;
	deviceContext->Map(
		layerBuffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedSubrsrc);
	// ここで書き込む
	BYTE* pBits = (BYTE*)mappedSubrsrc.pData;
	// フォント情報の書き込み
	// iOfs_x, iOfs_y : 書き出し位置(左上)
	// iBmp_w, iBmp_h : フォントビットマップの幅高
	// Level : α値の段階 (GGO_GRAY4_BITMAPなので17段階)
	int iOfs_x = gm.gmptGlyphOrigin.x;
	int iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y;
	int iBmp_w = gm.gmBlackBoxX + (4 - (gm.gmBlackBoxX % 4)) % 4;
	int iBmp_h = gm.gmBlackBoxY;
	int Level = 17;
	int x, y;
	DWORD Alpha, Color;
	memset(pBits, 0, mappedSubrsrc.RowPitch * tm.tmHeight);
	for (y = iOfs_y; y < iOfs_y + iBmp_h; y++)
	{
		for (x = iOfs_x; x < iOfs_x + iBmp_w; x++)
		{
			Alpha =
				(255 * pMono[x - iOfs_x + iBmp_w * (y - iOfs_y)])
				/ (Level - 1);
			Color = 0x00ffffff | (Alpha << 24);
			memcpy(
				(BYTE*)pBits + mappedSubrsrc.RowPitch * y + 4 * x,
				&Color,
				sizeof(DWORD));
		}
	}
	deviceContext->Unmap(layerBuffer, 0);


	// ShaderResourceViewの情報を作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = rtDesc.MipLevels;

	// ShaderResourceViewの情報を書き込む
	ID3D11ShaderResourceView* srv;
	pDevice->CreateShaderResourceView(layerBuffer, &srvDesc, &srv);

	g_Texture[0] = srv;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// いろいろ解放
	delete[] pMono;

	g_Load = true;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitFont(void)
{
	if (!g_Load) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}


	g_Load = false;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateFont(void) {

}
//=============================================================================
// 描画処理
//=============================================================================
void DrawFont(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	float px = 100;
	float py = 100;
	float pw = 100;
	float ph = 100;

	float tw = 1.0f;
	float th = 1.0f;
	float tx = 0.0f;
	float ty = 0.0f;

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSprite(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th);

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);
}