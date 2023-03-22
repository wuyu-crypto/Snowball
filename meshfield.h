//=============================================================================
//
// メッシュ地面の処理 [meshfield.h]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#pragma once

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
/*
* @brief メッシュフィールド生成
* @param [in] pos 中心座標
* @param [in] rot 回転
* @param [in] nNumBlockX ブロック数
* @param [in] nNumBlockZ ブロック数
* @param [in] nBlockSizeX ブロックサイズ
* @param [in] nBlockSizeZ ブロックサイズ
*/
HRESULT InitMeshField(XMFLOAT3 pos, XMFLOAT3 rot,
							int nNumBlockX, int nNumBlockZ, float nBlockSizeX, float nBlockSizeZ);
void UninitMeshField(void);
void UpdateMeshField(void);
void DrawMeshField(void);

bool RayHitField(XMFLOAT3 pos, XMFLOAT3 *HitPosition, XMFLOAT3 *Normal);

float GetFieldSize(void);