#pragma once

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include <wincodec.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Windowscodecs.lib")

template <typename T>
inline void SafeRelease(T *&p)
{
  if (NULL != p)
  {
    p->Release();
    p = NULL;
  }
}

class d2d
{
public:
  d2d(HWND hWnd, UINT32 w, UINT32 h);
  ~d2d();
  HRESULT	            CreateFactory();
  void	                      CreateBitmap(int w, int h);
  void	                      Render(BYTE *buf, int pitch);
  HRESULT             Render(UINT8* data);
  HRESULT             Render();
  //HRESULT             CreateBitmapFromStream(UINT8* data);
  //BYTE*                   rgb_to_rgba(BYTE *rgb, int count);
  HRESULT             CreateBitmapFromFile(const wchar_t* filename);

private:
  HWND						        m_hwnd;
  UINT32						      pixel_w;
  UINT32						      pixel_h;
  UINT32						      data_size;

  ID2D1Factory*		m_d2d_factory;
  ID2D1HwndRenderTarget*	m_render_target;
  ID2D1Bitmap*		m_bitmap;

  IWICImagingFactory       *m_pIWICFactory;
  IWICFormatConverter    *m_pConvertedSourceBitmap;
  IWICBitmapFrameDecode *p_source;
  //BYTE			             *rgba;

  IWICBitmap* p_bitmap;
  IWICBitmapLock* p_lock;
  IWICStream *p_stream;
  D2D1_SIZE_F renderTargetSize;
};

