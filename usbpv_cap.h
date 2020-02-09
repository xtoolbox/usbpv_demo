#ifndef __USBPV_CAP_H__
#define __USBPV_CAP_H__


#ifdef _WIN32
#define USBPV_API __declspec(dllexport)
//#define USBPV_API __declspec(dllimport)
#else
#define OV_API
#endif


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
typedef long  (__cdecl* pfn_packet_handler)(void* context, unsigned long ts, unsigned long nano, const void* data, unsigned long len, long status);


// option format

// "<select,name1,option1[,option2[,option3]]><text,name2[,defaultValue]><int,name3[,defaultValue[,1[,100]]]><file,name4,defaultName[,demo file (*.demo)]>"
// data in [] means optional
// example:
// "<select,name1,option1,option2,option3><text,name2,hello><int,name3,23,1,100><file,name4,123.test,Test file(*.test)>"
// there are 4 option
//  index  | type   | name   | default value | filter/range/values
//    1    | select | name1  | option1       | option1,option2,option3
//    2    | text   | name2  | hello         | 
//    3    | int    | name3  | 23            | [1, 100]
//    4    | file   | name4  | 123.test      | Test file (*.test)
//  The result option pass to usbpv_open may
//  "<name1,option2><name2,world><name3,14><name4,xxx.test>"


USBPV_API long  __cdecl usbpv_get_option(char* option, long length);

USBPV_API void* __cdecl usbpv_open(const char* option, void* context, pfn_packet_handler callback);

USBPV_API long  __cdecl usbpv_close(void* handle);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __USBPV_CAP_H__
