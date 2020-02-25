# USB Packet Viewer demo
Demo plugins for [USB Packet Viewer](http://pv.tusb.org).

## Packet Capture
A dll for capture data, must implement 1 lua function, 3 dll function.
### Function in lua
If there are more than one capture sources, USB packet viewer will display a capture select dialog.
```lua
local captureList = {
    "demoCap", -- demoCap.dll
}
function valid_capture()
    return table.concat(captureList, ",")
end
```
### Function in dll
``` C
typedef long  (__cdecl* pfn_packet_handler)(void* context, unsigned long ts, unsigned long nano, const void* data, unsigned long len, long status);

USBPV_API long  __cdecl usbpv_get_option(char* option, long length);

USBPV_API void* __cdecl usbpv_open(const char* option, void* context, pfn_packet_handler callback);

USBPV_API long  __cdecl usbpv_close(void* handle);
```


## File Read/Write
File read/write use lua script, must implement 3 function, valid_filter, open_file, write_file.

```lua
function valid_filter()
    return "example file (*.example);;All files (*.*)"
end
function open_file(name, packet_handler, context)
    --  open and parse the file
    --  packet_handler(context, ts, nano, packet, status or 0, current, total)
    return count -- total packet count in this file
end
function write_file(name, packet_handler, context)
    local count = 0
    while true do
        local ts, nano, packet, status = packet_handler(context)
        if ts and nano and packet then
            status = status or 0
            -- TODO: write packet to file
            count = count + 1
        else
            break
        end
    end
    return count
end
```

## Packet Parser
File read/write use lua script, must implement 3 function, parser_reset, parser_append_packet, parser_get_info.
```lua
function parser_reset()
    -- reset parser state
    -- no return value
end
function parser_append_packet(ts, nano, pkt, status, id, transId, graph_handler, context)
    -- parse the packt
    -- append parse result to graph view, see scripts.lua for graph description format detail
    graph_handler(context, graph_description, transId, id, id1, id2, id3)
    -- no return value
end
function parser_get_info(id1, id2, id3)
    -- id1, id2, id3,  id of the graph element
    -- data, data to display in data view
    -- html, html info to display in decode view
    return data, html
end
```

