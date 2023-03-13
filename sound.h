//=============================================================================
//
// �T�E���h���� [sound.h]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// �T�E���h�����ŕK�v

//*****************************************************************************
// �T�E���h�t�@�C��
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
// �v���g�^�C�v�錾
//*****************************************************************************
bool InitSound(HWND hWnd);
void UninitSound(void);

/*
* @brief ���y���f�t�H���g���ʂōĐ�
* �Z�O�����g�Đ�(�Đ����Ȃ��~)
* @param [in] label �T�E���h���x��
*/
void PlaySound(int label);

/*
* @brief ���y���Đ�
* �Z�O�����g�Đ�(�Đ����Ȃ��~)
* @param [in] label �T�E���h���x��
* @param [in] volume ����
*/
void PlaySound(int label, float volume);

void StopSound(int label);
void StopSound(void);

