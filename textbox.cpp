//=============================================================================
//
// �����̕\��
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "textbox.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************

#define	TEXTBOX_MAX		512					// �ő啶����
#define	TEXT_MAX		256					// �ꕶ(1���̃e�L�X�g�摜)�̍ő啶����

#define	DEFAULT_SIZE	32					// �f�t�H���g�t�H���g�T�C�Y
#define	DEFAULT_WEIGHT	10					// �f�t�H���g�t�H���g�E�F�C�g
#define	DEFAULT_GRAD	GGO_GRAY4_BITMAP	// �f�t�H���g�K��

#define	VECTOR


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct TEXTBOX {
	bool		isInitiated;	// �������ς݂�

	bool		use;	// �L�����t���O
	wchar_t		text;	// ����
	XMFLOAT3	pos;	// 1�����ڂ̍�����W
	XMFLOAT4	color;	// �F

	ID3D11Buffer* vertexBuffer;				// ���_�o�b�t�@
	ID3D11ShaderResourceView* textureSrv;	// �e�N�X�`���f�[�^
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static bool g_Load = false;

static TEXTBOX g_Textbox[TEXTBOX_MAX];

//=============================================================================
// ����������
//=============================================================================
HRESULT InitTextbox(void) {

	SetTextbox(L"�V��", "���b����", 0, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), true);

#ifndef VECTOR
	// �t�H���g�n���h���̐ݒ�
	int fontSize = 64;
	int fontWeight = 10000;
	LOGFONT lf = {
		fontSize, 0, 0, 0,
		fontWeight, 0, 0, 0,
		SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		DEFAULT_PITCH | FF_MODERN,
		_T("���b����")
	};

	// �t�H���g�n���h���̐���
	HFONT hFont = CreateFontIndirect(&lf);
	if (hFont == NULL) {
		return E_FAIL;
	}

	// ���݂̃E�B���h�E�ɓK�p
	// �f�o�C�X�Ƀt�H���g���������Ȃ���GetGlyphOutline�֐��̓G���[�ƂȂ�
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

	// Unicode�������擾
	const wchar_t* c = L"abc������";	// Unicode����
	UINT code = (UINT)*c;				// Unicode�ɕϊ�

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
	INT fontWidth = gm.gmCellIncX;
	INT fontHeight = tm.tmHeight;

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

	// �t�H���g�r�b�g�}�b�v���(�h�b�g���Ƃ̃��l)���e�N�X�`���ɏ�������
	// iOfs_x, iOfs_y : �����o���ʒu(����)
	int iOfs_x = gm.gmptGlyphOrigin.x;
	int iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y;
	// iBmp_w, iBmp_h : �t�H���g�r�b�g�}�b�v�̕���
	int iBmp_w = gm.gmBlackBoxX + (4 - (gm.gmBlackBoxX % 4)) % 4;	// 4�o�C�g�P�ʂɕϊ�
	int iBmp_h = gm.gmBlackBoxY;
	// Level : ���l�̒i�K(grad+1�i�K)
	int Level = grad + 1;
	DWORD Alpha, Color;
	// ���n�e�N�X�`���̃r�b�g�}�b�v��0�N���A
	memset(
		pBits,									// �Z�b�g�惁�����u���b�N 
		0,										// �Z�b�g���鐔�l
		mappedSubrsrc.RowPitch * tm.tmHeight	// �Z�b�g����o�C�g��
	);
	for (int y = iOfs_y; y < iOfs_y + iBmp_h; y++)
	{
		for (int x = iOfs_x; x < iOfs_x + iBmp_w; x++)
		{
			// ����s�\
			Alpha = (255 * pFontBMP[x - iOfs_x + iBmp_w * (y - iOfs_y)]) / (Level - 1);
			Color = 0x00ffffff | (Alpha << 24);
			memcpy(
				(BYTE*)pBits + mappedSubrsrc.RowPitch * y + 4 * x,	// �R�s�[��u���b�N
				&Color,												// �R�s�[��
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

#endif // !VECTOR

	g_Load = true;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTextbox(void)
{
#ifndef VECTOR


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

	// �e�L�X�g�\���̂����ׂĖ��������ɂ���
	for (int i = 0; i < TEXTBOX_MAX; i++) {
		g_Textbox[i].isInitiated = false;
	}


	g_Load = false;

#endif // !VECTOR
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateTextbox(void) {

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTextbox(void)
{

#ifndef VECTOR


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

	float px = 0;
	float py = 0;
	float pw = 100;
	float ph = 100;

	float tw = 1.0f;
	float th = 1.0f;
	float tx = 0.0f;
	float ty = 0.0f;

	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	// �����̐F�������Őݒ�
	SetSpriteLTColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// �|���S���`��
	GetDeviceContext()->Draw(4, 0);

#endif // !VECTOR


}



bool SetTextbox(wchar_t* text, TCHAR* font, int size, int weight, XMFLOAT4 color, XMFLOAT3 pos, bool mode) {

	// ��̃e�L�X�g�\���̃X���b�g��T��
	TEXTBOX* textbox;
	bool empty = false;
	for (int i = 0; i < TEXTBOX_MAX; i++) {
		if (!g_Textbox[i].isInitiated) {	// ���������X���b�g����
			textbox = &g_Textbox[i];		// �|�C���^���w��
			empty = true;
			break;
		}
	}
	// �󂫃X���b�g���Ȃ���΃e�L�X�g�������s
	if (!empty) {
		return false;
	}

	// �t�H���g�n���h���̐ݒ�
	int fontSize, fontWeight;
	if (size == 0) {
		fontSize = DEFAULT_SIZE;
	}
	else
	{
		fontSize = size;
	}
	if (weight == 0) {
		fontWeight = DEFAULT_WEIGHT;
	}
	else
	{
		fontWeight = weight;
	}

	// �t�H���g�������`
	LOGFONT lf = {
		fontSize, 0, 0, 0,
		fontWeight, 0, 0, 0,
		SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		DEFAULT_PITCH | FF_MODERN,
		_T("")
	};
	// �t�H���g�����Ď擾
	for (int i = 0; i < _tcslen(font); i++) {
		lf.lfFaceName[i] = font[i];
	}

	// �t�H���g�n���h���̐���
	HFONT hFont = CreateFontIndirect(&lf);
	if (hFont == NULL) {
		return false;
	}

	// ���݂̃E�B���h�E�ɓK�p
	// �f�o�C�X�Ƀt�H���g���������Ȃ���GetGlyphOutline�֐��̓G���[�ƂȂ�
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);


	// ������̒����𒲂ׂ�
	int len = wcslen(text);

	// 256�����ڈȍ~�͐؂藎�Ƃ����
	if (len > TEXT_MAX) {
		len = TEXT_MAX;
	}

	// Unicode�������擾
	UINT* code = new UINT[len];			// Unicode������
	for (int i = 0; i < len; i++) {
		code[i] = (UINT)(text + i);		// 1�������Ǝ擾
	}

	// �K����ݒ�
	const int gradFlag = DEFAULT_GRAD;
	int grad = 0;	// �K���̍ő�l
	switch (gradFlag) {
	case GGO_GRAY2_BITMAP: grad = 4;	break;
	case GGO_GRAY4_BITMAP: grad = 16;	break;
	case GGO_GRAY8_BITMAP: grad = 64;	break;
	}
	if (grad == 0) {
		return false;
	}

	// ���������1�������ƂɎ擾
	TEXTMETRIC tm[TEXT_MAX];
	GLYPHMETRICS gm[TEXT_MAX];
	for (int nText = 0; nText < len; nText++) {

		// �r�b�g�}�b�v��ݒ�
		GetTextMetrics(hdc, &tm[nText]);
		CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };
		// �r�b�g�}�b�v�ɕK�v�ȃu���b�N�T�C�Y�𒲂ׂ�
		DWORD size = GetGlyphOutlineW(
			hdc, 			// �t�H���g���ݒ肵�Ă���f�o�C�X�R���e�L�X�g�n���h��
			code[nText],	// �\��������������Unicode�Őݒ�
			gradFlag,		// �𑜓x
			&gm[nText],			// �r�b�g�}�b�v���i�[��
			0,				// �u���b�N�T�C�Y
			NULL,			// �r�b�g�}�b�v��ۑ�����u���b�N������
			&mat			// ��]�i�����͕ϊ��Ȃ��j
		);
		BYTE* pFontBMP = new BYTE[size];
		// ���ׂ��T�C�Y�ɂ��r�b�g�}�b�v���Đ���
		GetGlyphOutlineW(hdc, code[nText], gradFlag, &gm[nText], size, pFontBMP, &mat);
	}

	// �R���e�L���g�ƃn���h���͂����v��Ȃ�������
	SelectObject(hdc, oldFont);
	ReleaseDC(NULL, hdc);

	// �e�N�X�`���̍��v�������v�Z
	int textboxWidth, textboxHeight = 0;
	for (int nText = 0; nText < len; nText++) {
		// ���v���͊e������gm.gmCellIncX�̍��v
		textboxWidth += gm[nText].gmCellIncX;
		// ���v�����͑S������tm.tmHeight�̍ő�l
		textboxHeight = (textboxHeight < tm[nText].tmHeight) ? tm[nText].tmHeight : textboxHeight;

	}

	// 1���̃f�J���󔒃e�N�X�`�������
	// �����_�[�^�[�Q�b�g�̐ݒ�
	D3D11_TEXTURE2D_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));
	rtDesc.Width = textboxWidth;
	rtDesc.Height = textboxHeight;
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

	// �����ɏ�������
	BYTE* pBits = (BYTE*)mappedSubrsrc.pData;

	// �e�N�X�`���ɏ�������
	for (int nText = 0; nText < len; nText++) {
		// �t�H���g�r�b�g�}�b�v���(�h�b�g���Ƃ̃��l)���e�N�X�`���ɏ�������
		// iOfs_x, iOfs_y : �����o���ʒu(����)
		int iOfs_x = gm.gmptGlyphOrigin.x;
		int iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y;
		// iBmp_w, iBmp_h : �t�H���g�r�b�g�}�b�v�̕���
		int iBmp_w = gm.gmBlackBoxX + (4 - (gm.gmBlackBoxX % 4)) % 4;	// 4�o�C�g�P�ʂɕϊ�
		int iBmp_h = gm.gmBlackBoxY;
		// Level : ���l�̒i�K(grad+1�i�K)
		int Level = grad + 1;
		DWORD Alpha, Color;
		// ���n�e�N�X�`���̃r�b�g�}�b�v��0�N���A
		memset(
			pBits,									// �Z�b�g�惁�����u���b�N 
			0,										// �Z�b�g���鐔�l
			mappedSubrsrc.RowPitch * tm.tmHeight	// �Z�b�g����o�C�g��
		);
		for (int y = iOfs_y; y < iOfs_y + iBmp_h; y++)
		{
			for (int x = iOfs_x; x < iOfs_x + iBmp_w; x++)
			{
				// ����s�\
				Alpha = (255 * pFontBMP[x - iOfs_x + iBmp_w * (y - iOfs_y)]) / (Level - 1);
				Color = 0x00ffffff | (Alpha << 24);
				memcpy(
					(BYTE*)pBits + mappedSubrsrc.RowPitch * y + 4 * x,	// �R�s�[��u���b�N
					&Color,												// �R�s�[��
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
	}

	delete[] code;
}