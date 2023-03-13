//=============================================================================
//
// 木処理 [tree.cpp]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "shadow.h"
#include "meshfield.h"
#include "tree.h"
#include "player.h"
#include "collision.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX			(1)				// テクスチャの数

#define	TREE_WIDTH			(50.0f)			// 頂点サイズ
#define	TREE_HEIGHT			(80.0f)			// 頂点サイズ

#define	OFFSET_Y			-2.0f			// 埋まり具合

#define	COLLISION			8.0f			// 当たり判定

#define	COLLISOIN_TIME		60				// 多段ヒットの間隔


#define	TRANSPARENCY_BY_RAYCAST
#undef TRANSPARENCY_BY_RAYCAST

#ifdef TRANSPARENCY_BY_RAYCAST

#define	TRANSPARENCY_BY_RAYCAST_OFFSET	1000.0f		// 菱型のサイズ

#else

#define	HIDE_DEPTH			50.0f			// 隠れる深度

#endif

#define	OPACITY				0.8f			// 不透明度

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexTree(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;	// 頂点バッファ
static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static TREE					g_aTree[MAX_TREE];	// 木ワーク
static int					g_TexNo;			// テクスチャ番号
static bool					g_bAlpaTest;		// アルファテストON/OFF

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/tree.png",
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTree(void)
{
	MakeVertexTree();

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	g_TexNo = 0;

	// 木ワークの初期化
	for(int nCntTree = 0; nCntTree < MAX_TREE; nCntTree++)
	{
		ZeroMemory(&g_aTree[nCntTree].material, sizeof(g_aTree[nCntTree].material));
		g_aTree[nCntTree].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aTree[nCntTree].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aTree[nCntTree].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_aTree[nCntTree].fWidth = TREE_WIDTH;
		g_aTree[nCntTree].fHeight = TREE_HEIGHT;
		g_aTree[nCntTree].bUse = false;

		g_aTree[nCntTree].collision = COLLISION;
		g_aTree[nCntTree].isCrashed = false;
		g_aTree[nCntTree].collisionCnt = 0;
	}

	g_bAlpaTest = true;

	// 木の有効化
	int fieldSize = (int)GetFieldSize();	// フィールドの大きさ
	for (int i = 0; i < MAX_TREE; i++) {
		float fX = (rand() % fieldSize - fieldSize / 2) * 0.9f;		// 壁埋まり対策
		float fZ = (rand() % fieldSize - fieldSize / 2) * 0.9f;		// 壁埋まり対策

		SetTree(XMFLOAT3(fX, OFFSET_Y, fZ), 160.0f, 160.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTree(void)
{
	for(int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if(g_Texture[nCntTex] != NULL)
		{// テクスチャの解放
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	if(g_VertexBuffer != NULL)
	{// 頂点バッファの解放
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTree(void)
{

	for(int i = 0; i < MAX_TREE; i++)
	{
		if(g_aTree[i].bUse)
		{
			// 多段ヒットタイマー処理
			if (g_aTree[i].isCrashed) {
				g_aTree[i].collisionCnt++;

				if (g_aTree[i].collisionCnt > COLLISOIN_TIME) {
					g_aTree[i].isCrashed = false;
					g_aTree[i].collisionCnt = 0;
				}
			}


#ifndef TRANSPARENCY_BY_RAYCAST

			// 深度で透視処理
			{
				float depth = ViewDepth(g_aTree[i].pos);
				if (depth < HIDE_DEPTH) {
					// 近すぎたら半透明にする
					g_aTree[i].material.Diffuse.w = 0.8f;
				}
				else {
					// 戻す
					g_aTree[i].material.Diffuse.w = 1.0f;
				}
			}

#endif


			// 影の位置設定
			SetPositionShadow(g_aTree[i].nIdxShadow, XMFLOAT3(g_aTree[i].pos.x, 0.1f, g_aTree[i].pos.z));
		}
	}


#ifdef _DEBUG
	// アルファテストON/OFF
	if(GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? false: true;
	}

	//// アルファテストの閾値変更
	//if(GetKeyboardPress(DIK_I))
	//{
	//	g_nAlpha--;
	//	if(g_nAlpha < 0)
	//	{
	//		g_nAlpha = 0;
	//	}
	//}
	//if(GetKeyboardPress(DIK_K))
	//{
	//	g_nAlpha++;
	//	if(g_nAlpha > 255)
	//	{
	//		g_nAlpha = 255;
	//	}
	//}
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTree(void)
{
	// αテスト設定
	if (g_bAlpaTest == true)
	{
		// αテストを有効に
		SetAlphaTestEnable(true);
	}

	// ライティングを無効
	SetLightEnable(false);

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for(int i = 0; i < MAX_TREE; i++)
	{
		if(g_aTree[i].bUse)
		{
			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// ビューマトリックスを取得
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			//mtxWorld = XMMatrixInverse(nullptr, mtxView);
			//mtxWorld.r[3].m128_f32[0] = 0.0f;
			//mtxWorld.r[3].m128_f32[1] = 0.0f;
			//mtxWorld.r[3].m128_f32[2] = 0.0f;

			// カメラに向かせる処理
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

			// スケールを反映
			mtxScl = XMMatrixScaling(g_aTree[i].scl.x, g_aTree[i].scl.y, g_aTree[i].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aTree[i].pos.x, g_aTree[i].pos.y, g_aTree[i].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

#ifdef TRANSPARENCY_BY_RAYCAST

			// レイキャストで半透明処理
			{
				XMMATRIX mtxTree = mtxWorld;	// 木のビュー座標の入ったマトリックス
				XMMATRIX mtxCamPos = XMMatrixIdentity();	// カメラの位置のビュー座標の入ったマトリックス
				XMMATRIX mtxCamAt = XMLoadFloat4x4(&cam->mtxInvView);	// カメラの注視点のビュー座標の入ったマトリックス

				// マトリックスから座標情報を抽出
				XMFLOAT3 tree = GetTranslationFromXMMATRIX(mtxTree);
				XMFLOAT3 camPos = GetTranslationFromXMMATRIX(mtxCamPos);
				XMFLOAT3 camAt = GetTranslationFromXMMATRIX(mtxCamAt);

				XMFLOAT3 p0, p1, p2, p3, pos0, pos1, hit, normal;

				// ◇を描く
				p0 = XMFLOAT3(tree.x,									tree.y + TRANSPARENCY_BY_RAYCAST_OFFSET,	tree.z);	// 上
				p1 = XMFLOAT3(tree.x + TRANSPARENCY_BY_RAYCAST_OFFSET,	tree.y,										tree.z);	// 右
				p2 = XMFLOAT3(tree.x - TRANSPARENCY_BY_RAYCAST_OFFSET,	tree.y,										tree.z);	// 左
				p3 = XMFLOAT3(tree.x,									tree.y - TRANSPARENCY_BY_RAYCAST_OFFSET,	tree.z);	// 下

				pos0 = camPos;
				pos1 = camAt;

				if (RayCast(p0, p1, p2, p3, pos0, pos1, &hit, &normal)) {

					// レイがヒットしたら木を半透明にする
					g_aTree[i].material.Diffuse.w = OPACITY;
				}
			}

#endif

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);


			// マテリアル設定
			SetMaterial(g_aTree[i].material);

			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ライティングを有効に
	SetLightEnable(true);

	// αテストを無効に
	SetAlphaTestEnable(false);
}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexTree(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// 頂点バッファに値をセットする
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = 60.0f;
	float fHeight = 90.0f;

	// 頂点座標の設定
	vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

	// 拡散光の設定
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャ座標の設定
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}

int SetTree(XMFLOAT3 pos, float fWidth, float fHeight, XMFLOAT4 col)
{
	int nIdxTree = -1;

	for(int nCntTree = 0; nCntTree < MAX_TREE; nCntTree++)
	{
		if(!g_aTree[nCntTree].bUse)
		{
			g_aTree[nCntTree].pos = pos;
			g_aTree[nCntTree].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
			g_aTree[nCntTree].fWidth = fWidth;
			g_aTree[nCntTree].fHeight = fHeight;
			g_aTree[nCntTree].bUse = true;

			// 影の設定
			g_aTree[nCntTree].nIdxShadow = CreateShadow(g_aTree[nCntTree].pos, 0.5f, 0.5f);

			nIdxTree = nCntTree;

			// 1本セットしたら終了
			break;
		}
	}

	return nIdxTree;
}

TREE* GetTree(void) {
	return &g_aTree[0];
}

