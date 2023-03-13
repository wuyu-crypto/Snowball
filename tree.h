//=============================================================================
//
// �؏��� [tree.h]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MAX_TREE			(64)			// �؍ő吔


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
typedef struct
{
	XMFLOAT3	pos;			// �ʒu
	XMFLOAT3	scl;			// �X�P�[��
	MATERIAL	material;		// �}�e���A��
	float		fWidth;			// ��
	float		fHeight;		// ����
	int			nIdxShadow;		// �eID
	bool		bUse;			// �g�p���Ă��邩�ǂ���

	float		collision;		// �����蔻��
	bool		isCrashed;		// ���i�q�b�g�h�~�t���O
	int			collisionCnt;	// ���i�q�b�g�^�C�}�[

} TREE;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitTree(void);
void UninitTree(void);
void UpdateTree(void);
void DrawTree(void);

/*
* @brief �؂��Z�b�g
* ���g�p�؃X���b�g����1�{�I��ŗL����
* @param [in] pos ���W
* @param [in] fWidth ��
* @param [in] fHeight ����
* @param [in] col �F
*/
int SetTree(XMFLOAT3 pos, float fWidth, float fHeight, XMFLOAT4 col);

/*
* @brief �؃X���b�g���擾
*/
TREE* GetTree(void);