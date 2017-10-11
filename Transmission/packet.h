#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include <stdint.h>

#define MAX_PACKET_LEN          256
#define FIELDS_NUMBER           6

class packet
{
public:
    packet();

    void        set_src_address(uint8_t src);
    uint8_t     get_src_address();

    void        set_dst_address(uint8_t dst);
    uint8_t     get_dst_address();

    void        set_timestamp(uint32_t timestamp);
    uint32_t    get_timestamp();

    void        set_as_ACK(bool ack);
    bool        is_ACK();
    void        set_packet_ID_to_acknowledge(uint16_t pktID);
    uint16_t    get_packet_ID_to_acknowledge();
    void        set_ACK_on_next_packet_required(bool required);
    bool        is_ACK_on_packet_required();

    void        set_as_last_session_packet(bool last);
    bool        is_last_session_packet();

    void        set_packet_ID(uint16_t id);
    uint16_t    get_packet_ID();

    uint8_t     get_packet_length();
    uint8_t     get_payload_length();

    uint8_t *   get_buffer_ptr();

    uint8_t     set_payload(uint8_t * buf, uint8_t len);
    uint8_t *   get_payload_ptr();

    void        clear_payload();
protected:
    uint8_t     _buffer[MAX_PACKET_LEN];
};

#endif /* PACKET_H_INCLUDED */
