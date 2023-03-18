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
HRESULT InitTextbox(void);

/*
* @brief テキストの終了処理
*/
void UninitTextbox(void);

/*
* @brief テキストの更新処理
*/
void UpdateTextbox(void);

/*
* @brief テキストの描画処理
*/
void DrawTextbox(void);

/*
* @brief テキストをセット
* @param [in] text		Unicode文字列
* @param [in] font		フォント名
* @param [in] size		フォントサイズ。0=デフォルト
* @param [in] weight	フォントウェイト。0=デフォルト
* @param [in] color		文字の色
* @param [in] pos		座標
* @param [in] mode		有効化フラグ
* @retval true	正常終了
* @retval false 異常終了
*/
bool SetTextbox(wchar_t* text, TCHAR* font, int size, int weight, XMFLOAT4 color, XMFLOAT3 pos, bool mode);