//=============================================================================
//
// e [shadow.h]
// Author : HAL@Q[wΘ@ΰD
//
//=============================================================================
#pragma once


//*****************************************************************************
// vg^CvιΎ
//*****************************************************************************
HRESULT InitShadow(void);
void UninitShadow(void);
void UpdateShadow(void);
void DrawShadow(void);

int CreateShadow(XMFLOAT3 pos, float fSizeX, float fSizeZ);
void ReleaseShadow(int nIdxShadow);
void SetPositionShadow(int nIdxShadow, XMFLOAT3 pos);
void SetColorShadow(int nIdxShadow, XMFLOAT4 col);
