//=============================================================================
//
// 当たり判定処理 [collision.h]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// 構造体定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
BOOL CollisionBB(XMFLOAT3 mpos, float mw, float mh, XMFLOAT3 ypos, float yw, float yh);
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2);

float dotProduct(XMVECTOR *v1, XMVECTOR *v2);
void crossProduct(XMVECTOR *ret, XMVECTOR *v1, XMVECTOR *v2);

/*
* @brief レイキャスト
* @param [in] p0, p1, p2 ポリゴンの3頂点
* @param [in] pos0 レイの始点
* @param [in] pos1 レイの終点
* @param [out] hit 交点
* @param [out] normal 法線ベクトル
* @return 当たっていればtrue
*/
bool RayCast(XMFLOAT3 p0, XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 pos0, XMFLOAT3 pos1, XMFLOAT3 *hit, XMFLOAT3 *normal);

/*
* @brief レイキャスト
* @param [in] p0, p1, p2, p3 ポリゴンの4頂点をZ型に指定
* @param [in] pos0 レイの始点
* @param [in] pos1 レイの終点
* @param [out] hit 交点
* @param [out] normal 法線ベクトル
* @return 当たっていればtrue
*/
bool RayCast(XMFLOAT3 p0, XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 p3, XMFLOAT3 pos0, XMFLOAT3 pos1, XMFLOAT3* hit, XMFLOAT3* normal);