//=============================================================================
//
// プレイヤー処理 [player.cpp]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "player.h"
#include "shadow.h"
#include "bullet.h"
#include "debugproc.h"
#include "meshfield.h"
#include "particle.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/snowball.obj"			// 読み込むモデル名

#define	VALUE_MOVE			(4.0f)							// 移動量（最大速度）
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define PLAYER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

#define PLAYER_PARTS_MAX	(2)								// プレイヤーのパーツの数

#define	TIME_TO_REACH_FULLSPEED		60.0f					// フルスピードに至るフレーム数
#define	INERTIA_START				1.0f					// 動き出す際の慣性
#define	INERTIA_END					4.0f					// 停止する際の慣性

#define	TIME_TO_TURN				30.0f					// 曲がりやすさ（曲がり切るに必要なフレーム数）
#define	TURN_SPEED					XM_PI * 0.2f			// 曲がる速度

#define	RATE_CHANGE			0.1f							// 膨張しやすさ

#define	ROLL_SPEED			0.2f * XM_PI					// 転がる速度

#define	SNOWFLAKE_OFFSET_Y	2.0f							// パーティクル発生源のY座標

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
bool CheckMove(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER		g_Player;

static float		roty = 0.0f;	// 回転用目標方向値

static LIGHT		g_Light;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{
	g_Player.load = TRUE;   
	LoadModel(MODEL_PLAYER, &g_Player.model);

	g_Player.pos = XMFLOAT3(-10.0f, PLAYER_OFFSET_Y, -50.0f);
	g_Player.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player.rate = 1.0f;
	g_Player.roll = 0.0f;

	g_Player.spd = 0.0f;			// 移動スピードクリア
	g_Player.moveTimer = 0.0f;
	g_Player.moveTimerPre = 0.0f;

	g_Player.use = TRUE;			// true:生きてる
	g_Player.collision = PLAYER_COLLISION;	// 当たり判定の大きさ

	// ここでプレイヤー用の影を作成している
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	g_Player.shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
	//          ↑
	//        このメンバー変数が生成した影のIndex番号

	// キーを押した時のプレイヤーの向き
	roty = 0.0f;

	g_Player.parent = NULL;			// 本体（親）なのでNULLを入れる

	// クォータニオンの初期化
	XMStoreFloat4(&g_Player.Quaternion, XMQuaternionIdentity());

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{
	// モデルの解放処理
	if (g_Player.load == TRUE)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{
	CAMERA *cam = GetCamera();




	// 移動処理
	bool isKeyDown = CheckMove();

	// 慣性タイマー処理
	if (isKeyDown) {
		// タイマー進行
		g_Player.moveTimer += INERTIA_START;
		if (g_Player.moveTimer > TIME_TO_REACH_FULLSPEED) {
			g_Player.moveTimer = TIME_TO_REACH_FULLSPEED;
		}
	}
	else {
		// タイマー減少
		g_Player.moveTimer -= INERTIA_END;
		if (g_Player.moveTimer < 0.0f) {
			g_Player.moveTimer = 0.0f;
		}
	}

	// 現在速度を補間
	g_Player.spd = (g_Player.moveTimer / TIME_TO_REACH_FULLSPEED) * VALUE_MOVE;


	// カメラの向きを考慮し向く方向を計算
	g_Player.rot.y = roty + cam->rot.y;

	// 移動
	g_Player.pos.x -= sinf(g_Player.rot.y) * g_Player.spd;
	g_Player.pos.z -= cosf(g_Player.rot.y) * g_Player.spd;

	// 境界判定
	{
		float fieldSizse = GetFieldSize();
		
		if (g_Player.pos.x > fieldSizse / 2) {
			g_Player.pos.x = fieldSizse / 2;
		}
		else if (g_Player.pos.x < -fieldSizse / 2) {
			g_Player.pos.x = -fieldSizse / 2;
		}

		if (g_Player.pos.z > fieldSizse / 2) {
			g_Player.pos.z = fieldSizse / 2;
		}
		else if (g_Player.pos.z < -fieldSizse / 2) {
			g_Player.pos.z = -fieldSizse / 2;
		}
	}
	

	// 転がり処理
	{
		// 現在速度と最大速度との差で転がり具合を計算
		float rate = g_Player.spd / VALUE_MOVE;

		// 転がり
		g_Player.roll += ROLL_SPEED * rate;
		
		// オーバーフロー防止
		if (g_Player.roll >= XM_PI) {
			g_Player.roll = 0.0f;
		}
	}

	// レイキャストして足元の高さを求める
	XMFLOAT3 HitPosition;		// 交点
	XMFLOAT3 Normal;			// ぶつかったポリゴンの法線ベクトル（向き）
	bool ans = RayHitField(g_Player.pos, &HitPosition, &Normal);

	float offsetY = PLAYER_OFFSET_Y * g_Player.rate;	// 膨張率に合わせて浮く
	if (ans)
	{
		g_Player.pos.y = HitPosition.y + offsetY;
	}
	else
	{
		g_Player.pos.y = offsetY;
		Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}


	//// 弾発射処理
	//if (GetKeyboardTrigger(DIK_SPACE))
	//{
	//	SetBullet(g_Player.pos, g_Player.rot);
	//}


	// 影もプレイヤーの位置に合わせる
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Player.shadowIdx, pos);

	// ポイントライトのテスト
	//{
	//	LIGHT *light = GetLightData(1);
	//	XMFLOAT3 pos = g_Player.pos;
	//	pos.y += 20.0f;

	//	light->Position = pos;
	//	light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//	light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//	light->Type = LIGHT_TYPE_POINT;
	//	light->Enable = TRUE;
	//	SetLightData(1, light);
	//}




	//////////////////////////////////////////////////////////////////////
	// 姿勢制御
	//////////////////////////////////////////////////////////////////////

	XMVECTOR vx, nvx, up;
	XMVECTOR quat;
	float len, angle;


	g_Player.UpVector = Normal;
	up = { 0.0f, 1.0f, 0.0f, 0.0f };
	vx = XMVector3Cross(up, XMLoadFloat3(&g_Player.UpVector));

	nvx = XMVector3Length(vx);
	XMFLOAT3 test;
	XMStoreFloat3(&test, nvx);
	XMStoreFloat(&len, nvx);
	nvx = XMVector3Normalize(vx);
	angle = asinf(len);	// 逆正弦（正弦値から角度を求める）

	//quat = XMQuaternionIdentity();
	//quat = XMQuaternionRotationAxis(nvx, angle);
	quat = XMQuaternionRotationNormal(nvx, angle);

	//球面線形補間を使用して、2つの単位四元数の間を補間する
	quat = XMQuaternionSlerp(XMLoadFloat4(&g_Player.Quaternion), quat, 0.05f);

	XMStoreFloat4(&g_Player.Quaternion, quat);






#ifdef _DEBUG
	PrintDebugProc("g_Player.rot.y = %f, ", g_Player.rot.y);
#endif

	 //パーティクル
	if (g_Player.spd > 0.1) {

		XMFLOAT3 pos;
		XMFLOAT3 scl;
		XMFLOAT3 move;
		XMFLOAT4 col;
		float rot;
		float length;	// 噴出する勢い
		int life;

		scl = XMFLOAT3(0.2f, 0.2f, 0.2f);

		// 方向はプレイヤー進行方向の反対側 +- 0.5ラジアン以内
		rot = g_Player.rot.y;
		{
			float lRand = (rand() % 100) / 100.0f - 0.5f;
			lRand *= XM_PI;

			rot += lRand;
		}

		// プレイヤーのかかとから噴出
		float offset = 5.0f;	// 調整用
		pos.x = g_Player.pos.x + sinf(rot) * offset;
		pos.y = SNOWFLAKE_OFFSET_Y;
		pos.z = g_Player.pos.z + cosf(rot) * offset;


#ifdef _DEBUG
		PrintDebugProc("rot = %f\n", rot);
#endif

		// 勢い
		length = rand() % 100 / 100.0f + 0.2f;

		move.x = sinf(rot) * length;
		move.y = 0.5f * XM_PI;					// 斜め上噴出
		move.z = cosf(rot) * length;

		col = XMFLOAT4(1.0f, 1.0f, 1.0f, rand() % 50 / 50.0f + 50.0f);

		life = 30;

		// ビルボードの設定
		SetParticle(pos, scl, move, XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), life);
	}



#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Player.pos.z = g_Player.pos.x = 0.0f;
		g_Player.spd = 0.0f;
		roty = 0.0f;
	}
#endif


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
{
	//return;
	XMMATRIX mtxScl, mtxRoll, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Player.scl.x * g_Player.rate, g_Player.scl.y * g_Player.rate, g_Player.scl.z * g_Player.rate);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 転がりを反映
	mtxRoll = XMMatrixRotationRollPitchYaw(g_Player.roll, 0.0f, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRoll);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// クォータニオンを反映
	// D3DXMatrixTransformation()は無駄な計算量が多いから不使用
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player.Quaternion));	// クォータニオンから変換行列を生成
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);


	// 縁取りの設定
	SetFuchi(1);

	// モデル描画
	DrawModel(&g_Player.model);


	SetFuchi(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
PLAYER *GetPlayer(void)
{
	return &g_Player;
}

//=============================================================================
// 移動チェック
//=============================================================================
bool CheckMove(void) {

	bool key = false;

	if (GetKeyboardPress(DIK_LEFT) || GetKeyboardPress(DIK_A) || IsButtonPressed(0, BUTTON_LEFT))
	{
		roty = XM_PI / 2;
		key = true;
	}
	if (GetKeyboardPress(DIK_RIGHT) || GetKeyboardPress(DIK_D) || IsButtonPressed(0, BUTTON_RIGHT))
	{
		roty = -XM_PI / 2;
		key = true;
	}
	if (GetKeyboardPress(DIK_UP) || GetKeyboardPress(DIK_W) || IsButtonPressed(0, BUTTON_UP))
	{
		roty = XM_PI;
		key = true;
	}
	if (GetKeyboardPress(DIK_DOWN) || GetKeyboardPress(DIK_S) || IsButtonPressed(0, BUTTON_DOWN))
	{
		roty = 0.0f;
		key = true;
	}

	return key;
}

//=============================================================================
// 膨張率変更
//=============================================================================
void AddRate(float parameter) {
	g_Player.rate += RATE_CHANGE * parameter;

	// 1以下にはならない
	if (g_Player.rate < 1.0f) {
		g_Player.rate = 1.0f;
	}
}