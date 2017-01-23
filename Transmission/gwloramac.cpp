#include "gwloramac.h"
#include "packet.h"
#include "lora.h"
#include <string.h>
#include "packetfifo.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

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
        printf("ERROR >> cannot initialize radio : %d\n", ret);
        exit(-1);
    }
    if((ret = _radio.setMainParameters(CH_11_868, BW_500, CR_5, SF_12)) != 0)
    {
        printf("ERROR >> cannot set radio main parameters. Code error : %d\n", ret);
        exit(-1);
    }
    if((ret = _radio.enableCRCCheck(true)) != 0)
    {
        printf("ERROR >> Cannot enable crc check : %d\n", ret);
        exit(-1);
    }
    if((ret = _radio.enableImplicitHeader(false)) != 0)
    {
        printf("ERROR >> Cannot disable implicit header : %d\n", ret);
        exit(-1);
    }
    if((ret = _radio.setOutputPower(15)) != 0)
    {
        printf("ERROR >> Cannot set output power : %d\n", ret);
        exit(-1);
    }
    if((ret = _radio.setCallbacks(_macTxDone, _macRxDone, _macTimeout)) != 0)
    {
        printf("ERROR >> Cannot set callbacks : %d\n", ret);
        exit(-1);
    }

    _radio.setInContinuousReceiveMode();
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
        printf("ERROR >> fifo %d is full of messages\n", destAddr);
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
    srcAddr = instance()._recvPacket.getSrcAddress();
    len = instance()._recvPacket.getPayloadLength();
    memcpy(data, instance()._recvPacket.getPayloadPtr(), len);
}

void GwLoraMac::_macTxDone()
{
    int ret;
    if(instance()._packetOnWaiting)
    { // send packet when the radio module is ready
        instance()._sendPacket[instance()._addressOfClientWaitingForPacket].setTimestamp((uint32_t)time(0));
        if((ret = instance()._radio.send(instance()._sendPacket[instance()._addressOfClientWaitingForPacket].getBufferPtr(),
                                         instance()._sendPacket[instance()._addressOfClientWaitingForPacket].getPacketLength())) != 0)
        {
            printf("ERROR >> Cannot send data to radio module : %d\n", ret);
            if(_appError != NULL)
            {
                _appError(ret);
            }
        }
        instance()._nextPktIsACK[instance()._addressOfClientWaitingForPacket] = true;
    }
}

void GwLoraMac::_macRxDone()
{
    bool receivedPacketForApp = false;
    int ret = 0;
    Packet pkt;
    if((ret = instance()._radio.getReceivedData(pkt.getBufferPtr())) != 0)
    {
        printf("ERROR >> cannot receive data from the radio module : %d\n", ret);
        if(_appError != NULL)
        {
            _appError(ret);
        }
        return;
    }
    uint8_t currentAddr = pkt.getSrcAddress();
    if(pkt.isACK() && !instance()._nextPktIsACK[currentAddr])
    { // ack expected but no ack came back
        if(_appError != NULL)
        {
            _appError(ERR_ACK_NOT_EXPECTED_BUT_RECV);
        }
        return;
    }
    else if(!pkt.isACK() && instance()._nextPktIsACK[currentAddr])
    { // ack not expected but received
        if(_appError != NULL)
        {
            _appError(ERR_ACK_EXPECTED_BUT_NOT_RECV);
        }
        return;
    }
    else if(pkt.isACK() && instance()._nextPktIsACK[currentAddr])
    { // ack expected and received. this is end of this session
        return;
    }
    else
    { // this is a data packet, so we put this as the next packet handled by the application
        memcpy(instance()._recvPacket.getBufferPtr(), pkt.getBufferPtr(), pkt.getPacketLength());
        receivedPacketForApp = true;
    }

    if(instance()._packetsForClientFifo[currentAddr].isEmpty())
    { // no packet has to be sent to the current client
      // so the gateway must send ACK_END to tell the client this conversation is
      // over.
        instance()._sendPacket[currentAddr].clearPayload();
        instance()._sendPacket[currentAddr].setDstAddress(currentAddr);
        instance()._sendPacket[currentAddr].setSrcAddress(GW_ADDR);
        instance()._sendPacket[currentAddr].setAsACK(true);
        instance()._sendPacket[currentAddr].setAsLastSessionPacket(true);
        instance()._sendPacket[currentAddr].setTimestamp((uint32_t)time(0));
        instance()._radio.send(instance()._sendPacket[currentAddr].getBufferPtr(), instance()._sendPacket[currentAddr].getPacketLength());
    }
    else
    { // send ACK_NEND and then, set next packet to send as a waiting packet. It waits for the _ACK_NEND packet
      // to be sent by the radio module and then is is sent in the _macTxDone function
        instance()._sendPacket[currentAddr].clearPayload();
        instance()._sendPacket[currentAddr].setDstAddress(currentAddr);
        instance()._sendPacket[currentAddr].setSrcAddress(GW_ADDR);
        instance()._sendPacket[currentAddr].setAsACK(true);
        instance()._sendPacket[currentAddr].setAsLastSessionPacket(false);
        instance()._sendPacket[currentAddr].setTimestamp((uint32_t)time(0));
        if((ret = instance()._radio.send(instance()._sendPacket[currentAddr].getBufferPtr(), instance()._sendPacket[currentAddr].getPacketLength())) != 0)
        {
            printf("ERROR >> Cannot send data to radio module : %d\n", ret);
            if(_appError != NULL)
            {
                _appError(ret);
            }
            return;
        }

        // store the next packet into the packet to send container
        instance()._packetsForClientFifo[currentAddr].nextPacket(instance()._sendPacket[currentAddr]);
    }

    if(receivedPacketForApp)
    { // now, if a packet has been received, we advertise the application
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
