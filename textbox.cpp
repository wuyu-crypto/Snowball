//=============================================================================
//
// 文字の表示
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "textbox.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************

#define	TEXTBOX_MAX		512					// 最大文字列数
#define	TEXT_MAX		256					// 一文(1枚のテキスト画像)の最大文字数

#define	DEFAULT_SIZE	32					// デフォルトフォントサイズ
#define	DEFAULT_WEIGHT	10					// デフォルトフォントウェイト
#define	DEFAULT_GRAD	GGO_GRAY4_BITMAP	// デフォルト階調

#define	VECTOR


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct TEXTBOX {
	bool		isInitiated;	// 初期化済みか

	bool		use;	// 有効化フラグ
	wchar_t		text;	// 文字
	XMFLOAT3	pos;	// 1文字目の左上座標
	XMFLOAT4	color;	// 色

	ID3D11Buffer* vertexBuffer;				// 頂点バッファ
	ID3D11ShaderResourceView* textureSrv;	// テクスチャデータ
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static bool g_Load = false;

static TEXTBOX g_Textbox[TEXTBOX_MAX];

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTextbox(void) {

	SetTextbox(L"天空", "装甲明朝", 0, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), true);

#ifndef VECTOR
	// フォントハンドルの設定
	int fontSize = 64;
	int fontWeight = 10000;
	LOGFONT lf = {
		fontSize, 0, 0, 0,
		fontWeight, 0, 0, 0,
		SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		DEFAULT_PITCH | FF_MODERN,
		_T("装甲明朝")
	};

	// フォントハンドルの生成
	HFONT hFont = CreateFontIndirect(&lf);
	if (hFont == NULL) {
		return E_FAIL;
	}

	// 現在のウィンドウに適用
	// デバイスにフォントを持たせないとGetGlyphOutline関数はエラーとなる
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

	// Unicode文字を取得
	const wchar_t* c = L"abcあいう";	// Unicode文字
	UINT code = (UINT)*c;				// Unicodeに変換

	// 階調を設定
	const int gradFlag = GGO_GRAY4_BITMAP;
	int grad = 0;	// 階調の最大値
	switch (gradFlag) {
	case GGO_GRAY2_BITMAP: grad = 4;	break;
	case GGO_GRAY4_BITMAP: grad = 16;	break;
	case GGO_GRAY8_BITMAP: grad = 64;	break;
	}
	if (grad == 0) {
		return E_FAIL;
	}

	// ビットマップを設定
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	GLYPHMETRICS gm;
	CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };
	// ビットマップに必要なブロックサイズを調べる
	DWORD size = GetGlyphOutlineW(
		hdc, 		// フォントが設定してあるデバイスコンテキストハンドル
		code,		// 表示したい文字をUnicodeで設定
		gradFlag,	// 解像度
		&gm,		// ビットマップ情報格納先
		0,			// ブロックサイズ
		NULL,		// ビットマップを保存するブロックメモリ
		&mat		// 回転（ここは変換なし）
	);
	BYTE* pFontBMP = new BYTE[size];
	// 調べたサイズによりビットマップを再生成
	GetGlyphOutlineW(hdc, code, gradFlag, &gm, size, pFontBMP, &mat);

	// コンテキルトとハンドルはもう要らないから解放
	SelectObject(hdc, oldFont);
	ReleaseDC(NULL, hdc);



	// フォントの幅と高さ
	INT fontWidth = gm.gmCellIncX;
	INT fontHeight = tm.tmHeight;

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
	ID3D11Texture2D* pTex = 0;
	ID3D11Device* pDevice = GetDevice();
	if (FAILED(pDevice->CreateTexture2D(&rtDesc, nullptr, &pTex))) {
		return E_FAIL;
	}

	// デバイスコンテキストを取得
	auto deviceContext = GetDeviceContext();

	// フォント用テクスチャリソースにテクスチャ情報をコピー
	D3D11_MAPPED_SUBRESOURCE mappedSubrsrc;
	deviceContext->Map(pTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubrsrc);
	// ここで書き込む
	BYTE* pBits = (BYTE*)mappedSubrsrc.pData;

	// フォントビットマップ情報(ドットごとのα値)をテクスチャに書き込む
	// iOfs_x, iOfs_y : 書き出し位置(左上)
	int iOfs_x = gm.gmptGlyphOrigin.x;
	int iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y;
	// iBmp_w, iBmp_h : フォントビットマップの幅高
	int iBmp_w = gm.gmBlackBoxX + (4 - (gm.gmBlackBoxX % 4)) % 4;	// 4バイト単位に変換
	int iBmp_h = gm.gmBlackBoxY;
	// Level : α値の段階(grad+1段階)
	int Level = grad + 1;
	DWORD Alpha, Color;
	// 下地テクスチャのビットマップを0クリア
	memset(
		pBits,									// セット先メモリブロック 
		0,										// セットする数値
		mappedSubrsrc.RowPitch * tm.tmHeight	// セットするバイト数
	);
	for (int y = iOfs_y; y < iOfs_y + iBmp_h; y++)
	{
		for (int x = iOfs_x; x < iOfs_x + iBmp_w; x++)
		{
			// 理解不能
			Alpha = (255 * pFontBMP[x - iOfs_x + iBmp_w * (y - iOfs_y)]) / (Level - 1);
			Color = 0x00ffffff | (Alpha << 24);
			memcpy(
				(BYTE*)pBits + mappedSubrsrc.RowPitch * y + 4 * x,	// コピー先ブロック
				&Color,												// コピー元
				sizeof(DWORD));
		}
	}
	deviceContext->Unmap(pTex, 0);


	// ShaderResourceViewの情報を作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = rtDesc.MipLevels;

	// ShaderResourceViewの情報を書き込む
	ID3D11ShaderResourceView* srv;
	pDevice->CreateShaderResourceView(pTex, &srvDesc, &srv);

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
	delete[] pFontBMP;

#endif // !VECTOR

	g_Load = true;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTextbox(void)
{
#ifndef VECTOR


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

	// テキスト構造体をすべて未初期化にする
	for (int i = 0; i < TEXTBOX_MAX; i++) {
		g_Textbox[i].isInitiated = false;
	}


	g_Load = false;

#endif // !VECTOR
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTextbox(void) {

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTextbox(void)
{

#ifndef VECTOR


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

	float px = 0;
	float py = 0;
	float pw = 100;
	float ph = 100;

	float tw = 1.0f;
	float th = 1.0f;
	float tx = 0.0f;
	float ty = 0.0f;

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	// 文字の色もここで設定
	SetSpriteLTColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);

#endif // !VECTOR


}



bool SetTextbox(wchar_t* text, TCHAR* font, int size, int weight, XMFLOAT4 color, XMFLOAT3 pos, bool mode) {

	// 空のテキスト構造体スロットを探す
	TEXTBOX* textbox;
	bool empty = false;
	for (int i = 0; i < TEXTBOX_MAX; i++) {
		if (!g_Textbox[i].isInitiated) {	// 未初期化スロットあり
			textbox = &g_Textbox[i];		// ポインタを指す
			empty = true;
			break;
		}
	}
	// 空きスロットがなければテキスト生成失敗
	if (!empty) {
		return false;
	}

	// フォントハンドルの設定
	int fontSize, fontWeight;
	if (size == 0) {
		fontSize = DEFAULT_SIZE;
	}
	else
	{
		fontSize = size;
	}
	if (weight == 0) {
		fontWeight = DEFAULT_WEIGHT;
	}
	else
	{
		fontWeight = weight;
	}

	// フォント属性を定義
	LOGFONT lf = {
		fontSize, 0, 0, 0,
		fontWeight, 0, 0, 0,
		SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		DEFAULT_PITCH | FF_MODERN,
		_T("")
	};
	// フォント名を再取得
	for (int i = 0; i < _tcslen(font); i++) {
		lf.lfFaceName[i] = font[i];
	}

	// フォントハンドルの生成
	HFONT hFont = CreateFontIndirect(&lf);
	if (hFont == NULL) {
		return false;
	}

	// 現在のウィンドウに適用
	// デバイスにフォントを持たせないとGetGlyphOutline関数はエラーとなる
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);


	// 文字列の長さを調べる
	int len = wcslen(text);

	// 256文字目以降は切り落とされる
	if (len > TEXT_MAX) {
		len = TEXT_MAX;
	}

	// Unicode文字を取得
	UINT* code = new UINT[len];			// Unicode文字列
	for (int i = 0; i < len; i++) {
		code[i] = (UINT)(text + i);		// 1文字ごと取得
	}

	// 階調を設定
	const int gradFlag = DEFAULT_GRAD;
	int grad = 0;	// 階調の最大値
	switch (gradFlag) {
	case GGO_GRAY2_BITMAP: grad = 4;	break;
	case GGO_GRAY4_BITMAP: grad = 16;	break;
	case GGO_GRAY8_BITMAP: grad = 64;	break;
	}
	if (grad == 0) {
		return false;
	}

	// 文字列情報を1文字ごとに取得
	TEXTMETRIC tm[TEXT_MAX];
	GLYPHMETRICS gm[TEXT_MAX];
	for (int nText = 0; nText < len; nText++) {

		// ビットマップを設定
		GetTextMetrics(hdc, &tm[nText]);
		CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };
		// ビットマップに必要なブロックサイズを調べる
		DWORD size = GetGlyphOutlineW(
			hdc, 			// フォントが設定してあるデバイスコンテキストハンドル
			code[nText],	// 表示したい文字をUnicodeで設定
			gradFlag,		// 解像度
			&gm[nText],			// ビットマップ情報格納先
			0,				// ブロックサイズ
			NULL,			// ビットマップを保存するブロックメモリ
			&mat			// 回転（ここは変換なし）
		);
		BYTE* pFontBMP = new BYTE[size];
		// 調べたサイズによりビットマップを再生成
		GetGlyphOutlineW(hdc, code[nText], gradFlag, &gm[nText], size, pFontBMP, &mat);
	}

	// コンテキルトとハンドルはもう要らないから解放
	SelectObject(hdc, oldFont);
	ReleaseDC(NULL, hdc);

	// テクスチャの合計幅高を計算
	int textboxWidth, textboxHeight = 0;
	for (int nText = 0; nText < len; nText++) {
		// 合計幅は各文字のgm.gmCellIncXの合計
		textboxWidth += gm[nText].gmCellIncX;
		// 合計高さは全文字のtm.tmHeightの最大値
		textboxHeight = (textboxHeight < tm[nText].tmHeight) ? tm[nText].tmHeight : textboxHeight;

	}

	// 1枚のデカい空白テクスチャを作る
	// レンダーターゲットの設定
	D3D11_TEXTURE2D_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));
	rtDesc.Width = textboxWidth;
	rtDesc.Height = textboxHeight;
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
	ID3D11Texture2D* pTex = 0;
	ID3D11Device* pDevice = GetDevice();
	if (FAILED(pDevice->CreateTexture2D(&rtDesc, nullptr, &pTex))) {
		return E_FAIL;
	}

	// デバイスコンテキストを取得
	auto deviceContext = GetDeviceContext();

	// フォント用テクスチャリソースにテクスチャ情報をコピー
	D3D11_MAPPED_SUBRESOURCE mappedSubrsrc;
	deviceContext->Map(pTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubrsrc);

	// ここに書き込む
	BYTE* pBits = (BYTE*)mappedSubrsrc.pData;

	// テクスチャに書き込む
	for (int nText = 0; nText < len; nText++) {
		// フォントビットマップ情報(ドットごとのα値)をテクスチャに書き込む
		// iOfs_x, iOfs_y : 書き出し位置(左上)
		int iOfs_x = gm.gmptGlyphOrigin.x;
		int iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y;
		// iBmp_w, iBmp_h : フォントビットマップの幅高
		int iBmp_w = gm.gmBlackBoxX + (4 - (gm.gmBlackBoxX % 4)) % 4;	// 4バイト単位に変換
		int iBmp_h = gm.gmBlackBoxY;
		// Level : α値の段階(grad+1段階)
		int Level = grad + 1;
		DWORD Alpha, Color;
		// 下地テクスチャのビットマップを0クリア
		memset(
			pBits,									// セット先メモリブロック 
			0,										// セットする数値
			mappedSubrsrc.RowPitch * tm.tmHeight	// セットするバイト数
		);
		for (int y = iOfs_y; y < iOfs_y + iBmp_h; y++)
		{
			for (int x = iOfs_x; x < iOfs_x + iBmp_w; x++)
			{
				// 理解不能
				Alpha = (255 * pFontBMP[x - iOfs_x + iBmp_w * (y - iOfs_y)]) / (Level - 1);
				Color = 0x00ffffff | (Alpha << 24);
				memcpy(
					(BYTE*)pBits + mappedSubrsrc.RowPitch * y + 4 * x,	// コピー先ブロック
					&Color,												// コピー元
					sizeof(DWORD));
			}
		}
		deviceContext->Unmap(pTex, 0);


		// ShaderResourceViewの情報を作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = rtDesc.MipLevels;

		// ShaderResourceViewの情報を書き込む
		ID3D11ShaderResourceView* srv;
		pDevice->CreateShaderResourceView(pTex, &srvDesc, &srv);

		g_Texture[0] = srv;

		// 頂点バッファ生成
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VERTEX_3D) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);
	}

	delete[] code;
}