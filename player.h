//=============================================================================
//
// モデル処理 [player.h]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#pragma once
#include "model.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_PLAYER		(1)					// プレイヤーの数

#define	PLAYER_COLLISION		(8.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct PLAYER
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	float				spd;				// 移動スピード
	float				moveTimer;			// 移動タイマー
	float				moveTimerPre;		// 方向転換前のタイマー値
	
	float				rate;				// 雪玉の膨張率
	float				roll;				// 転がりメーター

	BOOL				load;
	DX11_MODEL			model;				// モデル情報

	int					shadowIdx;			// 影のインデックス番号

	BOOL				use;

	float				collision;

	// 階層アニメーション用のメンバー変数
	INTERPOLATION_DATA	*tbl_adr;			// アニメデータのテーブル先頭アドレス
	float				move_time;			// 実行時間

	// 親は、NULL、子供は親のアドレスを入れる
	PLAYER				*parent;			// 自分が親ならNULL、自分が子供なら親のplayerアドレス

	// クォータニオン
	XMFLOAT4			Quaternion;

	XMFLOAT3			UpVector;			// 自分が立っている所

};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER *GetPlayer(void);

/*
* @brief 膨張率変更
* @param [in] parameter 倍率
*/
void AddRate(float parameter);