//=============================================================================
//
// �t�H���g����
// Author : HAL�����@�Q�[���w�ȁ@���D
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "font.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static bool g_Load = false;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitFont(void) {

	// �t�H���g�n���h���̐���
	int fontSize = 64;
	int fontWeight = 1000;
	LOGFONT lf =
	{
		fontSize, 0, 0, 0, fontWeight, 0, 0, 0,
		SHIFTJIS_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY, DEFAULT_PITCH | FF_MODERN,
		"�l�r �o����"											// (WCHAR)��������ƃG���[���o��
	};
	HFONT hFont = CreateFontIndirectA(&lf);

	// ���݂̃E�B���h�E�ɓK�p
	// �f�o�C�X�Ƀt�H���g���������Ȃ���GetGlyphOutline�֐��̓G���[�ƂȂ�
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

	// �t�H���g�r�b�g�}�b�v�擾
	const wchar_t* c = L"S";
	UINT code = (UINT)*c;
	const int gradFlag = GGO_GRAY4_BITMAP;
	// �K���̍ő�l
	int grad = 0;
	switch (gradFlag)
	{
	case GGO_GRAY2_BITMAP:
		grad = 4;
		break;
	case GGO_GRAY4_BITMAP:
		grad = 16;
		break;
	case GGO_GRAY8_BITMAP:
		grad = 64;
		break;
	}

	// �r�b�g�}�b�v����
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	GLYPHMETRICS gm;	// �r�b�g�}�b�v�f�[�^
	CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };
	DWORD size = GetGlyphOutlineW(hdc, code, gradFlag, &gm, 0, NULL, &mat);
	BYTE* pMono = new BYTE[size];
	GetGlyphOutlineW(hdc, code, gradFlag, &gm, size, pMono, &mat);

	// ��e�N�X�`���쐬
	D3D11_TEXTURE2D_DESC fontTextureDesc;
	ZeroMemory(&fontTextureDesc, sizeof(fontTextureDesc));
	fontTextureDesc.Width = gm.gmCellIncX;
	fontTextureDesc.Height = gm.gmCellIncY;
	fontTextureDesc.MipLevels = 1;
	fontTextureDesc.ArraySize = 1;
	fontTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	fontTextureDesc.SampleDesc.Count = 1;
	fontTextureDesc.SampleDesc.Quality = 0;
	fontTextureDesc.Usage = D3D11_USAGE_DYNAMIC;
	fontTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	fontTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	fontTextureDesc.MiscFlags = 0;
	ID3D11Texture2D* pFontTexture = 0;
	
	ID3D11Device* pDevice = GetDevice();
	HRESULT hr = pDevice->CreateTexture2D(&fontTextureDesc, NULL, &pFontTexture);

	// �f�o�C�X�R���e�L�X�g
	auto deviceContext = GetDeviceContext();

	// �t�H���g�����e�N�X�`���ɏ������ޕ���
	D3D11_MAPPED_SUBRESOURCE hMappedResource;
	hr = deviceContext->Map(
		pFontTexture,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&hMappedResource);
	// �����ŏ�������
	BYTE* pBits = (BYTE*)hMappedResource.pData;
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
	memset(pBits, 0, hMappedResource.RowPitch * tm.tmHeight);
	for (y = iOfs_y; y < iOfs_y + iBmp_h; y++)
	{
		for (x = iOfs_x; x < iOfs_x + iBmp_w; x++)
		{
			Alpha =
				(255 * pMono[x - iOfs_x + iBmp_w * (y - iOfs_y)])
				/ (Level - 1);
			Color = 0x00ffffff | (Alpha << 24);
			memcpy(
				(BYTE*)pBits + hMappedResource.RowPitch * y + 4 * x,
				&Color,
				sizeof(DWORD));
		}
	}
	deviceContext->Unmap(pFontTexture, 0);


	// ShaderResourceView�̏����쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = fontTextureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = fontTextureDesc.MipLevels;

	// ShaderResourceView�̏�����������
	ID3D11ShaderResourceView* srv;
	pDevice->CreateShaderResourceView(pFontTexture, &srvDesc, &srv);














	// ���낢����
	delete[] pMono;
	SelectObject(hdc, oldFont);
	ReleaseDC(NULL, hdc);

	g_Load = true;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================

//=============================================================================
// �X�V����
//=============================================================================

//=============================================================================
// �`�揈��
//=============================================================================