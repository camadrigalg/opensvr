#ifndef CHARUTILS_INCLUDE
#define CHARUTILS_INCLUDE 1

#include <iostream>

class CharForLocal
{
public:
  CharForLocal(void)
  {
    setlocale(LC_ALL,"chs");
  }
};

static CharForLocal _CharForLocal;


inline const char *LOCAL_2_MCBS(LPCTSTR lpstr)
{
#ifdef _UNICODE
  const size_t len = wcslen(lpstr);
  static std::string strRet;
  strRet.resize(len + len + 2, '\0');
  wcstombs(&*strRet.begin(), lpstr, strRet.length());
  return strRet.c_str();
#else
  return lpstr;
#endif
}


inline LPCTSTR MCBS_2_LOCAL(const char *lpstr)
{
#ifdef _UNICODE
  std::string str;
  const size_t len = strlen(lpstr);
  static std::wstring strRet;
  strRet.resize(len + 2, 0);
  mbstowcs(&*strRet.begin(), lpstr, strRet.length());
  return strRet.c_str();
#else
  return lpstr;
#endif
}

#endif//CHARUTILS_INCLUDE