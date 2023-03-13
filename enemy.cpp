//=============================================================================
//
// �G�l�~�[���f������ [enemy.cpp]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "model.h"
#include "enemy.h"
#include "shadow.h"
#include "meshfield.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/snowball.obj"		// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(5.0f)						// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// ��]��

#define ENEMY_SHADOW_SIZE	(0.4f)						// �e�̑傫��
#define ENEMY_OFFSET_Y		(7.0f)						// �G�l�~�[�̑��������킹��

#define	SCL_MAX				60							// �G�l�~�[�̃X�P�[���̃u����(%)

#define	SIZE_MAX			20							// ��ʂ̃T�C�Y�̃u����+-

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// �G�l�~�[

static BOOL				g_Load = FALSE;


static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(   0.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*1 },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*0.5f },
	{ XMFLOAT3(   0.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },

};

//=============================================================================
// ����������
//=============================================================================
HRESULT InitEnemy(void)
{	
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);

		g_Enemy[i].load = true;

		// �T�C�Y����
		float rate = (rand() % SIZE_MAX) / SIZE_MAX;
		if (rand() % 2 == 0) {
			rate = -rate;
		}
		rate += 1.0f;

		g_Enemy[i].scl = XMFLOAT3(rate, rate, rate);

		{
			int fieldSize = (int)GetFieldSize();	// �t�B�[���h�̑傫��

			g_Enemy[i].pos = XMFLOAT3(rand() % fieldSize - fieldSize / 2, ENEMY_OFFSET_Y * rate, rand() % fieldSize - fieldSize / 2);
		}

		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		

		g_Enemy[i].spd = 0.0f;					// �ړ��X�s�[�h�N���A
		g_Enemy[i].collision = ENEMY_COLLISION * rate;	// �����蔻��̑傫��

		g_Enemy[i].rate = rate;

		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);

		g_Enemy[i].move_time = 0.0f;	// ���`��ԗp�̃^�C�}�[���N���A
		g_Enemy[i].tbl_adr = NULL;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Enemy[i].tbl_size = 0;		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

		g_Enemy[i].use = true;			// true:�����Ă�

	}


	//// 0�Ԃ������`��Ԃœ������Ă݂�
	//g_Enemy[0].move_time = 0.0f;		// ���`��ԗp�̃^�C�}�[���N���A
	//g_Enemy[0].tbl_adr = move_tbl;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	//g_Enemy[0].tbl_size = sizeof(move_tbl) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = false;
		}
	}

	g_Load = FALSE;

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
	//// �G�l�~�[�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	//for (int i = 0; i < MAX_ENEMY; i++)
	//{
	//	if (g_Enemy[i].use == true)			// ���̃G�l�~�[���g���Ă���H
	//	{									// Yes
	//		if (g_Enemy[i].tbl_adr != NULL)	// ���`��Ԃ����s����H
	//		{								// ���`��Ԃ̏���
	//			// �ړ�����
	//			int		index = (int)g_Enemy[i].move_time;
	//			float	time = g_Enemy[i].move_time - index;
	//			int		collision = g_Enemy[i].tbl_size;

	//			float dt = 1.0f / g_Enemy[i].tbl_adr[index].frame;	// 1�t���[���Ői�߂鎞��
	//			g_Enemy[i].move_time += dt;							// �A�j���[�V�����̍��v���Ԃɑ���

	//			if (index > (collision - 2))	// �S�[�����I�[�o�[���Ă�����A�ŏ��֖߂�
	//			{
	//				g_Enemy[i].move_time = 0.0f;
	//				index = 0;
	//			}

	//			// ���W�����߂�	X = StartX + (EndX - StartX) * ���̎���
	//			XMVECTOR p1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].pos);	// ���̏ꏊ
	//			XMVECTOR p0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].pos);	// ���݂̏ꏊ
	//			XMVECTOR vec = p1 - p0;
	//			XMStoreFloat3(&g_Enemy[i].pos, p0 + vec * time);

	//			// ��]�����߂�	R = StartX + (EndX - StartX) * ���̎���
	//			XMVECTOR r1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].rot);	// ���̊p�x
	//			XMVECTOR r0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].rot);	// ���݂̊p�x
	//			XMVECTOR rot = r1 - r0;
	//			XMStoreFloat3(&g_Enemy[i].rot, r0 + rot * time);

	//			// scale�����߂� S = StartX + (EndX - StartX) * ���̎���
	//			XMVECTOR s1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].scl);	// ����Scale
	//			XMVECTOR s0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].scl);	// ���݂�Scale
	//			XMVECTOR scl = s1 - s0;
	//			XMStoreFloat3(&g_Enemy[i].scl, s0 + scl * time);

	//		}

	//		
	//		// ���C�L���X�g���đ����̍��������߂�
	//		XMFLOAT3 HitPosition;		// ��_
	//		XMFLOAT3 Normal;			// �Ԃ������|���S���̖@���x�N�g���i�����j
	//		bool ans = RayHitField(g_Enemy[i].pos, &HitPosition, &Normal);
	//		if (ans)
	//		{
	//			g_Enemy[i].pos.y = HitPosition.y + ENEMY_OFFSET_Y;
	//		}
	//		else
	//		{
	//			g_Enemy[i].pos.y = ENEMY_OFFSET_Y;
	//			Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//		}


	//		// �e���G�l�~�[�̈ʒu�ɍ��킹��
	//		XMFLOAT3 pos = g_Enemy[i].pos;
	//		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
	//		SetPositionShadow(g_Enemy[i].shadowIdx, pos);
	//	}
	//}

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// �����̐ݒ�
	SetFuchi(1);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == false) continue;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);



		// ���f���`��
		DrawModel(&g_Enemy[i].model);
	}

	// �����̐ݒ�
	SetFuchi(1);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// �G�l�~�[�̎擾
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}
