#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include <stdint.h>

#define MAX_PACKET_LEN          256
#define FIELDS_NUMBER           6

class Packet
{
public:
    Packet();

    void        setSrcAddress(uint8_t src);
    uint8_t     getSrcAddress();

    void        setDstAddress(uint8_t dst);
    uint8_t     getDstAddress();

    void        setTimestamp(uint32_t timestamp);
    uint32_t    getTimestamp();

    void        setAsACK(bool ack);
    bool        isACK();
    void        setPacketIDToAcknowledge(uint16_t pktID);
    uint16_t    getPacketIDToAcknowledge();
    void        setACKOnNextPacketRequired(bool required);
    bool        isACKOnPacketRequired();

    void        setAsLastSessionPacket(bool last);
    bool        isLastSessionPacket();

    void        setPacketID(uint16_t id);
    uint16_t    getPacketID();

    uint8_t     getPacketLength();
    uint8_t     getPayloadLength();

    uint8_t *   getBufferPtr();

    uint8_t     setPayload(uint8_t * buf, uint8_t len);
    uint8_t *   getPayloadPtr();

    void        clearPayload();
protected:
    uint8_t     _buffer[MAX_PACKET_LEN];
};

#endif /* PACKET_H_INCLUDED */
