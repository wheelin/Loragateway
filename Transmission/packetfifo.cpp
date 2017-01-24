#include "packetfifo.h"
#include <string.h>

PacketFifo::PacketFifo()
{
    _freePkt[0] = true;
}

bool PacketFifo::isEmpty()
{
    return _freePkt[0];
}

bool PacketFifo::pushPacket(Packet *pkt)
{
    int i = 0;
    while (_freePkt[i] == false && i < MAX_FIFO_PKTS)
    {
        i++;
    }
    if((i == (MAX_FIFO_PKTS - 1)) && (_freePkt[i] == false))
    {
        return false;
    }
    memcpy(_pktFifo[i].getBufferPtr(), pkt->getBufferPtr(), MAX_PACKET_LEN);
    _freePkt[i] = false;
    return true;
}

bool PacketFifo::nextPacket(Packet &pkt)
{
    if(_freePkt[0] == true)
    {
        return false;
    }
    memcpy(pkt.getBufferPtr(), _pktFifo[0].getBufferPtr(), MAX_PACKET_LEN);
    for(int i = 1; i < MAX_FIFO_PKTS; i++)
    {
        _freePkt[i - 1] = _freePkt[i];
        memcpy(_pktFifo[i - 1].getBufferPtr(), _pktFifo[i].getBufferPtr(), MAX_PACKET_LEN);
    }
    _freePkt[MAX_FIFO_PKTS - 1] = true;
    return true;
}
