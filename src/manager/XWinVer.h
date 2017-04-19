#pragma once

#define WUNKNOWN	0
#define W9XFIRST	1
#define W95			1
#define W95SP1		2
#define W95OSR2		3
#define W98			4
#define W98SP1		5
#define W98SE		6
#define WME			7
#define W9XLAST		99

#define WNTFIRST	101
#define WNT351		101
#define WNT4		102
#define W2K			103
#define WXP			104
#define W2003		105
#define WVISTA		106
#define WNTLAST		199

#define WCEFIRST	201
#define WCE			201
#define WCELAST		299
#define WSEVEN 300

#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE           3
#endif

#define GetWinVersion (XWinVersion::Instance())

class XWinVersion
{
public:
  static XWinVersion *Instance();
  void Init();
  BOOL IsWin2000();
  BOOL IsXP();
  BOOL IsXPClassic();
  BOOL IsVista();
  BOOL IsSeven();

protected:
  XWinVersion()
  {
  };
  XWinVersion(const XWinVersion & other)
  {
  }

private:
  static XWinVersion *_instance;
  int version;
  int ParseInfo(OSVERSIONINFOEX& m_osinfo);
};
