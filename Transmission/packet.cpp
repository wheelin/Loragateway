#include "packet.h"
#include <string.h>

// Packet fields
#define DST_FIELD               0
#define SRC_FIELD               1
#define PAYLOAD_LENGTH_FIELD    2
#define TIMESTAMP_FIELD         3
#define PACKET_ID_FIELD         7
#define FLAG_FIELD              9

#define FLG_ACK                 0
#define FLG_ACK_MASK            0x01

#define FLG_ACK_NXT_PKT         1
#define FLG_ACK_NXT_PKT_MASK    0x02

#define FLG_LAST_PKT            2
#define FLG_LAST_PKT_MASK       0x04

#define PAYLOAD_START           10

packet::packet()
{
}

void packet::set_src_address(uint8_t src)
{
    _buffer[SRC_FIELD] = src;
}

uint8_t packet::get_src_address()
{
    return _buffer[SRC_FIELD];
}

void packet::set_dst_address(uint8_t dst)
{
    _buffer[DST_FIELD] = dst;
}

uint8_t packet::get_dst_address()
{
    return _buffer[DST_FIELD];
}

void packet::set_timestamp(uint32_t timestamp)
{
    _buffer[TIMESTAMP_FIELD]     =  ((uint8_t)((timestamp >> 24) && 0xFF));
    _buffer[TIMESTAMP_FIELD + 1] =  ((uint8_t)((timestamp >> 16) && 0xFF));
    _buffer[TIMESTAMP_FIELD + 2] =  ((uint8_t)((timestamp >>  8) && 0xFF));
    _buffer[TIMESTAMP_FIELD + 3] =  ((uint8_t)((timestamp)       && 0xFF));
}

uint32_t packet::get_timestamp()
{
    return  (((uint32_t)_buffer[TIMESTAMP_FIELD])     << 24) +
            (((uint32_t)_buffer[TIMESTAMP_FIELD + 1]) << 16) +
            (((uint32_t)_buffer[TIMESTAMP_FIELD + 2]) <<  8) +
            (((uint32_t)_buffer[TIMESTAMP_FIELD + 3]));
}

void packet::set_as_ACK(bool ack)
{
    if(ack)
    {
        _buffer[FLAG_FIELD] |= 1 << FLG_ACK;
    }
    else
    {
        _buffer[FLAG_FIELD] &= ~(1 << FLG_ACK);
    }
}

bool packet::is_ACK()
{
    if((_buffer[FLAG_FIELD] & FLG_ACK_MASK) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void packet::set_packet_ID_to_acknowledge(uint16_t pktID)
{
    if(is_ACK())
    {
        _buffer[PAYLOAD_START] = ((uint8_t)(pktID >> 8));
        _buffer[PAYLOAD_START + 1] = ((uint8_t)(pktID));
        _buffer[PAYLOAD_LENGTH_FIELD] = 1;
    }
}

uint16_t packet::get_packet_ID_to_acknowledge()
{
    uint16_t id;
    if(is_ACK())
    {
        id = (((uint16_t)_buffer[PAYLOAD_START]) << 8);
        id += ((uint16_t)_buffer[PAYLOAD_START + 1]);
        return id;
    }
    return 0;
}

void packet::set_ACK_on_next_packet_required(bool required)
{
    if(required)
    {
        _buffer[FLAG_FIELD] |= (1 << FLG_ACK_NXT_PKT);
    }
    else
    {
        _buffer[FLAG_FIELD] &= ~(1 << FLG_ACK_NXT_PKT);
    }
}

bool packet::is_ACK_on_packet_required()
{
    if((_buffer[FLAG_FIELD] & FLG_ACK_NXT_PKT_MASK) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void packet::set_as_last_session_packet(bool last)
{
    if(last)
    {
        _buffer[FLAG_FIELD] |= (1 << FLG_LAST_PKT);
    }
    else
    {
        _buffer[FLAG_FIELD] &= ~(1 << FLG_LAST_PKT);
    }
}

bool packet::is_last_session_packet()
{
    if((_buffer[FLAG_FIELD] & FLG_LAST_PKT_MASK) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void packet::set_packet_ID(uint16_t id)
{
    _buffer[PACKET_ID_FIELD]        = (uint8_t)((id >> 8) & 0xFF);
    _buffer[PACKET_ID_FIELD + 1]    = (uint8_t)(id & 0xFF);
}

uint8_t packet::get_packet_length()
{
    return _buffer[PAYLOAD_LENGTH_FIELD] + PAYLOAD_START;
}

uint8_t packet::get_payload_length()
{
    return _buffer[PAYLOAD_LENGTH_FIELD];
}

uint16_t packet::get_packet_ID()
{
    return ((uint16_t)(_buffer[PACKET_ID_FIELD << 8]) + (uint16_t)(_buffer[PACKET_ID_FIELD + 1]));
}

uint8_t * packet::get_buffer_ptr()
{
    return _buffer;
}

uint8_t packet::set_payload(uint8_t * buf, uint8_t len)
{
    if(len >= (MAX_PACKET_LEN - PAYLOAD_START))
    {
        return -1;
    }
    _buffer[PAYLOAD_LENGTH_FIELD] = len;
    memcpy(get_payload_ptr(), buf, len);
    return 0;
}

uint8_t * packet::get_payload_ptr()
{
    return &_buffer[PAYLOAD_START];
}

void packet::clear_payload()
{
    memset(&_buffer[PAYLOAD_START], 0, MAX_PACKET_LEN - PAYLOAD_START);
    _buffer[PAYLOAD_LENGTH_FIELD] = 0;
}
