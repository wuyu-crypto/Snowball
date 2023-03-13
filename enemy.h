//=============================================================================
//
// エネミーモデル処理 [enemy.h]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_ENEMY			(16)					// エネミーの最大数

#define	ENEMY_COLLISION		(8.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct ENEMY
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	bool				use;
	bool				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				rate;				// サイズの比率

	float				spd;				// 移動スピード
	float				collision;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号
	
	INTERPOLATION_DATA	*tbl_adr;			// アニメデータのテーブル先頭アドレス
	int					tbl_size;			// 登録したテーブルのレコード総数
	float				move_time;			// 実行時間

};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
/*
* @brief エネミーの初期化
* エネミーを初期化する。InitMeshField()の後じゃないとバグる。
*/
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY *GetEnemy(void);

