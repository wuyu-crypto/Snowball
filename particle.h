//=============================================================================
//
// �p�[�e�B�N������ [particle.h]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#pragma once

//*****************************************************************************
// �}�N����`
//*****************************************************************************
enum {
	PARTICLE_LABEL_SNOWFLAKE,

	PARTICLE_LABEL_NUM,
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitParticle(void);
void UninitParticle(void);
void UpdateParticle(void);
void DrawParticle(void);


#ifdef _DEBUG

/*
* @brief �p�[�e�B�N���̔�������
* �p�[�e�B�N���̔�������
* @param [in] pos �������W
* @param [in] move ���o����
* @param [in] col �F
* @param [in] fSizeX ���q�T�C�Y�i���ڎw��j
* @param [in] fSizeY ���q�T�C�Y�i���ڎw��j
* @param [in] nLife ����
*/
int SetParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);

#endif

/*
* @brief �p�[�e�B�N���̔�������
* �p�[�e�B�N���̔�������
* @param [in] pos �������W
* @param [in] scl �X�P�[��
* @param [in] move ���o����
* @param [in] col �F
* @param [in] nLife ����
*/
int SetParticle(XMFLOAT3 pos, XMFLOAT3 scl, XMFLOAT3 move, XMFLOAT4 col, int life);

void SetColorParticle(int nIdxParticle, XMFLOAT4 col);

