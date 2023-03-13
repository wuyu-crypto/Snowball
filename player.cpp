//=============================================================================
//
// �v���C���[���� [player.cpp]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "player.h"
#include "shadow.h"
#include "bullet.h"
#include "debugproc.h"
#include "meshfield.h"
#include "particle.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/snowball.obj"			// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(4.0f)							// �ړ��ʁi�ő呬�x�j
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// ��]��

#define PLAYER_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define PLAYER_OFFSET_Y		(7.0f)							// �v���C���[�̑��������킹��

#define PLAYER_PARTS_MAX	(2)								// �v���C���[�̃p�[�c�̐�

#define	TIME_TO_REACH_FULLSPEED		60.0f					// �t���X�s�[�h�Ɏ���t���[����
#define	INERTIA_START				1.0f					// �����o���ۂ̊���
#define	INERTIA_END					4.0f					// ��~����ۂ̊���

#define	TIME_TO_TURN				30.0f					// �Ȃ���₷���i�Ȃ���؂�ɕK�v�ȃt���[�����j
#define	TURN_SPEED					XM_PI * 0.2f			// �Ȃ��鑬�x

#define	RATE_CHANGE			0.1f							// �c�����₷��

#define	ROLL_SPEED			0.2f * XM_PI					// �]���鑬�x

#define	SNOWFLAKE_OFFSET_Y	2.0f							// �p�[�e�B�N����������Y���W

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
bool CheckMove(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static PLAYER		g_Player;

static float		roty = 0.0f;	// ��]�p�ڕW�����l

static LIGHT		g_Light;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayer(void)
{
	g_Player.load = TRUE;   
	LoadModel(MODEL_PLAYER, &g_Player.model);

	g_Player.pos = XMFLOAT3(-10.0f, PLAYER_OFFSET_Y, -50.0f);
	g_Player.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player.rate = 1.0f;
	g_Player.roll = 0.0f;

	g_Player.spd = 0.0f;			// �ړ��X�s�[�h�N���A
	g_Player.moveTimer = 0.0f;
	g_Player.moveTimerPre = 0.0f;

	g_Player.use = TRUE;			// true:�����Ă�
	g_Player.collision = PLAYER_COLLISION;	// �����蔻��̑傫��

	// �����Ńv���C���[�p�̉e���쐬���Ă���
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	g_Player.shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
	//          ��
	//        ���̃����o�[�ϐ������������e��Index�ԍ�

	// �L�[�����������̃v���C���[�̌���
	roty = 0.0f;

	g_Player.parent = NULL;			// �{�́i�e�j�Ȃ̂�NULL������

	// �N�H�[�^�j�I���̏�����
	XMStoreFloat4(&g_Player.Quaternion, XMQuaternionIdentity());

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPlayer(void)
{
	// ���f���̉������
	if (g_Player.load == TRUE)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdatePlayer(void)
{
	CAMERA *cam = GetCamera();




	// �ړ�����
	bool isKeyDown = CheckMove();

	// �����^�C�}�[����
	if (isKeyDown) {
		// �^�C�}�[�i�s
		g_Player.moveTimer += INERTIA_START;
		if (g_Player.moveTimer > TIME_TO_REACH_FULLSPEED) {
			g_Player.moveTimer = TIME_TO_REACH_FULLSPEED;
		}
	}
	else {
		// �^�C�}�[����
		g_Player.moveTimer -= INERTIA_END;
		if (g_Player.moveTimer < 0.0f) {
			g_Player.moveTimer = 0.0f;
		}
	}

	// ���ݑ��x����
	g_Player.spd = (g_Player.moveTimer / TIME_TO_REACH_FULLSPEED) * VALUE_MOVE;


	// �J�����̌������l���������������v�Z
	g_Player.rot.y = roty + cam->rot.y;

	// �ړ�
	g_Player.pos.x -= sinf(g_Player.rot.y) * g_Player.spd;
	g_Player.pos.z -= cosf(g_Player.rot.y) * g_Player.spd;

	// ���E����
	{
		float fieldSizse = GetFieldSize();
		
		if (g_Player.pos.x > fieldSizse / 2) {
			g_Player.pos.x = fieldSizse / 2;
		}
		else if (g_Player.pos.x < -fieldSizse / 2) {
			g_Player.pos.x = -fieldSizse / 2;
		}

		if (g_Player.pos.z > fieldSizse / 2) {
			g_Player.pos.z = fieldSizse / 2;
		}
		else if (g_Player.pos.z < -fieldSizse / 2) {
			g_Player.pos.z = -fieldSizse / 2;
		}
	}
	

	// �]���菈��
	{
		// ���ݑ��x�ƍő呬�x�Ƃ̍��œ]�������v�Z
		float rate = g_Player.spd / VALUE_MOVE;

		// �]����
		g_Player.roll += ROLL_SPEED * rate;
		
		// �I�[�o�[�t���[�h�~
		if (g_Player.roll >= XM_PI) {
			g_Player.roll = 0.0f;
		}
	}

	// ���C�L���X�g���đ����̍��������߂�
	XMFLOAT3 HitPosition;		// ��_
	XMFLOAT3 Normal;			// �Ԃ������|���S���̖@���x�N�g���i�����j
	bool ans = RayHitField(g_Player.pos, &HitPosition, &Normal);

	float offsetY = PLAYER_OFFSET_Y * g_Player.rate;	// �c�����ɍ��킹�ĕ���
	if (ans)
	{
		g_Player.pos.y = HitPosition.y + offsetY;
	}
	else
	{
		g_Player.pos.y = offsetY;
		Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}


	//// �e���ˏ���
	//if (GetKeyboardTrigger(DIK_SPACE))
	//{
	//	SetBullet(g_Player.pos, g_Player.rot);
	//}


	// �e���v���C���[�̈ʒu�ɍ��킹��
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Player.shadowIdx, pos);

	// �|�C���g���C�g�̃e�X�g
	//{
	//	LIGHT *light = GetLightData(1);
	//	XMFLOAT3 pos = g_Player.pos;
	//	pos.y += 20.0f;

	//	light->Position = pos;
	//	light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//	light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//	light->Type = LIGHT_TYPE_POINT;
	//	light->Enable = TRUE;
	//	SetLightData(1, light);
	//}




	//////////////////////////////////////////////////////////////////////
	// �p������
	//////////////////////////////////////////////////////////////////////

	XMVECTOR vx, nvx, up;
	XMVECTOR quat;
	float len, angle;


	g_Player.UpVector = Normal;
	up = { 0.0f, 1.0f, 0.0f, 0.0f };
	vx = XMVector3Cross(up, XMLoadFloat3(&g_Player.UpVector));

	nvx = XMVector3Length(vx);
	XMFLOAT3 test;
	XMStoreFloat3(&test, nvx);
	XMStoreFloat(&len, nvx);
	nvx = XMVector3Normalize(vx);
	angle = asinf(len);	// �t�����i�����l����p�x�����߂�j

	//quat = XMQuaternionIdentity();
	//quat = XMQuaternionRotationAxis(nvx, angle);
	quat = XMQuaternionRotationNormal(nvx, angle);

	//���ʐ��`��Ԃ��g�p���āA2�̒P�ʎl�����̊Ԃ��Ԃ���
	quat = XMQuaternionSlerp(XMLoadFloat4(&g_Player.Quaternion), quat, 0.05f);

	XMStoreFloat4(&g_Player.Quaternion, quat);






#ifdef _DEBUG
	PrintDebugProc("g_Player.rot.y = %f, ", g_Player.rot.y);
#endif

	 //�p�[�e�B�N��
	if (g_Player.spd > 0.1) {

		XMFLOAT3 pos;
		XMFLOAT3 scl;
		XMFLOAT3 move;
		XMFLOAT4 col;
		float rot;
		float length;	// ���o���鐨��
		int life;

		scl = XMFLOAT3(0.2f, 0.2f, 0.2f);

		// �����̓v���C���[�i�s�����̔��Α� +- 0.5���W�A���ȓ�
		rot = g_Player.rot.y;
		{
			float lRand = (rand() % 100) / 100.0f - 0.5f;
			lRand *= XM_PI;

			rot += lRand;
		}

		// �v���C���[�̂����Ƃ��畬�o
		float offset = 5.0f;	// �����p
		pos.x = g_Player.pos.x + sinf(rot) * offset;
		pos.y = SNOWFLAKE_OFFSET_Y;
		pos.z = g_Player.pos.z + cosf(rot) * offset;


#ifdef _DEBUG
		PrintDebugProc("rot = %f\n", rot);
#endif

		// ����
		length = rand() % 100 / 100.0f + 0.2f;

		move.x = sinf(rot) * length;
		move.y = 0.5f * XM_PI;					// �΂ߏ㕬�o
		move.z = cosf(rot) * length;

		col = XMFLOAT4(1.0f, 1.0f, 1.0f, rand() % 50 / 50.0f + 50.0f);

		life = 30;

		// �r���{�[�h�̐ݒ�
		SetParticle(pos, scl, move, XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), life);
	}



#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Player.pos.z = g_Player.pos.x = 0.0f;
		g_Player.spd = 0.0f;
		roty = 0.0f;
	}
#endif


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawPlayer(void)
{
	//return;
	XMMATRIX mtxScl, mtxRoll, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Player.scl.x * g_Player.rate, g_Player.scl.y * g_Player.rate, g_Player.scl.z * g_Player.rate);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// �]����𔽉f
	mtxRoll = XMMatrixRotationRollPitchYaw(g_Player.roll, 0.0f, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRoll);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �N�H�[�^�j�I���𔽉f
	// D3DXMatrixTransformation()�͖��ʂȌv�Z�ʂ���������s�g�p
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player.Quaternion));	// �N�H�[�^�j�I������ϊ��s��𐶐�
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);


	// �����̐ݒ�
	SetFuchi(1);

	// ���f���`��
	DrawModel(&g_Player.model);


	SetFuchi(0);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
//=============================================================================
PLAYER *GetPlayer(void)
{
	return &g_Player;
}

//=============================================================================
// �ړ��`�F�b�N
//=============================================================================
bool CheckMove(void) {

	bool key = false;

	if (GetKeyboardPress(DIK_LEFT) || GetKeyboardPress(DIK_A) || IsButtonPressed(0, BUTTON_LEFT))
	{
		roty = XM_PI / 2;
		key = true;
	}
	if (GetKeyboardPress(DIK_RIGHT) || GetKeyboardPress(DIK_D) || IsButtonPressed(0, BUTTON_RIGHT))
	{
		roty = -XM_PI / 2;
		key = true;
	}
	if (GetKeyboardPress(DIK_UP) || GetKeyboardPress(DIK_W) || IsButtonPressed(0, BUTTON_UP))
	{
		roty = XM_PI;
		key = true;
	}
	if (GetKeyboardPress(DIK_DOWN) || GetKeyboardPress(DIK_S) || IsButtonPressed(0, BUTTON_DOWN))
	{
		roty = 0.0f;
		key = true;
	}

	return key;
}

//=============================================================================
// �c�����ύX
//=============================================================================
void AddRate(float parameter) {
	g_Player.rate += RATE_CHANGE * parameter;

	// 1�ȉ��ɂ͂Ȃ�Ȃ�
	if (g_Player.rate < 1.0f) {
		g_Player.rate = 1.0f;
	}
}