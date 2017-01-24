#include "gwloramac.h"
#include "packet.h"
#include "lora.h"
#include <string.h>
#include "packetfifo.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "Log/log.h"

void (*GwLoraMac::_appTxDone)() = 0;
void (*GwLoraMac::_appRxDone)() = 0;
void (*GwLoraMac::_appError)(uint8_t err) = 0;

GwLoraMac::GwLoraMac()
{
}

GwLoraMac &GwLoraMac::instance()
{
    static GwLoraMac INSTANCE;
    return INSTANCE;
}

void GwLoraMac::initialize(void (*appTxDone)(), void (*appRxDone)(),
                           void (*appError)(uint8_t))
{
    int ret;
    GwLoraMac::_appTxDone = appTxDone;
    GwLoraMac::_appRxDone = appRxDone;
    GwLoraMac::_appError = appError;

    if((ret = _radio.on()))
    {
        Log::instance().message(Log::Severity::Error, false, "cannot initialize radio : %d", ret);
        exit(-1);
    }
    if((ret = _radio.setMainParameters(CH_11_868, BW_500, CR_5, SF_12)) != 0)
    {
        Log::instance().message(Log::Severity::Error, false, "cannot set radio main parameters. Code error : %d", ret);
        exit(-1);
    }
    if((ret = _radio.enableCRCCheck(true)) != 0)
    {
        Log::instance().message(Log::Severity::Error, false, "Cannot enable crc check : %d", ret);
        exit(-1);
    }
    if((ret = _radio.enableImplicitHeader(false)) != 0)
    {
        Log::instance().message(Log::Severity::Error, false, "Cannot disable implicit header : %d", ret);
        exit(-1);
    }
    if((ret = _radio.setOutputPower(15)) != 0)
    {
        Log::instance().message(Log::Severity::Error, false, "Cannot set output power : %d", ret);
        exit(-1);
    }
    if((ret = _radio.setCallbacks(_macTxDone, _macRxDone, _macTimeout)) != 0)
    {
        Log::instance().message(Log::Severity::Error, false, "Cannot set callbacks : %d", ret);
        exit(-1);
    }

    listen();
}

bool GwLoraMac::pushData(uint8_t destAddr, uint8_t *data, uint8_t len)
{
    Packet pkt;
    pkt.setDstAddress(destAddr);
    pkt.setTimestamp((uint32_t)time(0));
    pkt.setPacketID(_currentPktID[destAddr]++);
    pkt.setACKOnNextPacketRequired(true);
    pkt.setAsACK(false);
    pkt.clearPayload();
    pkt.setSrcAddress(GW_ADDR);
    pkt.setAsLastSessionPacket(true);
    pkt.setPayload(data, len);

    if(!_packetsForClientFifo[destAddr].pushPacket(&pkt))
    {
        Log::instance().message(Log::Severity::Error, false, "fifo %d is full of messages", destAddr);
        if(_appError != NULL)
        {
            _appError(ERR_FULL_FIFO);
        }
        return false;
    }
    return true;
}

void GwLoraMac::getLastData(uint8_t &srcAddr, uint8_t * data, uint8_t &len)
{
    Log::instance().message(Log::Severity::Note, false, "getting packet into application space");
    srcAddr = instance()._recvPacket.getSrcAddress();
    Log::instance().message(Log::Severity::Note, false, "srcAddr : 0x%02x", srcAddr);
    len = instance()._recvPacket.getPayloadLength();
    Log::instance().message(Log::Severity::Note, false, "len : %d bytes", len);
    memcpy(data, instance()._recvPacket.getPayloadPtr(), len);
    Log::instance().message(Log::Severity::Note, false, "finished getting packet into application space");
}

void GwLoraMac::listen()
{
    _radio.setInContinuousReceiveMode();
}

GwLoraMac::RadioState GwLoraMac::getRadioMode()
{
    int rm = _radio.getMode();
    switch(rm)
    {
    case LORA_CAD_DETECTION_MODE:
        return RadioState::CAD;
    case LORA_FREQ_RX_SYNTH_MODE:
        return RadioState::FSRx;
    case LORA_FREQ_TX_SYNTH_MODE:
        return RadioState::FSTx;
    case LORA_RX_MODE:
        return RadioState::RxContinuous;
    case LORA_RX_SINGLE_MODE:
        return RadioState::RxSingle;
    case LORA_SLEEP_MODE:
        return RadioState::Sleep;
    case LORA_STANDBY_MODE:
        return RadioState::Stand_by;
    case LORA_TX_MODE:
        return RadioState::Tx;
    }
    return RadioState::Undefined;
}

void GwLoraMac::_macTxDone()
{
    Log::instance().message(Log::Severity::Success, false, "Entering tx done call back");
    if(instance()._packetOnWaiting)
    { // send packet when the radio module is ready
        instance()._sendPacket[instance()._addressOfClientWaitingForPacket].setTimestamp((uint32_t)time(0));
        instance()._radio.send(instance()._sendPacket[instance()._addressOfClientWaitingForPacket].getBufferPtr(),
                                         instance()._sendPacket[instance()._addressOfClientWaitingForPacket].getPacketLength());
        instance()._nextPktIsACK[instance()._addressOfClientWaitingForPacket] = true;
    }
    if(_appTxDone != NULL)
    {
        _appTxDone();
    }
    instance().listen();
}

void GwLoraMac::_macRxDone()
{
    bool receivedPacketForApp = false;
    int ret = 0;
    Packet pkt;
    if((ret = instance()._radio.getReceivedData(pkt.getBufferPtr())) != 0)
    {
        Log::instance().message(Log::Severity::Error, false, "cannot receive data from the radio module : %d", ret);
        if(_appError != NULL)
        {
            _appError(ret);
        }
        instance().listen();
        return;
    }
    else
    {
        Log::instance().message(Log::Severity::Success, false, "packet received");
        instance()._snr = instance()._radio.getSNR();
        Log::instance().message(Log::Severity::Note, false, "Radio SNR : %d dBm", instance()._snr);
        instance()._pktRssi = instance()._radio.getPktRSSI();
        Log::instance().message(Log::Severity::Note, false, "Packet RSSI : %d dBm", instance()._pktRssi);
        instance()._rssi = instance()._radio.getRSSI();
        Log::instance().message(Log::Severity::Note, false, "RSSI : %d dBm", instance()._rssi);
    }
    uint8_t currentAddr = pkt.getSrcAddress();
    Log::instance().message(Log::Severity::Note, false, "packet received from node address : 0x%02x", currentAddr);
    if(currentAddr > MAX_CLIENT_ADDRESS)
    {
        Log::instance().message(Log::Severity::Error, false, "node address is invalid");
        if(_appError != NULL)
        {
            _appError(ERR_INVALID_ADDR);
        }
        instance().listen();
        return;
    }
    uint8_t currentNodeIdx = currentAddr - 1;
    if(pkt.isACK() && !instance()._nextPktIsACK[currentNodeIdx])
    { // ack expected but no ack came back
        Log::instance().message(Log::Severity::Error, false, "ACK expected but packet was not ack");
        if(_appError != NULL)
        {
            _appError(ERR_ACK_NOT_EXPECTED_BUT_RECV);
        }
        instance().listen();
        return;
    }
    else if(!pkt.isACK() && instance()._nextPktIsACK[currentNodeIdx])
    { // ack not expected but received
        Log::instance().message(Log::Severity::Error, false, "ack not expected but received");
        if(_appError != NULL)
        {
            _appError(ERR_ACK_EXPECTED_BUT_NOT_RECV);
        }
        instance().listen();
        return;
    }
    else if(pkt.isACK() && instance()._nextPktIsACK[currentNodeIdx])
    { // ack expected and received. this is end of this session
        Log::instance().message(Log::Severity::Success, false, "ACK has been received and was expected");
        instance().listen();
        return;
    }
    else
    { // this is a data packet, so we put this as the next packet handled by the application
        Log::instance().message(Log::Severity::Success, false, "data packet received correctly, ID : %d", pkt.getPacketID());
        memcpy(instance()._recvPacket.getBufferPtr(), pkt.getBufferPtr(), pkt.getPacketLength());
        receivedPacketForApp = true;
    }

    instance()._sendPacket[currentNodeIdx].clearPayload();
    instance()._sendPacket[currentNodeIdx].setDstAddress(currentAddr);
    instance()._sendPacket[currentNodeIdx].setSrcAddress(GW_ADDR);
    instance()._sendPacket[currentNodeIdx].setAsACK(true);
    instance()._sendPacket[currentNodeIdx].setTimestamp((uint32_t)time(0));
    if(instance()._packetsForClientFifo[currentNodeIdx].isEmpty())
    { // no packet has to be sent to the current client
      // so the gateway must send ACK_END to tell the client this conversation is
      // over.
        Log::instance().message(Log::Severity::Note, false, "no packet for this node, sending ACK");
        instance()._sendPacket[currentNodeIdx].setAsLastSessionPacket(true);
        instance()._radio.send(instance()._sendPacket[currentNodeIdx].getBufferPtr(), instance()._sendPacket[currentNodeIdx].getPacketLength());
    }
    else
    { // send ACK_NEND and then, set next packet to send as a waiting packet. It waits for the _ACK_NEND packet
      // to be sent by the radio module and then is is sent in the _macTxDone function
        Log::instance().message(Log::Severity::Note, false, "Packet waiting, sending NEND_ACK");
        instance()._sendPacket[currentNodeIdx].setAsLastSessionPacket(false);
        instance()._radio.send(instance()._sendPacket[currentNodeIdx].getBufferPtr(), instance()._sendPacket[currentNodeIdx].getPacketLength());
        // store the next packet into the packet to send container
        instance()._packetOnWaiting = true;
        instance()._packetsForClientFifo[currentNodeIdx].nextPacket(instance()._sendPacket[currentNodeIdx]);
    }

    if(receivedPacketForApp)
    { // now, if a packet has been received, we advertise the application
        receivedPacketForApp = false;
        if(_appRxDone != NULL)
        {
            _appRxDone();
        }
    }
}

void GwLoraMac::_macTimeout()
{
    GwLoraMac::_appError(ERR_TIMEOUT);
}
