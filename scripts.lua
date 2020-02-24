-- scripts.lua
------------------------------------------
-- example for register capture dll
------------------------------------------
local captureList = {
    "demoCap", -- demoCap.dll
}

local PS_SPEED_UNKNOWN  = 0
local PS_SPEED_LOW      = 1
local PS_SPEED_FULL     = 2
local PS_SPEED_HIGH     = 3
local PS_SPEED_SUPER    = 4
local PS_SPEED_MASK     = 0x0f


function valid_capture()
    return table.concat(captureList, ",")
end
------------------------------------------
-- example for file read/write
------------------------------------------
function valid_filter()
    return "example file (*.example);;All files (*.*)"
end
function open_file(name, packet_handler, context)
    --  TODO: open and parse the file
    local ack = "\xd2"
    local nak = "\x5a"
    local pkt = ack
    local count = 10
    local status = PS_SPEED_FULL
    for i=1, count do
        local timestamp_in_second = 1
        local timestamp_in_nano_second = i
        -- pending here to got packet data will not block UI, the file reader is running in a background thread
        packet_handler(context, timestamp_in_second, timestamp_in_nano_second, pkt, status, i ,10)
        pkt = pkt == ack and nak or ack
    end
    return count
end
function write_file(name, packet_handler, context)
    local count = 0
    while true do
        local ts, nano, packet, status = packet_handler(context)
        if ts and nano and packet then
            -- TODO: write packet to file
            print(ts,nano,#packt, status or 0)
            count = count + 1
        else
            break
        end
    end
    return count
end
------------------------------------------
-- example for update graph viewer
------------------------------------------
local parser_info = nil
function parser_reset()
    parser_info = nil
end
---- Graph element format
--  total_width;(name1,data1,backgroundcolor,width[,textColor[,separator_width]])[;(name2,data2...)];flags
-- flags are defined as below, F_PACKET,F_TRANSACTION,F_XFER must near the ';' sign
local F_PACKET =     "("
local F_TRANSACTION= "["
local F_XFER       = "{"
local F_ACK =        "A"
local F_NAK =        "N"
local F_NYET =       "N"
local F_STALL =      "S"
local F_SOF =        "F"
local F_INCOMPLETE = "I"
local F_ISO =        "O"
local F_ERROR =      "E"
local F_PING  =      "P"

function parser_append_packet(ts, nano, pkt, status, id, transId, handler, context)
    if parser_info then return end
    local speed = (status or 0) & PS_SPEED_MASK
    parser_info = {}
    local topItem = "1000;(top name,top data,red,100);(top name2,top data2,blue,100,white,40);(top name3,top data3,yellow,100,red);" .. F_XFER
    local midItem = "1000;(mid name,mid data,red,100);(mid name2,mid data2,blue,100,white,40);(mid name3,mid data3,yellow,100,red);" .. F_TRANSACTION
    local botItem = "1000;(bot name,bot "..ts..",red,100);(bot name2,bot "..nano..",blue,100,white,40);(bot name3,bot data3,yellow,100,red);" .. F_PACKET
    local highlight = transId
    -- ADD ack items
    r = handler(context, topItem .. F_ACK, highlight, id, 1, -1, -1) -- top id:1
    highlight = highlight + 1
    r = handler(context, midItem .. F_ACK, highlight, id, 1, 1, -1) -- top id:1, mid id:1
    highlight = highlight + 1
    r = handler(context, botItem .. F_ACK, highlight, id, 1, 1, 1)  -- top 1d:1, mid 1d:1, bot id:1
    -- ADD ISO items
    highlight = highlight + 1
    r = handler(context, topItem .. F_ISO, highlight, id, 2, -1, -1) -- top id:2
    highlight = highlight + 1
    r = handler(context, midItem .. F_ISO, highlight, id, 2, 2, -1)  -- top id:2, mid id:2
    highlight = highlight + 1
    r = handler(context, botItem .. F_ISO, highlight, id, 2, 2, 2)   -- top id:2, mid id:2, bot id:2
    -- update exist item , change it's flag to NAK
    r = handler(context, botItem .. F_NAK, transId + 2, id, 1, 1, 1) -- top id:1, mid id:1, bot id:1, already exist, update it
end

function parser_get_info(id1, id2, id3)
    local item = "Top item, always transfer data"
    if id2 and id2>0 then item = "Middle Item, always transaction data" end
    if id3 and id3>0 then item = "Bottom Item, always packet data" end
    local html = "<h1>"..item.."</h1> The graph element ID is " .. string.format("%d,%d,%d",id1 or -1, id2 or -1,id3 or -1)
    local data = "example data \x11\x22\x33" .. html
    return data, html
end
