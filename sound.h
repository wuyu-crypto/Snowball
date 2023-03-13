//=============================================================================
//
// サウンド処理 [sound.h]
// Author : HAL東京　ゲーム学科　呉優
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// サウンド処理で必要

//*****************************************************************************
// サウンドファイル
//*****************************************************************************
enum 
{
	SOUND_LABEL_BGM_TITLE,
	SOUND_LABEL_BGM_TUTORIAL,
	SOUND_LABEL_BGM_GAME,
	SOUND_LABEL_BGM_RESULT,

	SOUND_LABEL_SE_CONFIRM,
	SOUND_LABEL_SE_ABSORB,
	SOUND_LABEL_SE_CRASH00,
	SOUND_LABEL_SE_CRASH01,

	SOUND_LABEL_NUM,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
bool InitSound(HWND hWnd);
void UninitSound(void);

/*
* @brief 音楽をデフォルト音量で再生
* セグメント再生(再生中なら停止)
* @param [in] label サウンドラベル
*/
void PlaySound(int label);

/*
* @brief 音楽を再生
* セグメント再生(再生中なら停止)
* @param [in] label サウンドラベル
* @param [in] volume 音量
*/
void PlaySound(int label, float volume);

void StopSound(int label);
void StopSound(void);

