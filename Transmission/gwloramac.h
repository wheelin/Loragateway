#ifndef GWMAC_H
#define GWMAC_H

#include <stdint-gcc.h>
#include <queue>
#include "packet.h"
#include "packetfifo.h"
#include "lora.h"

#define GW_ADDR                     1

#define MAX_ALLOWED_CLIENTS         10
#define MAX_CLIENT_ADDRESS          11

#define DEBUG 0

using namespace std;

class gw_lora_mac
{
public:

    static constexpr uint8_t ERR_ACK_NOT_EXPECTED_BUT_RECV = 0xF1;
    static constexpr uint8_t ERR_ACK_EXPECTED_BUT_NOT_RECV = 0xF2;
    static constexpr uint8_t ERR_TIMEOUT = 0xF3;
    static constexpr uint8_t ERR_FULL_FIFO = 0xF4;
    static constexpr uint8_t ERR_INVALID_ADDR = 0xF5;

    enum radio_state_t {
        Sleep,
        Stand_by,
        FSTx,
        FSRx,
        Tx,
        RxContinuous,
        RxSingle,
        CAD,
        Undefined,
    };

    static gw_lora_mac& instance();

    void initialize(void (*atd)(void), void (*ard)(void),
                    void (*ae)(uint8_t err));

    bool push_data(uint8_t dst_addr, uint8_t * data, uint8_t len);
    void get_last_data(uint8_t &src_addr, uint8_t * data, uint8_t &len);
    void listen();
    radio_state_t get_radio_mode();

    static void (*_app_tx_done)(void);
    static void (*_app_rx_done)(void);
    static void (*_app_error)(uint8_t err);

    static void _mac_tx_done();
    static void _mac_rx_done();
    static void _mac_timeout();

    int get_snr();
    int get_pkt_rssi();
    int get_rssi();

private:

    gw_lora_mac();

    packet_fifo _packets_for_client_fifo[MAX_ALLOWED_CLIENTS];
    packet      _sent_packet[MAX_ALLOWED_CLIENTS];
    packet      _recv_packet;
    lora        _radio;

    uint16_t    _packet_ID_to_acknowledge[MAX_ALLOWED_CLIENTS];
    uint16_t    _current_pkt_ID[MAX_ALLOWED_CLIENTS];
    bool        _next_pkt_is_ACK[MAX_ALLOWED_CLIENTS];

    bool        _packet_on_waiting;
    uint8_t     _address_of_client_waiting_for_packet;

    int         _snr;
    int         _pkt_rssi;
    int         _rssi;
};

#endif // GWMAC_H
