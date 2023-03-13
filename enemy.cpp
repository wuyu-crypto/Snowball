//=============================================================================
//
// エネミーモデル処理 [enemy.cpp]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "model.h"
#include "enemy.h"
#include "shadow.h"
#include "meshfield.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/snowball.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define ENEMY_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define ENEMY_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる

#define	SCL_MAX				60							// エネミーのスケールのブレ幅(%)

#define	SIZE_MAX			20							// 雪玉のサイズのブレ幅+-

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// エネミー

static BOOL				g_Load = FALSE;


static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(   0.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*1 },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*0.5f },
	{ XMFLOAT3(   0.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },

};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{	
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);

		g_Enemy[i].load = true;

		// サイズ決め
		float rate = (rand() % SIZE_MAX) / SIZE_MAX;
		if (rand() % 2 == 0) {
			rate = -rate;
		}
		rate += 1.0f;

		g_Enemy[i].scl = XMFLOAT3(rate, rate, rate);

		{
			int fieldSize = (int)GetFieldSize();	// フィールドの大きさ

			g_Enemy[i].pos = XMFLOAT3(rand() % fieldSize - fieldSize / 2, ENEMY_OFFSET_Y * rate, rand() % fieldSize - fieldSize / 2);
		}

		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		

		g_Enemy[i].spd = 0.0f;					// 移動スピードクリア
		g_Enemy[i].collision = ENEMY_COLLISION * rate;	// 当たり判定の大きさ

		g_Enemy[i].rate = rate;

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);

		g_Enemy[i].move_time = 0.0f;	// 線形補間用のタイマーをクリア
		g_Enemy[i].tbl_adr = NULL;		// 再生するアニメデータの先頭アドレスをセット
		g_Enemy[i].tbl_size = 0;		// 再生するアニメデータのレコード数をセット

		g_Enemy[i].use = true;			// true:生きてる

	}


	//// 0番だけ線形補間で動かしてみる
	//g_Enemy[0].move_time = 0.0f;		// 線形補間用のタイマーをクリア
	//g_Enemy[0].tbl_adr = move_tbl;		// 再生するアニメデータの先頭アドレスをセット
	//g_Enemy[0].tbl_size = sizeof(move_tbl) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = false;
		}
	}

	g_Load = FALSE;

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	//// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	//for (int i = 0; i < MAX_ENEMY; i++)
	//{
	//	if (g_Enemy[i].use == true)			// このエネミーが使われている？
	//	{									// Yes
	//		if (g_Enemy[i].tbl_adr != NULL)	// 線形補間を実行する？
	//		{								// 線形補間の処理
	//			// 移動処理
	//			int		index = (int)g_Enemy[i].move_time;
	//			float	time = g_Enemy[i].move_time - index;
	//			int		collision = g_Enemy[i].tbl_size;

	//			float dt = 1.0f / g_Enemy[i].tbl_adr[index].frame;	// 1フレームで進める時間
	//			g_Enemy[i].move_time += dt;							// アニメーションの合計時間に足す

	//			if (index > (collision - 2))	// ゴールをオーバーしていたら、最初へ戻す
	//			{
	//				g_Enemy[i].move_time = 0.0f;
	//				index = 0;
	//			}

	//			// 座標を求める	X = StartX + (EndX - StartX) * 今の時間
	//			XMVECTOR p1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].pos);	// 次の場所
	//			XMVECTOR p0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].pos);	// 現在の場所
	//			XMVECTOR vec = p1 - p0;
	//			XMStoreFloat3(&g_Enemy[i].pos, p0 + vec * time);

	//			// 回転を求める	R = StartX + (EndX - StartX) * 今の時間
	//			XMVECTOR r1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].rot);	// 次の角度
	//			XMVECTOR r0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].rot);	// 現在の角度
	//			XMVECTOR rot = r1 - r0;
	//			XMStoreFloat3(&g_Enemy[i].rot, r0 + rot * time);

	//			// scaleを求める S = StartX + (EndX - StartX) * 今の時間
	//			XMVECTOR s1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].scl);	// 次のScale
	//			XMVECTOR s0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].scl);	// 現在のScale
	//			XMVECTOR scl = s1 - s0;
	//			XMStoreFloat3(&g_Enemy[i].scl, s0 + scl * time);

	//		}

	//		
	//		// レイキャストして足元の高さを求める
	//		XMFLOAT3 HitPosition;		// 交点
	//		XMFLOAT3 Normal;			// ぶつかったポリゴンの法線ベクトル（向き）
	//		bool ans = RayHitField(g_Enemy[i].pos, &HitPosition, &Normal);
	//		if (ans)
	//		{
	//			g_Enemy[i].pos.y = HitPosition.y + ENEMY_OFFSET_Y;
	//		}
	//		else
	//		{
	//			g_Enemy[i].pos.y = ENEMY_OFFSET_Y;
	//			Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//		}


	//		// 影もエネミーの位置に合わせる
	//		XMFLOAT3 pos = g_Enemy[i].pos;
	//		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
	//		SetPositionShadow(g_Enemy[i].shadowIdx, pos);
	//	}
	//}

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// 縁取りの設定
	SetFuchi(1);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == false) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);



		// モデル描画
		DrawModel(&g_Enemy[i].model);
	}

	// 縁取りの設定
	SetFuchi(1);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}
