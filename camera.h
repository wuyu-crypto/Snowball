//=============================================================================
//
// カメラ処理 [camera.h]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#pragma once


//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
struct CAMERA
{
	XMFLOAT4X4			mtxView;		// ビューマトリックス
	XMFLOAT4X4			mtxInvView;		// ビューマトリックス
	XMFLOAT4X4			mtxProjection;	// プロジェクションマトリックス
	
	XMFLOAT3			pos;			// カメラの視点(位置)
	XMFLOAT3			at;				// カメラの注視点
	XMFLOAT3			up;				// カメラの上方向ベクトル
	XMFLOAT3			rot;			// カメラの回転
	
	float				len;			// カメラの視点と注視点の距離

};


enum {
	TYPE_FULL_SCREEN,
	TYPE_LEFT_HALF_SCREEN,
	TYPE_RIGHT_HALF_SCREEN,
	TYPE_UP_HALF_SCREEN,
	TYPE_DOWN_HALF_SCREEN,
	TYPE_NONE,

};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void InitCamera(void);
void UninitCamera(void);
void UpdateCamera(void);
void SetCamera(void);

CAMERA *GetCamera(void);

void SetViewPort(int type);
int GetViewPortType(void);

void SetCameraAT(XMFLOAT3 pos);

/*
* @brief 視点からの深度
* 視線を沿って目標位置からカメラまでの距離を計算。カメラと近すぎた物体を半透明にする時などに使う
* @param [in] pos 目標位置
*/
float ViewDepth(XMFLOAT3 pos);

/*
* @brief マトリックスから移動座標を取得
* @param [in] mtx 対象マトリックス
*/
XMFLOAT3 GetTranslationFromXMMATRIX(XMMATRIX mtx);