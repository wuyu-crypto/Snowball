//=============================================================================
//
// 文字の表示
//
//=============================================================================
#pragma once

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
/*
* @brief テキストの初期化
*/
HRESULT InitTextBox(void);

/*
* @brief テキストの終了処理
*/
void UninitTextBox(void);

/*
* @brief テキストの更新処理
*/
void UpdateTextBox(void);

/*
* @brief テキストの描画処理
*/
void DrawTextBox(void);

/*
* @brief テキストボックスをセット
* 受け取った文を元に合成した1枚のテキスト画像を生成。文は256文字まで。
* @param [in] inputText		Unicode文字列
* @param [in] font			フォント名
* @param [in] size			フォントサイズ。0=デフォルト
* @param [in] weight		フォントウェイト。0=デフォルト
* @param [in] color			文字の色
* @param [in] pos			座標
* @param [in] mode			有効化フラグ
* @retval true	正常終了
* @retval false 異常終了
*/
bool SetTextBox(wchar_t* inputText, TCHAR* font, int size, int weight, XMFLOAT4 color, XMFLOAT3 pos, bool mode);