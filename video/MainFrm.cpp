#include "stdafx.h"
#include "MainFrm.h"

#include <thread>
#include <mutex>
std::mutex g_mutex;
std::condition_variable g_cond;

void CMainFrame::init()
{
  m_view.GetClientRect(&m_frame_rect);

  if (m_render != nullptr)
    delete m_render;
  m_render = new d2d(m_view.m_hWnd, pixel_w, pixel_h);

}

void CMainFrame::display(const wchar_t* path)
{
  //m_render->CreateBitmapFromFile(path);
  //m_render->Render();

  if (b_start == TRUE) return;

  if (_wfopen_s(&fp, path, L"rb+") == 0)
  {
    yuvPlaneSz = pixel_w * pixel_h * 3/2; 
    yuvPlane = std::make_unique<UINT8[]>(yuvPlaneSz);
#if 0
    yPlaneSz = pixel_w * pixel_h;
    uvPlaneSz = pixel_w * pixel_h / 4;
    uvPitch = pixel_w / 2;
    yPlane = std::make_unique<UINT8[]>(yPlaneSz);
    uPlane = std::make_unique<UINT8[]>(uvPlaneSz);
    vPlane = std::make_unique<UINT8[]>(uvPlaneSz);
#endif

    b_start = TRUE;
    std::thread t = std::thread([=] {display_proc(&b_start); });
  }

#if 0
  if (fopen_s(&fp, path, "rb+") == 0)
  {
    yPlaneSz = pixel_w * pixel_h;
    uvPlaneSz = pixel_w * pixel_h / 4;
    uvPitch = pixel_w / 2;
    yPlane = std::make_unique<UINT8[]>(yPlaneSz);
    uPlane = std::make_unique<UINT8[]>(uvPlaneSz);
    vPlane = std::make_unique<UINT8[]>(uvPlaneSz);

    int width = m_frame_rect.right - m_frame_rect.left;
    int height = m_frame_rect.bottom - m_frame_rect.top;

    HRESULT hr = CreateDeviceResources(m_view.m_hWnd);
  }
#endif

}


void CMainFrame::display_proc(BOOL *b_start)
{
  std::unique_lock<std::mutex> lock(g_mutex);
  while (*b_start)
  {
    size_t st = fread(yuvPlane.get(), 1, yuvPlaneSz, fp);
    if(st)
      m_render->Render(yuvPlane.get());
    else
      fseek(fp, 0, SEEK_SET);

#if 0
    size_t st = fread(yPlane.get(), 1, pixel_w * pixel_h, fp);
    fread(uPlane.get(), 1, pixel_w * pixel_h / 4, fp);
    fread(vPlane.get(), 1, pixel_w * pixel_h / 4, fp);
    SDL_UpdateYUVTexture(texture, NULL, yPlane.get(), pixel_w, uPlane.get(),
      uvPitch, vPlane.get(), uvPitch);
    SDL_RenderCopy(renderer, texture, &rect, &win_rect);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
#endif

    g_cond.wait_for(lock, std::chrono::milliseconds(33));
  }
}




void CMainFrame::render()
{

#if 0
  if (!(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
  {
    m_pRT->BeginDraw();

    m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

    // Clear the background
    m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

    D2D1_SIZE_F rtSize = m_pRT->GetSize();

    // Create a rectangle same size of current window
    D2D1_RECT_F rectangle = D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height);

    // D2DBitmap may have been released due to device loss. 
    // If so, re-create it from the source bitmap
    if (m_pConvertedSourceBitmap && !m_pD2DBitmap)
    {
      m_pRT->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, NULL, &m_pD2DBitmap);
    }

    // Draws an image and scales it to the current window size
    if (m_pD2DBitmap)
    {
      m_pRT->DrawBitmap(m_pD2DBitmap, rectangle);
    }

    HRESULT hr = m_pRT->EndDraw();

    // In case of device loss, discard D2D render target and D2DBitmap
    // They will be re-create in the next rendering pass
    if (hr == D2DERR_RECREATE_TARGET)
    {
      SafeRelease(m_pD2DBitmap);
      SafeRelease(m_pRT);
      // Force a re-render
      hr = ::InvalidateRect(m_view.m_hWnd, NULL, TRUE) ? S_OK : E_FAIL;
    }
  }
#endif

}
