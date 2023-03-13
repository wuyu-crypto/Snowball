//=============================================================================
//
// 木処理 [tree.h]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MAX_TREE			(64)			// 木最大数


//*****************************************************************************
// 構造体定義
//*****************************************************************************
typedef struct
{
	XMFLOAT3	pos;			// 位置
	XMFLOAT3	scl;			// スケール
	MATERIAL	material;		// マテリアル
	float		fWidth;			// 幅
	float		fHeight;		// 高さ
	int			nIdxShadow;		// 影ID
	bool		bUse;			// 使用しているかどうか

	float		collision;		// 当たり判定
	bool		isCrashed;		// 多段ヒット防止フラグ
	int			collisionCnt;	// 多段ヒットタイマー

} TREE;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitTree(void);
void UninitTree(void);
void UpdateTree(void);
void DrawTree(void);

/*
* @brief 木をセット
* 未使用木スロットから1本選んで有効化
* @param [in] pos 座標
* @param [in] fWidth 幅
* @param [in] fHeight 高さ
* @param [in] col 色
*/
int SetTree(XMFLOAT3 pos, float fWidth, float fHeight, XMFLOAT4 col);

/*
* @brief 木スロットを取得
*/
TREE* GetTree(void);