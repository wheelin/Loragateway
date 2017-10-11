#include "packetfifo.h"
#include <string.h>

packet_fifo::packet_fifo()
{
    _free_pkt[0] = true;
}

bool packet_fifo::is_empty()
{
    return _free_pkt[0];
}

bool packet_fifo::push_packet(packet *pkt)
{
    int i = 0;
    while (_free_pkt[i] == false && i < MAX_FIFO_PKTS)
    {
        i++;
    }
    if((i == (MAX_FIFO_PKTS - 1)) && (_free_pkt[i] == false))
    {
        return false;
    }
    memcpy(_pkt_fifo[i].get_buffer_ptr(), pkt->get_buffer_ptr(), MAX_PACKET_LEN);
    _free_pkt[i] = false;
    return true;
}

bool packet_fifo::next_packet(packet &pkt)
{
    if(_free_pkt[0] == true)
    {
        return false;
    }
    memcpy(pkt.get_buffer_ptr(), _pkt_fifo[0].get_buffer_ptr(), MAX_PACKET_LEN);
    for(int i = 1; i < MAX_FIFO_PKTS; i++)
    {
        _free_pkt[i - 1] = _free_pkt[i];
        memcpy(_pkt_fifo[i - 1].get_buffer_ptr(), _pkt_fifo[i].get_buffer_ptr(), MAX_PACKET_LEN);
    }
    _free_pkt[MAX_FIFO_PKTS - 1] = true;
    return true;
}
