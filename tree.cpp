//=============================================================================
//
// �؏��� [tree.cpp]
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "shadow.h"
#include "meshfield.h"
#include "tree.h"
#include "player.h"
#include "collision.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX			(1)				// �e�N�X�`���̐�

#define	TREE_WIDTH			(50.0f)			// ���_�T�C�Y
#define	TREE_HEIGHT			(80.0f)			// ���_�T�C�Y

#define	OFFSET_Y			-2.0f			// ���܂�

#define	COLLISION			8.0f			// �����蔻��

#define	COLLISOIN_TIME		60				// ���i�q�b�g�̊Ԋu


#define	TRANSPARENCY_BY_RAYCAST
#undef TRANSPARENCY_BY_RAYCAST

#ifdef TRANSPARENCY_BY_RAYCAST

#define	TRANSPARENCY_BY_RAYCAST_OFFSET	1000.0f		// �H�^�̃T�C�Y

#else

#define	HIDE_DEPTH			50.0f			// �B���[�x

#endif

#define	OPACITY				0.8f			// �s�����x

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexTree(void);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;	// ���_�o�b�t�@
static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static TREE					g_aTree[MAX_TREE];	// �؃��[�N
static int					g_TexNo;			// �e�N�X�`���ԍ�
static bool					g_bAlpaTest;		// �A���t�@�e�X�gON/OFF

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/tree.png",
};

//=============================================================================
// ����������
//=============================================================================
HRESULT InitTree(void)
{
	MakeVertexTree();

	// �e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	g_TexNo = 0;

	// �؃��[�N�̏�����
	for(int nCntTree = 0; nCntTree < MAX_TREE; nCntTree++)
	{
		ZeroMemory(&g_aTree[nCntTree].material, sizeof(g_aTree[nCntTree].material));
		g_aTree[nCntTree].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aTree[nCntTree].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aTree[nCntTree].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_aTree[nCntTree].fWidth = TREE_WIDTH;
		g_aTree[nCntTree].fHeight = TREE_HEIGHT;
		g_aTree[nCntTree].bUse = false;

		g_aTree[nCntTree].collision = COLLISION;
		g_aTree[nCntTree].isCrashed = false;
		g_aTree[nCntTree].collisionCnt = 0;
	}

	g_bAlpaTest = true;

	// �؂̗L����
	int fieldSize = (int)GetFieldSize();	// �t�B�[���h�̑傫��
	for (int i = 0; i < MAX_TREE; i++) {
		float fX = (rand() % fieldSize - fieldSize / 2) * 0.9f;		// �ǖ��܂�΍�
		float fZ = (rand() % fieldSize - fieldSize / 2) * 0.9f;		// �ǖ��܂�΍�

		SetTree(XMFLOAT3(fX, OFFSET_Y, fZ), 160.0f, 160.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTree(void)
{
	for(int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if(g_Texture[nCntTex] != NULL)
		{// �e�N�X�`���̉��
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	if(g_VertexBuffer != NULL)
	{// ���_�o�b�t�@�̉��
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateTree(void)
{

	for(int i = 0; i < MAX_TREE; i++)
	{
		if(g_aTree[i].bUse)
		{
			// ���i�q�b�g�^�C�}�[����
			if (g_aTree[i].isCrashed) {
				g_aTree[i].collisionCnt++;

				if (g_aTree[i].collisionCnt > COLLISOIN_TIME) {
					g_aTree[i].isCrashed = false;
					g_aTree[i].collisionCnt = 0;
				}
			}


#ifndef TRANSPARENCY_BY_RAYCAST

			// �[�x�œ�������
			{
				float depth = ViewDepth(g_aTree[i].pos);
				if (depth < HIDE_DEPTH) {
					// �߂������甼�����ɂ���
					g_aTree[i].material.Diffuse.w = 0.8f;
				}
				else {
					// �߂�
					g_aTree[i].material.Diffuse.w = 1.0f;
				}
			}

#endif


			// �e�̈ʒu�ݒ�
			SetPositionShadow(g_aTree[i].nIdxShadow, XMFLOAT3(g_aTree[i].pos.x, 0.1f, g_aTree[i].pos.z));
		}
	}


#ifdef _DEBUG
	// �A���t�@�e�X�gON/OFF
	if(GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? false: true;
	}

	//// �A���t�@�e�X�g��臒l�ύX
	//if(GetKeyboardPress(DIK_I))
	//{
	//	g_nAlpha--;
	//	if(g_nAlpha < 0)
	//	{
	//		g_nAlpha = 0;
	//	}
	//}
	//if(GetKeyboardPress(DIK_K))
	//{
	//	g_nAlpha++;
	//	if(g_nAlpha > 255)
	//	{
	//		g_nAlpha = 255;
	//	}
	//}
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTree(void)
{
	// ���e�X�g�ݒ�
	if (g_bAlpaTest == true)
	{
		// ���e�X�g��L����
		SetAlphaTestEnable(true);
	}

	// ���C�e�B���O�𖳌�
	SetLightEnable(false);

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for(int i = 0; i < MAX_TREE; i++)
	{
		if(g_aTree[i].bUse)
		{
			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �r���[�}�g���b�N�X���擾
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			//mtxWorld = XMMatrixInverse(nullptr, mtxView);
			//mtxWorld.r[3].m128_f32[0] = 0.0f;
			//mtxWorld.r[3].m128_f32[1] = 0.0f;
			//mtxWorld.r[3].m128_f32[2] = 0.0f;

			// �J�����Ɍ������鏈��
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_aTree[i].scl.x, g_aTree[i].scl.y, g_aTree[i].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aTree[i].pos.x, g_aTree[i].pos.y, g_aTree[i].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

#ifdef TRANSPARENCY_BY_RAYCAST

			// ���C�L���X�g�Ŕ���������
			{
				XMMATRIX mtxTree = mtxWorld;	// �؂̃r���[���W�̓������}�g���b�N�X
				XMMATRIX mtxCamPos = XMMatrixIdentity();	// �J�����̈ʒu�̃r���[���W�̓������}�g���b�N�X
				XMMATRIX mtxCamAt = XMLoadFloat4x4(&cam->mtxInvView);	// �J�����̒����_�̃r���[���W�̓������}�g���b�N�X

				// �}�g���b�N�X������W���𒊏o
				XMFLOAT3 tree = GetTranslationFromXMMATRIX(mtxTree);
				XMFLOAT3 camPos = GetTranslationFromXMMATRIX(mtxCamPos);
				XMFLOAT3 camAt = GetTranslationFromXMMATRIX(mtxCamAt);

				XMFLOAT3 p0, p1, p2, p3, pos0, pos1, hit, normal;

				// ����`��
				p0 = XMFLOAT3(tree.x,									tree.y + TRANSPARENCY_BY_RAYCAST_OFFSET,	tree.z);	// ��
				p1 = XMFLOAT3(tree.x + TRANSPARENCY_BY_RAYCAST_OFFSET,	tree.y,										tree.z);	// �E
				p2 = XMFLOAT3(tree.x - TRANSPARENCY_BY_RAYCAST_OFFSET,	tree.y,										tree.z);	// ��
				p3 = XMFLOAT3(tree.x,									tree.y - TRANSPARENCY_BY_RAYCAST_OFFSET,	tree.z);	// ��

				pos0 = camPos;
				pos1 = camAt;

				if (RayCast(p0, p1, p2, p3, pos0, pos1, &hit, &normal)) {

					// ���C���q�b�g������؂𔼓����ɂ���
					g_aTree[i].material.Diffuse.w = OPACITY;
				}
			}

#endif

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);


			// �}�e���A���ݒ�
			SetMaterial(g_aTree[i].material);

			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

			// �|���S���̕`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ���C�e�B���O��L����
	SetLightEnable(true);

	// ���e�X�g�𖳌���
	SetAlphaTestEnable(false);
}

//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexTree(void)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// ���_�o�b�t�@�ɒl���Z�b�g����
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = 60.0f;
	float fHeight = 90.0f;

	// ���_���W�̐ݒ�
	vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

	// �g�U���̐ݒ�
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// �e�N�X�`�����W�̐ݒ�
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}

int SetTree(XMFLOAT3 pos, float fWidth, float fHeight, XMFLOAT4 col)
{
	int nIdxTree = -1;

	for(int nCntTree = 0; nCntTree < MAX_TREE; nCntTree++)
	{
		if(!g_aTree[nCntTree].bUse)
		{
			g_aTree[nCntTree].pos = pos;
			g_aTree[nCntTree].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
			g_aTree[nCntTree].fWidth = fWidth;
			g_aTree[nCntTree].fHeight = fHeight;
			g_aTree[nCntTree].bUse = true;

			// �e�̐ݒ�
			g_aTree[nCntTree].nIdxShadow = CreateShadow(g_aTree[nCntTree].pos, 0.5f, 0.5f);

			nIdxTree = nCntTree;

			// 1�{�Z�b�g������I��
			break;
		}
	}

	return nIdxTree;
}

TREE* GetTree(void) {
	return &g_aTree[0];
}

