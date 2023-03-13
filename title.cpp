//=============================================================================
//
// タイトル画面処理 [title.cpp]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "title.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 

#define TEXTURE_WIDTH_LOGO			(480)			// ロゴサイズ
#define TEXTURE_HEIGHT_LOGO			(80)			// 

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TITLE_TEXTURE_NUM] = { NULL };	// テクスチャ情報

static char *g_TextureName[TITLE_TEXTURE_NUM] = {
	"data/TEXTURE/title00.jpg",
	"data/TEXTURE/title01.jpg",
	"data/TEXTURE/title02.jpg",
	"data/TEXTURE/pressAnyKey.png",
};


static bool						g_Use;						// true:使っている  false:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標


static BOOL						g_Load = FALSE;


static XMFLOAT3					g_PressAnyKeyPos;			// PRESSANYKEYの座標
static float					alpha;
static bool						flag_alpha;

static int						g_TitleBgNum;				// 使うタイトル背景の番号

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTitle(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TITLE_TEXTURE_NUM; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// 背景の初期化
	g_Use   = true;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = XMFLOAT3(g_w/2, g_h/2, 0.0f);

	// 背景をランダムに決める
	g_TitleBgNum = rand() % (TITLE_TEXTURE_BG02 + 1);



	// PRESSANYKEYの初期化
	g_PressAnyKeyPos = XMFLOAT3(0.5f * SCREEN_WIDTH, 0.9f * SCREEN_HEIGHT, 0.0f);
	alpha = 1.0f;
	flag_alpha = true;


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTitle(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TITLE_TEXTURE_NUM; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTitle(void)
{
	// シーン遷移
	if (GetKeyboardTrigger() || IsButtonTriggered(0)) {
		PlaySound(SOUND_LABEL_SE_CONFIRM, 1.0f);
		SetFade(FADE_OUT, MODE_TUTORIAL);
	}


	if (flag_alpha == true)
	{
		alpha -= 0.02f;
		if (alpha <= 0.0f)
		{
			alpha = 0.0f;
			flag_alpha = false;
		}
	}
	else
	{
		alpha += 0.02f;
		if (alpha >= 1.0f)
		{
			alpha = 1.0f;
			flag_alpha = true;
		}
	}






#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTitle(void)
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

	// タイトルの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TitleBgNum]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// PRESSANYKEY
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TITLE_TEXTURE_PRESSANYKEY]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
	//	SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f);
		SetSpriteColor(g_VertexBuffer, g_PressAnyKeyPos.x, g_PressAnyKeyPos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f,
						XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

//	// 加減算のテスト
//	SetBlendState(BLEND_MODE_ADD);		// 加算合成
////	SetBlendState(BLEND_MODE_SUBTRACT);	// 減算合成
//	for(int i=0; i<30; i++)
//	{
//		// テクスチャ設定
//		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
//
//		// １枚のポリゴンの頂点とテクスチャ座標を設定
//		float dx = 100.0f;
//		float dy = 100.0f;
//		float sx = (float)(rand() % 100);
//		float sy = (float)(rand() % 100);
//
//
//		SetSpriteColor(g_VertexBuffer, dx+sx, dy+sy, 50, 50, 0.0f, 0.0f, 1.0f, 1.0f,
//			XMFLOAT4(0.3f, 0.3f, 1.0f, 0.5f));
//
//		// ポリゴン描画
//		GetDeviceContext()->Draw(4, 0);
//	}
//	SetBlendState(BLEND_MODE_ALPHABLEND);	// 半透明処理を元に戻す

}


//=============================================================================
// タイトルテクスチャファイル名を取得
//=============================================================================
int GetTitleBgNum(void) {
	return g_TitleBgNum;
}
