//=============================================================================
//
// ���b�V���n�ʂ̏��� [meshfield.h]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#pragma once

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
/*
* @brief ���b�V���t�B�[���h����
* @param [in] pos ���S���W
* @param [in] rot ��]
* @param [in] nNumBlockX �u���b�N��
* @param [in] nNumBlockZ �u���b�N��
* @param [in] nBlockSizeX �u���b�N�T�C�Y
* @param [in] nBlockSizeZ �u���b�N�T�C�Y
*/
HRESULT InitMeshField(XMFLOAT3 pos, XMFLOAT3 rot,
							int nNumBlockX, int nNumBlockZ, float nBlockSizeX, float nBlockSizeZ);
void UninitMeshField(void);
void UpdateMeshField(void);
void DrawMeshField(void);

bool RayHitField(XMFLOAT3 pos, XMFLOAT3 *HitPosition, XMFLOAT3 *Normal);

float GetFieldSize(void);