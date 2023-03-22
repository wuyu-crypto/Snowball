//=============================================================================
//
// �����̕\��
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "textBox.h"
#include "sprite.h"

using namespace std;

//*****************************************************************************
// �}�N����`
//*****************************************************************************

#define	TEXT_BOX_MAX	512					// �ő啶(�e�L�X�g�摜)��
#define	CHAR_MAX		256					// �ꕶ(1���̃e�L�X�g�摜)�̍ő啶����

#define	DEFAULT_SIZE	32					// �f�t�H���g�t�H���g�T�C�Y
#define	DEFAULT_WEIGHT	10					// �f�t�H���g�t�H���g�E�F�C�g
#define	DEFAULT_GRAD	GGO_GRAY4_BITMAP	// �f�t�H���g�K��

#define	VECTOR

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct TEXT_BOX {

	bool								isInitiated;		// �������ς݂�

	XMFLOAT3							absPos;				// ������̍����΍��W

	vector<bool>						use;				// �L�����t���O
	vector<XMFLOAT3>					pos;				// �e�����̑��΍��W(�O���ɑ΂��āA[0]��(0,0,0))
	float								scl;				// �g�嗦
	XMFLOAT4							color;				// �F

	vector<ID3D11Buffer*>				vertexBuffer;		// ���_�o�b�t�@
	vector<ID3D11ShaderResourceView*>	texture;			// �e�N�X�`���f�[�^
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static bool g_Load = true;		// �ŏ��Ɉ��Uninit���邽��true�ɐݒ�

static TEXT_BOX g_TextBox[TEXT_BOX_MAX];

//=============================================================================
// ����������
//=============================================================================
HRESULT InitTextBox(void) {

	SetTextBox(L"�V", "���b����", 0, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), true);

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
void UninitTextBox(void)
{
	if (!g_Load) return;

	for (int i = 0; i < TEXT_BOX_MAX; i++) {

		for (int j = 0; j < g_TextBox[i].use.size(); j++) {

			g_TextBox[i].use[j] = false;

			if (g_TextBox[i].vertexBuffer[j]) {
				g_TextBox[i].vertexBuffer[j]->Release();
				g_TextBox[i].vertexBuffer[j] = NULL;
			}

			if (g_TextBox[i].texture[j]) {
				g_TextBox[i].texture[j]->Release();
				g_TextBox[i].texture[j] = NULL;
			}
		}

		// ���������t���O��false��
		g_TextBox[i].isInitiated = false;
	}

	g_Load = false;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateTextBox(void) {

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTextBox(void)
{
	for (auto textBox : g_TextBox) {
		if (!textBox.isInitiated)	continue;

		float px = textBox.absPos.x;
		float py = textBox.absPos.y;

		// 1�������`��
		for (int i = 0; i < textBox.use.size(); i++) {
			if (!textBox.use[i])	continue;

			// ���_�o�b�t�@�ݒ�
			UINT stride = sizeof(VERTEX_3D);
			UINT offset = 0;
			GetDeviceContext()->IASetVertexBuffers(0, 1, &textBox.vertexBuffer[i], &stride, &offset);

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
			GetDeviceContext()->PSSetShaderResources(0, 1, &textBox.texture[i]);

			float pw = textBox.scl;
			float ph = textBox.scl;

			float tw = 1.0f;
			float th = 1.0f;
			float tx = 0.0f;
			float ty = 0.0f;

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			// �����̐F�������Őݒ�
			SetSpriteLTColor(textBox.vertexBuffer[i], px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

			// ���̃e�N�X�`���̍��W�����߂�
			px += textBox.pos[i].x;
		}
	}
}



bool SetTextBox(wchar_t* code, TCHAR* font, int size, int weight, XMFLOAT4 color, XMFLOAT3 pos, bool mode) {

	// ��̃e�L�X�g�\���̃X���b�g��T��
	for (int i = 0; i < TEXT_BOX_MAX; i++) {

		if (g_TextBox[i].isInitiated)	continue;

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
		int len = wcslen(code);

		// 256�����ڈȍ~�͐؂藎�Ƃ����
		if (len > CHAR_MAX) {
			len = CHAR_MAX;
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





		// ��������1�������ƂɎ擾
		// ������������Ȃ��̂Ńq�[�v���g��
		TEXTMETRIC* tm = (TEXTMETRIC*)malloc(sizeof(TEXTMETRIC) * len);
		GLYPHMETRICS* gm = (GLYPHMETRICS*)malloc(sizeof(GLYPHMETRICS) * len);
		//TEXTMETRIC tm[CHAR_MAX];
		//GLYPHMETRICS gm[CHAR_MAX];
		BYTE* pFontBMP = new BYTE[size];
		for (int nText = 0; nText < len; nText++) {

			// �r�b�g�}�b�v��ݒ�
			GetTextMetrics(hdc, &tm[nText]);
			CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };
			// �r�b�g�}�b�v�ɕK�v�ȃu���b�N�T�C�Y�𒲂ׂ�
			DWORD size = GetGlyphOutlineW(
				hdc, 				// �t�H���g���ݒ肵�Ă���f�o�C�X�R���e�L�X�g�n���h��
				code[nText],		// �\��������������Unicode�Őݒ�
				gradFlag,			// �𑜓x
				&gm[nText],			// �r�b�g�}�b�v���i�[��
				0,					// �u���b�N�T�C�Y
				NULL,				// �r�b�g�}�b�v��ۑ�����u���b�N������
				&mat				// ��]�i�����͕ϊ��Ȃ��j
			);
			pFontBMP.push_back(new BYTE[size]);
			// ���ׂ��T�C�Y�ɂ��r�b�g�}�b�v���Đ���
			GetGlyphOutlineW(hdc, code[nText], gradFlag, &gm[nText], size, pFontBMP.back(), &mat);
		}

		// �R���e�L���g�ƃn���h���͂����v��Ȃ�������
		SelectObject(hdc, oldFont);
		ReleaseDC(NULL, hdc);







		// �e�N�X�`���̍��v�������v�Z
		int text_w = 0, text_h = 0;
		for (int nText = 0; nText < len; nText++) {
			// ���v���͊e������gm.gmCellIncX�̍��v
			text_w += gm[nText].gmCellIncX;
			// ���v�����͑S������tm.tmHeight�̍ő�l
			text_h = (text_h < tm[nText].tmHeight) ? tm[nText].tmHeight : text_h;

		}

		// 1���̃f�J���󔒃e�N�X�`�������
		// �����_�[�^�[�Q�b�g�̐ݒ�
		D3D11_TEXTURE2D_DESC rtDesc;
		ZeroMemory(&rtDesc, sizeof(rtDesc));
		rtDesc.Width = text_w;
		rtDesc.Height = text_h;
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
		BYTE* pBits = (BYTE*)mappedSubrsrc.pData;	// �������u���b�N
		// ���n�e�N�X�`���̃r�b�g�}�b�v��0�N���A
		memset(
			pBits,									// �Z�b�g�惁�����u���b�N 
			0,										// �Z�b�g���鐔�l
			mappedSubrsrc.RowPitch * text_h			// �Z�b�g����o�C�g��(�e�N�X�`���T�C�Y�H)
		);

		// �e�N�X�`���ɏ�������
		int iOfs_x = 0;
		for (int nText = 0; nText < len; nText++) {

			// �t�H���g�r�b�g�}�b�v���(�h�b�g���Ƃ̃��l)���e�N�X�`���ɏ�������
			// �Q�l�Fhttp://marupeke296.com/DXG_No67_NewFont.html

			// iOfs_x, iOfs_y : �����o���ʒu(����)
			iOfs_x += gm[nText].gmptGlyphOrigin.x;
			int iOfs_y = tm[nText].tmAscent - gm[nText].gmptGlyphOrigin.y;
			// iBmp_w, iBmp_h : �t�H���g�r�b�g�}�b�v(�u���b�N�{�b�N�X)�̕���
			int iBmp_w = iOfs_x + gm[nText].gmBlackBoxX + (4 - (gm[nText].gmBlackBoxX % 4)) % 4;	// ���_���W+�u���b�N�{�b�N�X�̕�(4�o�C�g�P�ʂɕϊ�)
			int iBmp_h = gm[nText].gmBlackBoxY;
			// Level : ���l�̒i�K(grad+1�i�K)
			int Level = grad + 1;
			DWORD Alpha, Color;

			// �r�b�g�}�b�v��������
			for (int y = iOfs_y; y < iOfs_y + iBmp_h; y++) {
				for (int x = iOfs_x; x < iOfs_x + iBmp_w; x++) {
					// ���l���K���Ɋ�Â���0�`255�ɂ���
					Alpha = pFontBMP[nText][x - iOfs_x + iBmp_w * (y - iOfs_y)] * 255 / (Level - 1);
					Color = 0x00ffffff | (Alpha << 24);
					memcpy(
						(BYTE*)pBits + mappedSubrsrc.RowPitch * y + 4 * x,	// �R�s�[��u���b�N
						&Color,												// �R�s�[��
						sizeof(DWORD));
				}
			}
		}

		deviceContext->Unmap(pTex, 0);

		// �r�b�g�}�b�v�͂����v��Ȃ��̂ŉ��
		free(tm);
		free(gm);

		// ShaderResourceView�A���_�o�b�t�@����
		for (int nText = 0; nText < len; nText++) {

			// ShaderResourceView�̏����쐬
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = rtDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = rtDesc.MipLevels;

			// ShaderResourceView�̏����e�L�X�g�\���̂ɏ�������
			pDevice->CreateShaderResourceView(pTex, &srvDesc,
				&g_TextBox[i].texture	// SRV�i�[��
			);

			// ���_�o�b�t�@����
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(VERTEX_3D) * 4;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			GetDevice()->CreateBuffer(&bd, NULL, 
				&g_TextBox[i].vertexBuffer	// ���_�o�b�t�@�i�[��
			);



			g_TextBox[i].pos = pos;
			g_TextBox[i].color = color;
			g_TextBox[i].use = mode;
			g_TextBox[i].isInitiated = true;

			// ��Z�b�g�ł�����I��
			return true;
		}
	}

	return false;	// ����Z�b�g�ł��Ȃ�����
}