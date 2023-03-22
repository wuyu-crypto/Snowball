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
HRESULT InitTextBox(void);

/*
* @brief �e�L�X�g�̏I������
*/
void UninitTextBox(void);

/*
* @brief �e�L�X�g�̍X�V����
*/
void UpdateTextBox(void);

/*
* @brief �e�L�X�g�̕`�揈��
*/
void DrawTextBox(void);

/*
* @brief �e�L�X�g�{�b�N�X���Z�b�g
* �󂯎�����������ɍ�������1���̃e�L�X�g�摜�𐶐��B����256�����܂ŁB
* @param [in] inputText		Unicode������
* @param [in] font			�t�H���g��
* @param [in] size			�t�H���g�T�C�Y�B0=�f�t�H���g
* @param [in] weight		�t�H���g�E�F�C�g�B0=�f�t�H���g
* @param [in] color			�����̐F
* @param [in] pos			���W
* @param [in] mode			�L�����t���O
* @retval true	����I��
* @retval false �ُ�I��
*/
bool SetTextBox(wchar_t* inputText, TCHAR* font, int size, int weight, XMFLOAT4 color, XMFLOAT3 pos, bool mode);