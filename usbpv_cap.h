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

/** Wait or get a pipe transfer state
 *
 *  \ingroup Group_Host
 *
 *  \param[in] pipe           Pipe pointer, initial by \ref tusb_pipe_open
 *  \param[in] timeout        0 - don't wait, just get the current pipe state
 *                            0xffffffff - wait forever
 *  \return    Channel state, type is \ref channel_state_t
 */

/** callback when got packet
 * 
 * \param[in] context      passed from usbpv_open
 * \param[in] ts           timestamp in second
 * \param[in] nano         timestamp in nano second
 * \param[in] data         packet data
 * \param[in] len          packet length
 * \param[in] status       status < 0, means error, data parameter holds error description, the capture thread will stop on error
 *                         status >= 0 means success
 * 
 * \return not used
 */
typedef long  (__cdecl* pfn_packet_handler)(void* context, unsigned long ts, unsigned long nano, const void* data, unsigned long len, long status);

// option format
// "<TYPE,NAME[,PARAM[,PARAM[,PARAM]]]><TYPE,NAME[,PARAM[,PARAM[,PARAM]]]>..."
// valid TYPE:   select/text/int/file
// option result format
// <NAME,DATA><NAME,DATA>...

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

/** get capture option
 * 
 * \param[out] option    capture option
 * \param[in]  length    option buffer length
 * 
 * \return actual  option content length
 */
USBPV_API long  __cdecl usbpv_get_option(char* option, long length);

/** open capture
 * 
 * \param[in]  option
 * \param[in]  context used for callback
 * \param[in]  callback function when packet received
 * 
 * \return capture handler, null for fail, more error info can be set by callback
 */
USBPV_API void* __cdecl usbpv_open(const char* option, void* context, pfn_packet_handler callback);

/** close capture
 * 
 * \param[in]  handle   return bu usbpv_open
 * 
 * \return not used
 */
USBPV_API long  __cdecl usbpv_close(void* handle);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __USBPV_CAP_H__
