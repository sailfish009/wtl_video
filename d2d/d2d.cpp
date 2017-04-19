#include "d2d.h"
#include <stdio.h>
#pragma comment(lib, "d2d1")

d2d::d2d(HWND hWnd, UINT32 w, UINT32 h):
  m_hwnd(hWnd), pixel_w(w), pixel_h(h),
  m_d2d_factory(nullptr), m_render_target(nullptr), m_bitmap(nullptr),
  m_pIWICFactory(nullptr), m_pConvertedSourceBitmap(nullptr),
  p_bitmap(nullptr), p_lock(nullptr),
  p_source(nullptr)
{
  HRESULT hr = S_OK;

  data_size = pixel_w * pixel_h * 3 / 2; // YUV

  // Create WIC factory
  hr = CoCreateInstance(
    CLSID_WICImagingFactory,
    NULL,
    CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&m_pIWICFactory)
  );

  if (SUCCEEDED(hr))
  {
    CreateFactory();
    //CreateBitmap(m_size_x, m_size_y);
    renderTargetSize = m_render_target->GetSize();
  }
}

d2d::~d2d()
{
	SafeRelease(m_d2d_factory);
	SafeRelease(m_render_target);
	SafeRelease(m_bitmap);

  SafeRelease(m_pConvertedSourceBitmap);
  SafeRelease(m_pIWICFactory);
}

//-----------------------------------------------------------------------
//	CreateRenderTarget
//-----------------------------------------------------------------------
HRESULT d2d::CreateFactory()
{
	HRESULT hr = E_FAIL;

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2d_factory);

  if(FAILED(hr))
    return hr;

	hr = m_d2d_factory->CreateHwndRenderTarget
  (
    D2D1::RenderTargetProperties(),
	  D2D1::HwndRenderTargetProperties(m_hwnd, D2D1::SizeU(pixel_w, pixel_h)),
	  &m_render_target
  );

  return hr;
}

//-----------------------------------------------------------------------
//	CreateBtmap
//-----------------------------------------------------------------------
void d2d::CreateBitmap(int w, int h)
{
	HRESULT hr = E_FAIL;
	FLOAT	dpiX, dpiY;
	m_d2d_factory->GetDesktopDpi(&dpiX, &dpiY);

	D2D1_PIXEL_FORMAT		    pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE};
	D2D1_SIZE_U				      bitmapSize  = { (UINT32)w, (UINT32)h };
	D2D1_BITMAP_PROPERTIES	bitmapProps = {pixelFormat, dpiX, dpiY};
	hr = m_render_target->CreateBitmap(bitmapSize, bitmapProps, &m_bitmap);
}

//-----------------------------------------------------------------------
//	OnRender1ch
//-----------------------------------------------------------------------
void d2d::Render(BYTE *buf, int pitch)
{
	PAINTSTRUCT			ps;
	HDC					    hdc;

	hdc = BeginPaint(m_hwnd, &ps);

	//image copy
  m_bitmap->CopyFromMemory(NULL, buf, pitch);

	//Draw begin
	m_render_target->BeginDraw();
	m_render_target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
	m_render_target->DrawBitmap(m_bitmap, D2D1::RectF((FLOAT)0, (FLOAT)0, (FLOAT)pixel_w, (FLOAT)pixel_h));
	m_render_target->EndDraw();

	EndPaint(m_hwnd, &ps);
}

HRESULT d2d::Render()
{
  HRESULT hr = S_OK;

#if 0
  PAINTSTRUCT			ps;
  HDC					    hdc;
  hdc = BeginPaint(m_hwnd, &ps);
#endif

  m_render_target->BeginDraw();
  m_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
  m_render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));
  D2D1_SIZE_F rtSize = m_render_target->GetSize();

  // Create a rectangle same size of current window
  D2D1_RECT_F rectangle = D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height);

  // D2DBitmap may have been released due to device loss. 
  // If so, re-create it from the source bitmap
  if (m_pConvertedSourceBitmap && !m_bitmap)
  {
    m_render_target->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, NULL, &m_bitmap);
  }

  // Draws an image and scales it to the current window size
  if (m_bitmap)
    m_render_target->DrawBitmap(m_bitmap, rectangle);

  hr = m_render_target->EndDraw();
  // In case of device loss, discard D2D render target and D2DBitmap
  // They will be re-create in the next rendering pass
  if (hr == D2DERR_RECREATE_TARGET)
  {
    SafeRelease(m_bitmap);
    SafeRelease(m_render_target);
    // Force a re-render
    hr = InvalidateRect(m_hwnd, NULL, TRUE) ? S_OK : E_FAIL;
  }
	//EndPaint(m_hwnd, &ps);

  return hr;
}


#if 0
void d2d::Render(const UINT8* data)
{
  WICRect rcLock = { 0, 0, (INT)pixel_w, (INT)pixel_h };
  HRESULT hr = p_bitmap->Lock(&rcLock, WICBitmapLockWrite, &p_lock);

  if (SUCCEEDED(hr))
  {
    UINT cbBufferSize = 0;
    BYTE *pv = NULL;
    hr = p_lock->GetDataPointer(&cbBufferSize, &pv);
    if (SUCCEEDED(hr))
    {
      memcpy(pv, data, cbBufferSize);
    }

    m_render_target->BeginDraw();
    m_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
    hr = m_render_target->CreateBitmapFromWicBitmap(p_bitmap, NULL, &m_bitmap);
    if (SUCCEEDED(hr))
    {
      D2D1_RECT_F rectangle = D2D1::RectF(0.0f, 0.0f, renderTargetSize.width, renderTargetSize.height);
      // Draws an image and scales it to the current window size
      if (m_bitmap)
        m_render_target->DrawBitmap(m_bitmap, rectangle);
    }
    m_render_target->EndDraw();
  }

}
#endif


HRESULT d2d::Render(UINT8* data)
{
  HRESULT hr = S_OK;

  // Create a decoder
  IWICBitmapDecoder *p_decoder = NULL;

  hr = m_pIWICFactory->CreateStream(&p_stream);
  if (FAILED(hr))
  {
    printf("%d\n", __LINE__);
    return hr;
  }

  hr = p_stream->InitializeFromMemory(data, data_size);
  if (FAILED(hr))
  {
    printf("%d\n", __LINE__);
    return hr;
  }

  hr = m_pIWICFactory->CreateDecoderFromStream(
    p_stream,                      // Image to be decoded
    NULL,                            // Do not prefer a particular vendor
    WICDecodeMetadataCacheOnLoad,                    // Desired read access to the file
    &p_decoder                        // Pointer to the decoder
  );
  if (FAILED(hr))
  {
    printf("%d\n", __LINE__);
    return hr;
  }

  hr = p_decoder->GetFrame(0, &p_source);
  if (FAILED(hr))
  {
    printf("%d\n", __LINE__);
    return hr;
  }

  SafeRelease(m_pConvertedSourceBitmap);
  hr = m_pIWICFactory->CreateFormatConverter(&m_pConvertedSourceBitmap);
  if (FAILED(hr))
  {
    printf("%d\n", __LINE__);
    return hr;
  }

  hr = m_pConvertedSourceBitmap->Initialize(
    p_source,                          // Input bitmap to convert
    GUID_WICPixelFormat32bppPBGRA,   // Destination pixel format
    WICBitmapDitherTypeNone,         // Specified dither pattern
    NULL,                            // Specify a particular palette 
    0.f,                             // Alpha threshold
    WICBitmapPaletteTypeCustom       // Palette translation type
  );
  if (FAILED(hr))
  {
    printf("%d\n", __LINE__);
    return hr;
  }

  SafeRelease(m_bitmap);
  hr = m_render_target->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, &m_bitmap);
  if (FAILED(hr))
  {
    printf("%d\n", __LINE__);
    return hr;
  }

  m_render_target->BeginDraw();
  m_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
  if (SUCCEEDED(hr))
  {
    D2D1_RECT_F rectangle = D2D1::RectF(0.0f, 0.0f, renderTargetSize.width, renderTargetSize.height);
    // Draws an image and scales it to the current window size
    if (m_bitmap)
      m_render_target->DrawBitmap(m_bitmap, rectangle);
  }
  m_render_target->EndDraw();

  return hr;
}


HRESULT d2d::CreateBitmapFromFile(const wchar_t* filename)
{
  HRESULT hr = S_OK;

  // Create a decoder
  IWICBitmapDecoder *pDecoder = NULL;

  hr = m_pIWICFactory->CreateDecoderFromFilename(
    filename,                      // Image to be decoded
    NULL,                            // Do not prefer a particular vendor
    GENERIC_READ,                    // Desired read access to the file
    WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
    &pDecoder                        // Pointer to the decoder
  );

  // Retrieve the first frame of the image from the decoder
  IWICBitmapFrameDecode *pFrame = NULL;

  if (SUCCEEDED(hr))
  {
    hr = pDecoder->GetFrame(0, &pFrame);
  }

  //Step 3: Format convert the frame to 32bppPBGRA
  if (SUCCEEDED(hr))
  {
    SafeRelease(m_pConvertedSourceBitmap);
    hr = m_pIWICFactory->CreateFormatConverter(&m_pConvertedSourceBitmap);
  }

  if (SUCCEEDED(hr))
  {
    hr = m_pConvertedSourceBitmap->Initialize(
      pFrame,                          // Input bitmap to convert
      GUID_WICPixelFormat32bppPBGRA,   // Destination pixel format
      WICBitmapDitherTypeNone,         // Specified dither pattern
      NULL,                            // Specify a particular palette 
      0.f,                             // Alpha threshold
      WICBitmapPaletteTypeCustom       // Palette translation type
    );
  }

  //Step 4: Create render target and D2D bitmap from IWICBitmapSource
  if (SUCCEEDED(hr))
  {
    // Need to release the previous D2DBitmap if there is one
    SafeRelease(m_bitmap);
    hr = m_render_target->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, NULL, &m_bitmap);
  }

  SafeRelease(pDecoder);
  SafeRelease(pFrame);

  return hr;
}


#if 0
HRESULT d2d::CreateBitmapFromStream(const wchar_t* filename)
{
  HRESULT hr = S_OK;
  // Create a decoder
  IWICBitmapDecoder *pDecoder = NULL;
  return hr;
}
#endif