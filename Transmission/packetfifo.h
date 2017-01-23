#ifndef PACKETFIFO_H
#define PACKETFIFO_H

#include <stdint-gcc.h>
#include "packet.h"

#define MAX_FIFO_PKTS   20

class PacketFifo
{
public:
    PacketFifo();

    bool isEmpty();
    bool pushPacket(Packet * pkt);
    bool nextPacket(Packet &pkt);

private:
    Packet  _pktFifo[MAX_FIFO_PKTS];
    bool    _freePkt[MAX_FIFO_PKTS];
};

#endif // PACKETFIFO_H
