#include "windows.h"
#include "usbpv_cap.h"
#include "string.h"
#include "stdio.h"



struct info_t{
  HANDLE h;
  void* context;
  pfn_packet_handler cb;
  DWORD id;
  volatile bool run;
  int current_speed;
  int mul;
  FILE* fp;
};

struct Record{
  UINT32 ts;
  UINT32 nano;
  UINT32 act_len;
  UINT32 org_len;
};

char buf[4096];

DWORD WINAPI thread(
  LPVOID param
  )
{
  info_t* info = (info_t*)param;
  int count = 0;

  while (info->run) {

    if (info->fp) {
      do {
        Record rec;
        if (fread(&rec, 1, sizeof(rec), info->fp) != sizeof(rec)) {
          break;
        }
        if (fread(buf, 1, rec.act_len, info->fp) != rec.act_len) {
          break;
        }
        info->cb(info->context, rec.ts, rec.nano, buf, rec.act_len, info->current_speed);
      } while (info->run);
      static const char msg[] = "File end";
      info->cb(info->context, GetCurrentTime(), 0, msg, sizeof(msg), -1);
      break;
    }
    else {
      UINT8 data[] = { 0xb4,0x01,0x02 };
      if (info->run) {
        count++;
        if (count > 10) {
          static const char msg[] = "Too many data";
          info->cb(info->context, GetCurrentTime(), 0, msg, sizeof(msg), -1);
        }
        else {
          info->cb(info->context, GetCurrentTime(), 0, data, 3, info->current_speed);
        }
      }
      Sleep(200);
    }
  }
  static const char msg[] = "Demo capture thread need quit";
  //info->cb(info->context, GetCurrentTime(), 0, msg, sizeof(msg), -2);
  return 0;
}

static const char demoOption[] = "<select,speed,HS,FS,LS>"
"<file,DataFile,,Pcap File (*.pcap);;All files (*.*)>";
long  __cdecl usbpv_get_option(char* option, long length)
{
  strcpy_s(option, length, demoOption);
  return sizeof(sizeof(demoOption));
}

struct PcapHeader{
  UINT32 magic_number;   /* magic number */
  UINT16 version_major;  /* major version number */
  UINT16 version_minor;  /* minor version number */
  UINT32 thiszone;       /* GMT to local correction */
  UINT32 sigfigs;        /* accuracy of timestamps */
  UINT32 snaplen;        /* max length of captured packets, in octets */
  UINT32 network;        /* data link type */
};

#define MS_MAGIC 0xa1b2c3d4
#define NS_MAGIC 0xa1b23c4d
#define DLT_USBLL 288
static bool checkpacp_header(info_t* info)
{
  PcapHeader header;
  fread(&header, sizeof(header), 1, info->fp);
  if (header.network != DLT_USBLL) {
    return false;
  }
  if (header.magic_number == MS_MAGIC) {
    info->mul = 1000;
    return true;
  }
  if (header.magic_number == NS_MAGIC) {
    info->mul = 1;
    return true;
  }
  return false;
}

static bool parse_option(const char* option, info_t* info)
{
  info->fp = NULL;

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
  if (strncmp(speed, "HS", 2) == 0) {
    info->current_speed = PACKET_STATUS_SPEED_HIGH;
  }else if (strncmp(speed, "FS", 2) == 0) {
    info->current_speed = PACKET_STATUS_SPEED_FULL;
  }
  else if (strncmp(speed, "LS", 2) == 0) {
    info->current_speed = PACKET_STATUS_SPEED_LOW;
  }else{
    REPORT_ERROR("speed not valid, got %c%c", speed[0], speed[1]);
    return false;
  }

  const char* file = strstr(option, "<DataFile,");
  if (!file) {
    REPORT_ERROR("DataFile option not set");
    return false;
  }
  file += 10;
  if (strncmp(file, "test", 4) != 0) {
    char fname[MAX_PATH];
    strcpy_s(fname, MAX_PATH, file);
    fname[strlen(fname) - 1] = 0;
    errno_t e = fopen_s(&info->fp, fname, "rb");
    if (e != 0) {
      info->fp = NULL;
      REPORT_ERROR("File to open %s", fname);
      return false;
    }
    if (!checkpacp_header(info)) {
      fclose(info->fp);
      info->fp = NULL;
      REPORT_ERROR("Unknown file format %s", fname);
      return false;
    }
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
    if (info->fp != NULL) {
      fclose(info->fp);
    }
    delete info;
  }
  return 0;
}
