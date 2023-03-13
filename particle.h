//=============================================================================
//
// パーティクル処理 [particle.h]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#pragma once

//*****************************************************************************
// マクロ定義
//*****************************************************************************
enum {
	PARTICLE_LABEL_SNOWFLAKE,

	PARTICLE_LABEL_NUM,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitParticle(void);
void UninitParticle(void);
void UpdateParticle(void);
void DrawParticle(void);


#ifdef _DEBUG

/*
* @brief パーティクルの発生処理
* パーティクルの発生処理
* @param [in] pos 発生座標
* @param [in] move 噴出方向
* @param [in] col 色
* @param [in] fSizeX 粒子サイズ（直接指定）
* @param [in] fSizeY 粒子サイズ（直接指定）
* @param [in] nLife 寿命
*/
int SetParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);

#endif

/*
* @brief パーティクルの発生処理
* パーティクルの発生処理
* @param [in] pos 発生座標
* @param [in] scl スケール
* @param [in] move 噴出方向
* @param [in] col 色
* @param [in] nLife 寿命
*/
int SetParticle(XMFLOAT3 pos, XMFLOAT3 scl, XMFLOAT3 move, XMFLOAT4 col, int life);

void SetColorParticle(int nIdxParticle, XMFLOAT4 col);

