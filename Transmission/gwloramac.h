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


using namespace std;

class GwLoraMac
{
public:

    static constexpr uint8_t ERR_ACK_NOT_EXPECTED_BUT_RECV = 0xF1;
    static constexpr uint8_t ERR_ACK_EXPECTED_BUT_NOT_RECV = 0xF2;
    static constexpr uint8_t ERR_TIMEOUT = 0xF3;
    static constexpr uint8_t ERR_FULL_FIFO = 0xF4;

    static GwLoraMac& instance();

    void initialize(void (*appTxDone)(void), void (*appRxDone)(void),
                    void (*appError)(uint8_t err));

    bool pushData(uint8_t destAddr, uint8_t * data, uint8_t len);
    void getLastData(uint8_t &srcAddr, uint8_t * data, uint8_t &len);

    static void (*_appTxDone)(void);
    static void (*_appRxDone)(void);
    static void (*_appError)(uint8_t err);

    static void _macTxDone();
    static void _macRxDone();
    static void _macTimeout();

private:

    GwLoraMac();

    PacketFifo  _packetsForClientFifo[MAX_ALLOWED_CLIENTS];
    Packet      _sendPacket[MAX_ALLOWED_CLIENTS];
    Packet      _recvPacket;
    Lora        _radio;

    uint16_t    _packetIDtoAcknowledge[MAX_ALLOWED_CLIENTS];
    uint16_t    _currentPktID[MAX_ALLOWED_CLIENTS];
    bool        _nextPktIsACK[MAX_ALLOWED_CLIENTS];

    bool        _packetOnWaiting;
    uint8_t     _addressOfClientWaitingForPacket;
};

#endif // GWMAC_H
