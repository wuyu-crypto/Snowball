//=============================================================================
//
// �����̕\���B
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "text.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	TEXTURE_MAX	128

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void DrawTextMetrics(ID3D11Device* dev, TEXTMETRIC tm, GLYPHMETRICS gm, int ox, int oy);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static bool g_Load = false;

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };

//=============================================================================
// ����������
//=============================================================================
HRESULT InitText(void) {

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
	if (hFont == NULL) {
		return E_FAIL;
	}

	// ���݂̃E�B���h�E�ɓK�p
	// �f�o�C�X�Ƀt�H���g���������Ȃ���GetGlyphOutline�֐��̓G���[�ƂȂ�
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

	// �t�H���g�r�b�g�}�b�v�擾
	const wchar_t* c = L"a";
	UINT code = (UINT)*c;

	// �K����ݒ�
	const int gradFlag = GGO_GRAY4_BITMAP;
	int grad = 0;	// �K���̍ő�l
	switch (gradFlag) {
	case GGO_GRAY2_BITMAP: grad = 4;	break;
	case GGO_GRAY4_BITMAP: grad = 16;	break;
	case GGO_GRAY8_BITMAP: grad = 64;	break;
	}
	if (grad == 0) {
		return E_FAIL;
	}

	// �r�b�g�}�b�v��ݒ�
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	GLYPHMETRICS gm;
	CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };
	// �r�b�g�}�b�v�ɕK�v�ȃu���b�N�T�C�Y�𒲂ׂ�
	DWORD size = GetGlyphOutlineW(
		hdc, 		// �t�H���g���ݒ肵�Ă���f�o�C�X�R���e�L�X�g�n���h��
		code,		// �\��������������Unicode�Őݒ�
		gradFlag,	// �𑜓x
		&gm,		// �r�b�g�}�b�v���i�[��
		0,			// �u���b�N�T�C�Y
		NULL,		// �r�b�g�}�b�v��ۑ�����u���b�N������
		&mat		// ��]�i�����͕ϊ��Ȃ��j
	);
	BYTE* pFontBMP = new BYTE[size];
	// ���ׂ��T�C�Y�ɂ��r�b�g�}�b�v���Đ���
	GetGlyphOutlineW(hdc, code, gradFlag, &gm, size, pFontBMP, &mat);

	// �R���e�L���g�ƃn���h���͂����v��Ȃ�������
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
	ID3D11Texture2D* pTex = 0;
	ID3D11Device* pDevice = GetDevice();
	if (FAILED(pDevice->CreateTexture2D(&rtDesc, nullptr, &pTex))) {
		return E_FAIL;
	}

	// �f�o�C�X�R���e�L�X�g���擾
	auto deviceContext = GetDeviceContext();

	// �t�H���g�p�e�N�X�`�����\�[�X�Ƀe�N�X�`�������R�s�[
	D3D11_MAPPED_SUBRESOURCE mappedSubrsrc;
	deviceContext->Map(pTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubrsrc);
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
	DWORD Alpha, Color;
	memset(pBits, 0, mappedSubrsrc.RowPitch * tm.tmHeight);
	for (int y = iOfs_y; y < iOfs_y + iBmp_h; y++)
	{
		for (int x = iOfs_x; x < iOfs_x + iBmp_w; x++)
		{
			Alpha =
				(255 * pFontBMP[x - iOfs_x + iBmp_w * (y - iOfs_y)])
				/ (Level - 1);
			Color = 0x00ffffff | (Alpha << 24);
			memcpy(
				(BYTE*)pBits + mappedSubrsrc.RowPitch * y + 4 * x,
				&Color,
				sizeof(DWORD));
		}
	}
	deviceContext->Unmap(pTex, 0);


	// ShaderResourceView�̏����쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = rtDesc.MipLevels;

	// ShaderResourceView�̏�����������
	ID3D11ShaderResourceView* srv;
	pDevice->CreateShaderResourceView(pTex, &srvDesc, &srv);

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
	delete[] pFontBMP;

	g_Load = true;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitText(void)
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
void UpdateText(void) {

}
//=============================================================================
// �`�揈��
//=============================================================================
void DrawText(void)
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

//=============================================================================
// �A���C���`��
//=============================================================================
//void DrawTextMetrics(ID3D11Device* dev, TEXTMETRIC tm, GLYPHMETRICS gm, int ox, int oy) {
//	XMMATRIX idn = XMMatrixIdentity();
//	dev->SetTransform(D3DTS_WORLD, &idn);
//	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
//
//	// Base Line
//	D3DVIEWPORT9 vp;
//	dev->GetViewport(&vp);
//	drawLineW(dev, (float)vp.Width / -2.0f, (float)oy, 0.0f, (float)vp.Width / 2, (float)oy, 0.0f, 0xffff0000);
//
//	// Ascent Line
//	drawLineW(dev, (float)vp.Width / -2.0f, (float)(oy + tm.tmAscent), 0.0f, (float)vp.Width / 2, (float)(oy + tm.tmAscent), 0.0f, 0xffff0000);
//
//	// Descent Line
//	drawLineW(dev, (float)vp.Width / -2.0f, (float)(oy - tm.tmDescent), 0.0f, (float)vp.Width / 2, (float)(oy - tm.tmDescent), 0.0f, 0xffff0000);
//
//	// Origin
//	drawRectW(dev, (float)ox - 2.0f, (float)oy + 2.0f, 4.0f, 4.0f, 0xff00ff00);
//
//	// Next Origin
//	drawRectW(dev, (float)(ox + gm.gmCellIncX) - 2.0f, (float)oy + 2.0f, 4.0f, 4.0f, 0xffffff00);
//
//	// BlackBox
//	drawRectW(dev, (float)(ox + gm.gmptGlyphOrigin.x), (float)(oy + gm.gmptGlyphOrigin.y), (float)gm.gmBlackBoxX, (float)gm.gmBlackBoxY, 0x00ff0000ff);
//};