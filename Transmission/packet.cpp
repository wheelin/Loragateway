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

Packet::Packet()
{
}

void Packet::setSrcAddress(uint8_t src)
{
    _buffer[SRC_FIELD] = src;
}

uint8_t Packet::getSrcAddress()
{
    return _buffer[SRC_FIELD];
}

void Packet::setDstAddress(uint8_t dst)
{
    _buffer[DST_FIELD] = dst;
}

uint8_t Packet::getDstAddress()
{
    return _buffer[DST_FIELD];
}

void Packet::setTimestamp(uint32_t timestamp)
{
    _buffer[TIMESTAMP_FIELD]     =  ((uint8_t)((timestamp >> 24) && 0xFF));
    _buffer[TIMESTAMP_FIELD + 1] =  ((uint8_t)((timestamp >> 16) && 0xFF));
    _buffer[TIMESTAMP_FIELD + 2] =  ((uint8_t)((timestamp >>  8) && 0xFF));
    _buffer[TIMESTAMP_FIELD + 3] =  ((uint8_t)((timestamp)       && 0xFF));
}

uint32_t Packet::getTimestamp()
{
    return  (((uint32_t)_buffer[TIMESTAMP_FIELD])     << 24) +
            (((uint32_t)_buffer[TIMESTAMP_FIELD + 1]) << 16) +
            (((uint32_t)_buffer[TIMESTAMP_FIELD + 2]) <<  8) +
            (((uint32_t)_buffer[TIMESTAMP_FIELD + 3]));
}

void Packet::setAsACK(bool ack)
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

bool Packet::isACK()
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

void Packet::setPacketIDToAcknowledge(uint16_t pktID)
{
    if(isACK())
    {
        _buffer[PAYLOAD_START] = ((uint8_t)(pktID >> 8));
        _buffer[PAYLOAD_START + 1] = ((uint8_t)(pktID));
        _buffer[PAYLOAD_LENGTH_FIELD] = 1;
    }
}

uint16_t Packet::getPacketIDToAcknowledge()
{
    uint16_t id;
    if(isACK())
    {
        id = (((uint16_t)_buffer[PAYLOAD_START]) << 8);
        id += ((uint16_t)_buffer[PAYLOAD_START + 1]);
        return id;
    }
    return 0;
}

void Packet::setACKOnNextPacketRequired(bool required)
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

bool Packet::isACKOnPacketRequired()
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

void Packet::setAsLastSessionPacket(bool last)
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

bool Packet::isLastSessionPacket()
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

void Packet::setPacketID(uint16_t id)
{
    _buffer[PACKET_ID_FIELD]        = (uint8_t)((id >> 8) & 0xFF);
    _buffer[PACKET_ID_FIELD + 1]    = (uint8_t)(id & 0xFF);
}

uint8_t Packet::getPacketLength()
{
    return _buffer[PAYLOAD_LENGTH_FIELD] + PAYLOAD_START;
}

uint8_t Packet::getPayloadLength()
{
    return _buffer[PAYLOAD_LENGTH_FIELD];
}

uint16_t Packet::getPacketID()
{
    return ((uint16_t)(_buffer[PACKET_ID_FIELD << 8]) + (uint16_t)(_buffer[PACKET_ID_FIELD + 1]));
}

uint8_t * Packet::getBufferPtr()
{
    return _buffer;
}

uint8_t Packet::setPayload(uint8_t * buf, uint8_t len)
{
    if(len >= (MAX_PACKET_LEN - PAYLOAD_START))
    {
        return -1;
    }
    _buffer[PAYLOAD_LENGTH_FIELD] = len;
    memcpy(getPayloadPtr(), buf, len);
    return 0;
}

uint8_t * Packet::getPayloadPtr()
{
    return &_buffer[PAYLOAD_START];
}

void Packet::clearPayload()
{
    memset(&_buffer[PAYLOAD_START], 0, MAX_PACKET_LEN - PAYLOAD_START);
    _buffer[PAYLOAD_LENGTH_FIELD] = 0;
}
