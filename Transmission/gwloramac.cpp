#include "gwloramac.h"
#include "packet.h"
#include "lora.h"
#include <string.h>
#include "packetfifo.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void (*gw_lora_mac::_app_tx_done)() = 0;
void (*gw_lora_mac::_app_rx_done)() = 0;
void (*gw_lora_mac::_app_error)(uint8_t err) = 0;

gw_lora_mac::gw_lora_mac()
{
}

gw_lora_mac &gw_lora_mac::instance()
{
    static gw_lora_mac INSTANCE;
    return INSTANCE;
}

void gw_lora_mac::initialize(void (*atd)(), void (*ard)(),
                           void (*ae)(uint8_t))
{
    int ret;
    gw_lora_mac::_app_tx_done = atd;
    gw_lora_mac::_app_rx_done = ard;
    gw_lora_mac::_app_error = ae;

    if((ret = _radio.on()))
    {
        exit(-1);
    }
    if((ret = _radio.set_main_parameters(CH_11_868, BW_500, CR_5, SF_12)) != 0)
    {
        exit(-1);
    }
    if((ret = _radio.enable_crc_check(true)) != 0)
    {
        exit(-1);
    }
    if((ret = _radio.enable_implicit_header(false)) != 0)
    {
        exit(-1);
    }
    if((ret = _radio.set_output_power(15)) != 0)
    {
        exit(-1);
    }
    if((ret = _radio.set_callbacks(_mac_tx_done, _mac_rx_done, _mac_timeout)) != 0)
    {
        exit(-1);
    }

    listen();
}

bool gw_lora_mac::push_data(uint8_t destAddr, uint8_t *data, uint8_t len)
{
    packet pkt;
    pkt.set_dst_address(destAddr);
    pkt.set_timestamp((uint32_t)time(0));
    pkt.set_packet_ID(_current_pkt_ID[destAddr]++);
    pkt.set_ACK_on_next_packet_required(true);
    pkt.set_as_ACK(false);
    pkt.clear_payload();
    pkt.set_src_address(GW_ADDR);
    pkt.set_as_last_session_packet(true);
    pkt.set_payload(data, len);

    if(!_packets_for_client_fifo[destAddr].push_packet(&pkt))
    {
        if(_app_error != NULL)
        {
            _app_error(ERR_FULL_FIFO);
        }
        return false;
    }
    return true;
}

void gw_lora_mac::get_last_data(uint8_t &srcAddr, uint8_t * data, uint8_t &len)
{
    srcAddr = instance()._recv_packet.get_src_address();
    len = instance()._recv_packet.get_payload_length();
    memcpy(data, instance()._recv_packet.get_payload_ptr(), len);
}

void gw_lora_mac::listen()
{
    _radio.listen();
}

gw_lora_mac::radio_state_t gw_lora_mac::get_radio_mode()
{
    int rm = _radio.get_mode();
    switch(rm)
    {
    case LORA_CAD_DETECTION_MODE:
        return radio_state_t::CAD;
    case LORA_FREQ_RX_SYNTH_MODE:
        return radio_state_t::FSRx;
    case LORA_FREQ_TX_SYNTH_MODE:
        return radio_state_t::FSTx;
    case LORA_RX_MODE:
        return radio_state_t::RxContinuous;
    case LORA_RX_SINGLE_MODE:
        return radio_state_t::RxSingle;
    case LORA_SLEEP_MODE:
        return radio_state_t::Sleep;
    case LORA_STANDBY_MODE:
        return radio_state_t::Stand_by;
    case LORA_TX_MODE:
        return radio_state_t::Tx;
    }
    return radio_state_t::Undefined;
}

void gw_lora_mac::_mac_tx_done()
{
    if(instance()._packet_on_waiting)
    { // send packet when the radio module is ready
        instance()._packet_on_waiting = false;
        instance()._sent_packet[instance()._address_of_client_waiting_for_packet].set_timestamp((uint32_t)time(0));
        instance()._radio.send(instance()._sent_packet[instance()._address_of_client_waiting_for_packet].get_buffer_ptr(),
                                         instance()._sent_packet[instance()._address_of_client_waiting_for_packet].get_packet_length());
        instance()._next_pkt_is_ACK[instance()._address_of_client_waiting_for_packet] = true;
        return;
    }
    if(_app_tx_done != NULL)
    {
        _app_tx_done();
    }
    instance().listen();
}

void gw_lora_mac::_mac_rx_done()
{
    bool received_packet_for_app = false;
    int ret = 0;
    packet pkt;
    if((ret = instance()._radio.get_received_data(pkt.get_buffer_ptr())) != 0)
    {
        if(_app_error != NULL)
        {
            _app_error(ret);
        }
        instance().listen();
        return;
    }
    else
    {
        instance()._snr = instance()._radio.get_snr();
        instance()._pkt_rssi = instance()._radio.get_pkt_rssi();
        instance()._rssi = instance()._radio.get_rssi();
    }
    uint8_t current_addr = pkt.get_src_address();
    if(current_addr > MAX_CLIENT_ADDRESS)
    {
        if(_app_error != NULL)
        {
            _app_error(ERR_INVALID_ADDR);
        }
        instance().listen();
        return;
    }
    uint8_t current_node_idx = current_addr - 1;
    if(pkt.is_ACK() && !instance()._next_pkt_is_ACK[current_node_idx])
    { // ack expected but no ack came back
        if(_app_error != NULL)
        {
            _app_error(ERR_ACK_NOT_EXPECTED_BUT_RECV);
        }
        instance().listen();
        return;
    }
    else if(!pkt.is_ACK() && instance()._next_pkt_is_ACK[current_node_idx])
    { // ack not expected but received
        if(_app_error != NULL)
        {
            _app_error(ERR_ACK_EXPECTED_BUT_NOT_RECV);
        }
        instance().listen();
        return;
    }
    else if(pkt.is_ACK() && instance()._next_pkt_is_ACK[current_node_idx])
    { // ack expected and received. this is end of this session
        instance().listen();
        return;
    }
    else
    { // this is a data packet, so we put this as the next packet handled by the application
        memcpy(instance()._recv_packet.get_buffer_ptr(), pkt.get_buffer_ptr(), pkt.get_packet_length());
        received_packet_for_app = true;
    }

    instance()._sent_packet[current_node_idx].clear_payload();
    instance()._sent_packet[current_node_idx].set_dst_address(current_addr);
    instance()._sent_packet[current_node_idx].set_src_address(GW_ADDR+1);
    instance()._sent_packet[current_node_idx].set_as_ACK(true);
    instance()._sent_packet[current_node_idx].set_timestamp((uint32_t)time(0));
    uint8_t id[] = {(uint8_t)((pkt.get_packet_ID() >> 8) & 0xFF), (uint8_t)((pkt.get_packet_ID()) & 0xFF)};
    instance()._sent_packet[current_node_idx].set_payload(id, 2);
    if(instance()._packets_for_client_fifo[current_node_idx].is_empty())
    { // no packet has to be sent to the current client
      // so the gateway must send ACK_END to tell the client this conversation is
      // over.
        instance()._sent_packet[current_node_idx].set_as_last_session_packet(true);
        instance()._radio.send(instance()._sent_packet[current_node_idx].get_buffer_ptr(),
                               instance()._sent_packet[current_node_idx].get_packet_length());
    }
    else
    { // send ACK_NEND and then, set next packet to send as a waiting packet. It waits for the _ACK_NEND packet
      // to be sent by the radio module and then is is sent in the _macTxDone function
        instance()._sent_packet[current_node_idx].set_as_last_session_packet(false);
        instance()._radio.send(instance()._sent_packet[current_node_idx].get_buffer_ptr(),
                               instance()._sent_packet[current_node_idx].get_packet_length());
        // store the next packet into the packet to send container
        instance()._packet_on_waiting = true;
        instance()._packets_for_client_fifo[current_node_idx].next_packet(instance()._sent_packet[current_node_idx]);
    }

    if(received_packet_for_app)
    { // now, if a packet has been received, we advertise the application
        received_packet_for_app = false;
        if(_app_rx_done != NULL)
        {
            _app_rx_done();
        }
    }
}

void gw_lora_mac::_mac_timeout()
{
    gw_lora_mac::_app_error(ERR_TIMEOUT);
}

int gw_lora_mac::get_snr()
{
    return instance()._snr;
}

int gw_lora_mac::get_pkt_rssi()
{
    return instance()._pkt_rssi;
}

int gw_lora_mac::get_rssi()
{
    return instance()._rssi;
}
