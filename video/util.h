#include <atldlgs.h>
#include <string>

const FLOAT DEFAULT_DPI = 96.f;   // Default DPI that maps image resolution directly to screen resoltuion

inline BOOL get_file_path(wchar_t(&file_path_str)[128])
{
  WCHAR szFilter[] = L"All Files(*.*)|*.*||";
  CFileDialog dlg(TRUE, L"*", L"*", OFN_HIDEREADONLY, szFilter);
  if (IDOK == dlg.DoModal())
  {
    std::wstring wstr = &dlg.m_ofn.lpstrFile[0];
    if (wstr.length() > 128)
    {
      printf("Error get_file_path: length >128\n");
      return FALSE;
    }
    wcscpy_s(file_path_str, wstr.c_str());
    wprintf(L"printf: file path is %s\n", file_path_str);

    return TRUE;
  }
  return FALSE;
}

inline BOOL get_file_path(char(&file_path_str)[128])
{
  WCHAR szFilter[] = L"All Files(*.*)|*.*||";
  CFileDialog dlg(TRUE, L"*", L"*", OFN_HIDEREADONLY, szFilter);
  if (IDOK == dlg.DoModal())
  {
    std::wstring wstr = &dlg.m_ofn.lpstrFile[0];
    std::string str(wstr.begin(), wstr.end());
    if (str.length() > 128)
    {
      printf("Error get_file_path: length >128\n");
      return FALSE;
    }
    strncpy_s(file_path_str, str.c_str(), str.length());
    printf("printf: file path is %s\n", file_path_str);

    return TRUE;
  }
  return FALSE;
}