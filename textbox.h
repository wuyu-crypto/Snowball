//=============================================================================
//
// �����̕\��
//
//=============================================================================
#pragma once

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
/*
* @brief �e�L�X�g�̏�����
*/
HRESULT InitTextbox(void);

/*
* @brief �e�L�X�g�̏I������
*/
void UninitTextbox(void);

/*
* @brief �e�L�X�g�̍X�V����
*/
void UpdateTextbox(void);

/*
* @brief �e�L�X�g�̕`�揈��
*/
void DrawTextbox(void);

/*
* @brief �e�L�X�g���Z�b�g
* @param [in] text		Unicode������
* @param [in] font		�t�H���g��
* @param [in] size		�t�H���g�T�C�Y�B0=�f�t�H���g
* @param [in] weight	�t�H���g�E�F�C�g�B0=�f�t�H���g
* @param [in] color		�����̐F
* @param [in] pos		���W
* @param [in] mode		�L�����t���O
* @retval true	����I��
* @retval false �ُ�I��
*/
bool SetTextbox(wchar_t* text, TCHAR* font, int size, int weight, XMFLOAT4 color, XMFLOAT3 pos, bool mode);