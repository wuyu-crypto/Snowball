//=============================================================================
//
// �t�H���g����
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "font.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	TEXTURE_MAX	128

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static bool g_Load = false;

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };

//=============================================================================
// ����������
//=============================================================================
HRESULT InitFont(void) {

	// �t�H���g�n���h���̐ݒ�
	int fontSize = 64;
	int fontWeight = 1000;
	LOGFONTW lf =
	{
		fontSize, 0, 0, 0, 
		fontWeight, 0, 0, 0,
		SHIFTJIS_CHARSET, 
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		DEFAULT_PITCH | FF_MODERN,
		(WCHAR)"�l�r �o����"
	};
	// �t�H���g�n���h���̐���
	HFONT hFont = CreateFontIndirectW(&lf);

	// ���݂̃E�B���h�E�ɓK�p
	// �f�o�C�X�Ƀt�H���g���������Ȃ���GetGlyphOutline�֐��̓G���[�ƂȂ�
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

	// �t�H���g�r�b�g�}�b�v�擾
	const wchar_t* c = L"S";
	UINT code = (UINT)*c;

	// �K����ݒ�
	const int gradFlag = GGO_GRAY4_BITMAP;

	// �r�b�g�}�b�v��ݒ�
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	GLYPHMETRICS gm;	// �r�b�g�}�b�v�f�[�^
	CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };

	// �t�H���g�r�b�g�}�b�v���擾
	DWORD size = GetGlyphOutlineW(hdc, code, gradFlag, &gm, 0, NULL, &mat);
	BYTE* pMono = new BYTE[size];
	GetGlyphOutlineW(hdc, code, gradFlag, &gm, size, pMono, &mat);

	// �n���h���ƃR���e�L�X�g���
	SelectObject(hdc, oldFont);
	ReleaseDC(NULL, hdc);

	// �t�H���g�̕��ƍ���
	INT fontWidth = (gm.gmBlackBoxX + 3) / 4 * 4;
	INT fontHeight = gm.gmBlackBoxY;

	// �����_�[�^�[�Q�b�g�̐ݒ�
	D3D11_TEXTURE2D_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));
	rtDesc.Width = fontWidth;
	rtDesc.Height = fontHeight;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.SampleDesc.Quality = 0;
	rtDesc.Usage = D3D11_USAGE_DYNAMIC;
	rtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	rtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	rtDesc.MiscFlags = 0;

	// �t�H���g�p�e�N�X�`���쐬
	ID3D11Texture2D* layerBuffer = 0;
	ID3D11Device* pDevice = GetDevice();
	if (FAILED(pDevice->CreateTexture2D(&rtDesc, nullptr, &layerBuffer))) {
		return E_FAIL;
	}

	// �f�o�C�X�R���e�L�X�g���擾
	auto deviceContext = GetDeviceContext();

	// �t�H���g�p�e�N�X�`�����\�[�X�Ƀe�N�X�`�������R�s�[
	D3D11_MAPPED_SUBRESOURCE mappedSubrsrc;
	deviceContext->Map(
		layerBuffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedSubrsrc);
	// �����ŏ�������
	BYTE* pBits = (BYTE*)mappedSubrsrc.pData;
	// �t�H���g���̏�������
	// iOfs_x, iOfs_y : �����o���ʒu(����)
	// iBmp_w, iBmp_h : �t�H���g�r�b�g�}�b�v�̕���
	// Level : ���l�̒i�K (GGO_GRAY4_BITMAP�Ȃ̂�17�i�K)
	int iOfs_x = gm.gmptGlyphOrigin.x;
	int iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y;
	int iBmp_w = gm.gmBlackBoxX + (4 - (gm.gmBlackBoxX % 4)) % 4;
	int iBmp_h = gm.gmBlackBoxY;
	int Level = 17;
	int x, y;
	DWORD Alpha, Color;
	memset(pBits, 0, mappedSubrsrc.RowPitch * tm.tmHeight);
	for (y = iOfs_y; y < iOfs_y + iBmp_h; y++)
	{
		for (x = iOfs_x; x < iOfs_x + iBmp_w; x++)
		{
			Alpha =
				(255 * pMono[x - iOfs_x + iBmp_w * (y - iOfs_y)])
				/ (Level - 1);
			Color = 0x00ffffff | (Alpha << 24);
			memcpy(
				(BYTE*)pBits + mappedSubrsrc.RowPitch * y + 4 * x,
				&Color,
				sizeof(DWORD));
		}
	}
	deviceContext->Unmap(layerBuffer, 0);


	// ShaderResourceView�̏����쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = rtDesc.MipLevels;

	// ShaderResourceView�̏�����������
	ID3D11ShaderResourceView* srv;
	pDevice->CreateShaderResourceView(layerBuffer, &srvDesc, &srv);

	g_Texture[0] = srv;

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// ���낢����
	delete[] pMono;

	g_Load = true;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitFont(void)
{
	if (!g_Load) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}


	g_Load = false;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateFont(void) {

}
//=============================================================================
// �`�揈��
//=============================================================================
void DrawFont(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	float px = 100;
	float py = 100;
	float pw = 100;
	float ph = 100;

	float tw = 1.0f;
	float th = 1.0f;
	float tx = 0.0f;
	float ty = 0.0f;

	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	SetSprite(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th);

	// �|���S���`��
	GetDeviceContext()->Draw(4, 0);
}