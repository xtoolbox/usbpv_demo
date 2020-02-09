#include "windows.h"
#include "usbpv_cap.h"
#include "string.h"




struct info_t{
  HANDLE h;
  void* context;
  pfn_packet_handler cb;
  DWORD id;
  volatile bool run;
};

DWORD WINAPI thread(
  LPVOID param
  )
{
  info_t* info = (info_t*)param;
  int count = 0;

  while (info->run) {
    UINT8 data[] = { 0xb4,0x01,0x02 };
    if (info->run) {
      count++;
      if (count > 10) {
        static const char msg[] = "Too many data";
        info->cb(info->context, GetCurrentTime(), 0, msg, sizeof(msg), -1);
      }
      else {
        info->cb(info->context, GetCurrentTime(), 0, data, 3, 0);
      }
    }
    Sleep(200);
  }
  static const char msg[] = "Demo capture thread need quit";
  //info->cb(info->context, GetCurrentTime(), 0, msg, sizeof(msg), -2);
  return 0;
}

static const char demoOption[] = "<select,speed,HS,FS,LS>"
"<file,DataFile>";
long  __cdecl usbpv_get_option(char* option, long length)
{
  strcpy_s(option, length, demoOption);
  return sizeof(sizeof(demoOption));
}

static bool parse_option(const char* option, info_t* info)
{
#define REPORT_ERROR(fmt, ...)\
do{\
  char temp[1024];\
  wsprintfA(temp, fmt, ##__VA_ARGS__);\
  info->cb(info->context, 0, 0, temp, strlen(temp) + 1, -1);\
}while(0)

  const char* speed = strstr(option, "<speed,");
  if (!speed) {
    REPORT_ERROR("speed option not set");
    return false;
  }
  speed += 7;
  if (strncmp(speed, "FS", 2) != 0) {
    REPORT_ERROR("speed not FS, got %c%c", speed[0], speed[1]);
    return false;
  }

  const char* file = strstr(option, "<DataFile,");
  if (!file) {
    REPORT_ERROR("DataFile option not set");
    return false;
  }
  file += 10;
  if (strncmp(file, "test", 4) != 0) {
    REPORT_ERROR("file name not test, got %c%c...", file[0], file[1]);
    return false;
  }
  return true;
}

void* __cdecl usbpv_open(const char* option, void* context, pfn_packet_handler callback)
{
  DWORD id;
  info_t* info = new info_t;
  info->cb = callback;
  info->context = context;
  info->run = true;
  if (parse_option(option, info)) {

    HANDLE h = ::CreateThread(NULL, 0, thread, info, 0, &id);
    if (h != NULL) {
      info->h = h;
      info->id = id;
      return info;
    }
  }
  delete info;
  return NULL;
}

long  __cdecl usbpv_close(void* handle)
{
  if (handle) {
    info_t* info = (info_t*)handle;
    info->run = false;

    WaitForSingleObject(info->h, 1000);
    delete info;
  }
  return 0;
}
