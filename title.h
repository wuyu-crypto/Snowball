//=============================================================================
//
// �^�C�g����ʏ��� [title.h]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
enum {
	TITLE_TEXTURE_BG00,			// �w�i00
	TITLE_TEXTURE_BG01,			// �w�i01
	TITLE_TEXTURE_BG02,			// �w�i02

	TITLE_TEXTURE_PRESSANYKEY,	// PRESSANYKEY

	TITLE_TEXTURE_NUM,
};



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitTitle(void);
void UninitTitle(void);
void UpdateTitle(void);
void DrawTitle(void);


int GetTitleBgNum(void);