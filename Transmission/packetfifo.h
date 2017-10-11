#ifndef PACKETFIFO_H
#define PACKETFIFO_H

#include <stdint-gcc.h>
#include "packet.h"

#define MAX_FIFO_PKTS   20

class packet_fifo
{
public:
    packet_fifo();

    bool is_empty();
    bool push_packet(packet * pkt);
    bool next_packet(packet &pkt);

private:
    packet  _pkt_fifo[MAX_FIFO_PKTS];
    bool    _free_pkt[MAX_FIFO_PKTS];
};

#endif // PACKETFIFO_H
