//=============================================================================
//
// �����蔻�菈�� [collision.h]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************


//*****************************************************************************
// �\���̒�`
//*****************************************************************************


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
BOOL CollisionBB(XMFLOAT3 mpos, float mw, float mh, XMFLOAT3 ypos, float yw, float yh);
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2);

float dotProduct(XMVECTOR *v1, XMVECTOR *v2);
void crossProduct(XMVECTOR *ret, XMVECTOR *v1, XMVECTOR *v2);

/*
* @brief ���C�L���X�g
* @param [in] p0, p1, p2 �|���S����3���_
* @param [in] pos0 ���C�̎n�_
* @param [in] pos1 ���C�̏I�_
* @param [out] hit ��_
* @param [out] normal �@���x�N�g��
* @return �������Ă����true
*/
bool RayCast(XMFLOAT3 p0, XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 pos0, XMFLOAT3 pos1, XMFLOAT3 *hit, XMFLOAT3 *normal);

/*
* @brief ���C�L���X�g
* @param [in] p0, p1, p2, p3 �|���S����4���_��Z�^�Ɏw��
* @param [in] pos0 ���C�̎n�_
* @param [in] pos1 ���C�̏I�_
* @param [out] hit ��_
* @param [out] normal �@���x�N�g��
* @return �������Ă����true
*/
bool RayCast(XMFLOAT3 p0, XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 p3, XMFLOAT3 pos0, XMFLOAT3 pos1, XMFLOAT3* hit, XMFLOAT3* normal);