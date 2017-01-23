#include "lora.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "wiringPi.h"
#include "wiringPiSPI.h"
#include <pthread.h>
#include <unistd.h>

void (*Lora::_txDone)() = 0;
void (*Lora::_rxDone)() = 0;
void (*Lora::_timeout)() = 0;

Lora::Lora()
{
    _channel = CH_10_868;
    _bandwidth = BW_125;
    _codingRate = CR_5;
    _spreadingFactor = SF_12;
    _CRCCheckEnabled = true;
    _implicitHeaderEnabled = true;
    _pwrdB = 15;
    _continuousReceivingEnabled = false;
}

int Lora::on()
{
    wiringPiSetupPhys();
    _spiFd = wiringPiSPISetupMode(0, SPI_SPEED, SPI_MODE);
    if(_spiFd < 0)
    {
        return -1;
    }
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    pinMode(RST_PIN, INPUT);
    wiringPiISR(DIO0_PIN, INT_EDGE_RISING, onDIO0Interrupt);

    memset(_lastReceivedBuffer, 0, MAX_DATA_LEN);
    memset(_lastSentBuffer, 0, MAX_DATA_LEN);

    reset();

    // put module in fsk ook sleep mode
    writeReg(REG_OP_MODE, 0x00);
    // put module in lora mode
    writeReg(REG_OP_MODE, 0x80);
    // put module in sleep mode, lora side
    writeReg(REG_OP_MODE, LORA_SLEEP_MODE);
    // set overcurrent protection on but leave overcurrent protection value default
    uint8_t tmp = readReg(REG_OCP);
    writeReg(REG_OCP, tmp | 0x20);

    // set fifo base pointers to 0x00, we use the full fifo for each operation
    writeReg(REG_FIFO_TX_BASE_ADDR, 0x00);
    writeReg(REG_FIFO_RX_BASE_ADDR, 0x00);

    // set preamble length to its custom default value
    writeReg(REG_PREAMBLE_MSB_LORA, (uint8_t)((PREAMBLE_LEN >> 8) & 0xFF));
    writeReg(REG_PREAMBLE_LSB_LORA, (uint8_t)(PREAMBLE_LEN & 0xFF));

    // disable frequency hopping
    writeReg(REG_HOP_PERIOD, 0x00);

    // set this node as static node, lna gain set by register
    writeReg(REG_MODEM_CONFIG3, 0x00);

    // set max payload length to 255
    writeReg(REG_MAX_PAYLOAD_LENGTH, 0xFF);

    return 0;
}

int Lora::off()
{
    return 0;
}

int Lora::setMainParameters(uint32_t chan, uint8_t bandwidth,
                            uint8_t codingRate, uint8_t spreadingFactor)
{
    uint8_t tmp;

    setMode(LORA_SLEEP_MODE);
    // check is chan argument is valid
    if(chan != CH_10_868 && chan != CH_11_868 &&
            chan != CH_12_868 && chan != CH_13_868 &&
            chan != CH_14_868 && chan != CH_15_868 &&
            chan != CH_16_868 && chan != CH_17_868)
    {
        return -1;
    }
    else
    {
        writeReg(REG_FRF_MSB, (uint8_t)((chan >> 16) & 0xFF));
        writeReg(REG_FRF_MID, (uint8_t)((chan >> 8)  & 0xFF));
        writeReg(REG_FRF_LSB, (uint8_t)(chan & 0xFF));

        _channel = chan;
    }

    if(bandwidth < BW_125 || bandwidth > BW_500 || codingRate < 0x01 || codingRate > 0x04)
    {
        return -2;
    }
    else
    {
        tmp = readReg(REG_MODEM_CONFIG1);
        tmp &= 0x0F;
        tmp |= (bandwidth << 4);
        tmp &= 0xF1;
        tmp |= (codingRate << 1);
        writeReg(REG_MODEM_CONFIG1, tmp);

        _bandwidth = bandwidth;
        _codingRate = codingRate;
    }

    if(spreadingFactor < SF_7 || spreadingFactor > SF_12)
    {
        return -3;
    }
    else
    {
        tmp = readReg(REG_MODEM_CONFIG2);
        tmp &= 0x0F;
        tmp |= (spreadingFactor << 4);
        writeReg(REG_MODEM_CONFIG2, tmp);
        _spreadingFactor = spreadingFactor;
    }

    switch (bandwidth) {
    case BW_125:
        _symbRate = 125000/pow(2, (double)(spreadingFactor));
        break;
    case BW_250:
        _symbRate = 250000/pow(2, (double)(spreadingFactor));
        break;
    case BW_500:
        _symbRate = 500000/pow(2, (double)(spreadingFactor));
        break;
    default:
        return -4;
        break;
    }

    return 0;
}

int Lora::enableCRCCheck(bool state)
{
    uint8_t tmp = readReg(REG_MODEM_CONFIG2);
    if(state == true)
    {
        tmp |= (1 << 2);
    }
    else
    {
        tmp &= ~(1 << 2);
    }
    writeReg(REG_MODEM_CONFIG2, tmp);
    _CRCCheckEnabled = state;
    return 0;
}

int Lora::enableImplicitHeader(bool state)
{
    uint8_t tmp = readReg(REG_MODEM_CONFIG1);
    if(state == true)
    {
        tmp |= 0x01;
    }
    else
    {
        tmp &= 0xFE;
    }
    writeReg(REG_MODEM_CONFIG1, tmp);
    _implicitHeaderEnabled = state;
    return 0;
}

int Lora::setOutputPower(int pwrdB)
{
    uint8_t out, tmp;
    // we only use PA_BOOST
    if(pwrdB > 20)
    {
        pwrdB = 20;
    }
    out = (pwrdB - 2) & 0x0F;
    tmp = readReg(REG_PA_CONFIG);
    // use PA_BOOST
    tmp |= 0x80;
    tmp &= 0xF0;
    tmp |= (out & 0x0F);
    writeReg(REG_PA_CONFIG, tmp);
    _pwrdB = pwrdB;
    return 0;
}

int Lora::setMode(uint8_t mode)
{
    if(mode != LORA_SLEEP_MODE && mode != LORA_STANDBY_MODE &&
            mode != LORA_TX_MODE && mode != LORA_RX_MODE &&
            mode != LORA_RX_SINGLE_MODE && mode != LORA_STANDBY_FSK_REGS_MODE)
    {
        return -1;
    }
    else
    {
        writeReg(REG_OP_MODE, mode);
        return 0;
    }
}

int Lora::setCallbacks(void (*txDone)(void), callback rxDone, callback timeout)
{
    Lora::_txDone = txDone;
    Lora::_rxDone = rxDone;
    Lora::_timeout = timeout;
    return 0;
}

void *Lora::timerThreadFunction(void *timeout)
{
    uint32_t tm = (uint32_t)timeout;
    usleep(tm * 1000);
    _timeout();
    pthread_exit(0);
}

int Lora::send(uint8_t *buf, uint8_t length)
{
    // go in standby mode to write the fifo
    writeReg(REG_OP_MODE, LORA_STANDBY_MODE);

    // clear all flags in interrupt register
    writeReg(REG_IRQ_FLAGS, 0xFF);

    // set buffer length
    writeReg(REG_PAYLOAD_LENGTH_LORA, length);

    // write data into the fifo at address 0x00
    writeReg(REG_FIFO_TX_BASE_ADDR, 0x00);
    writeReg(REG_FIFO_ADDR_PTR, 0x00);
    writeBuf(REG_FIFO, buf, length);

    // set dio mapped to function txDone
    setDio0Mapping(DIO0_FN_TX_DONE);

    // set module in tx mode
    setMode(LORA_TX_MODE);

    return 0;
}

int Lora::setInTimedReceiveMode(uint32_t timeout)
{
    int ret;
    // set RF rx block parameters
    writeReg(REG_PA_RAMP, 0x09);
    // set max lna gain
    writeReg(REG_LNA, 0x23);
    // set fifo address pointer at 0x00
    writeReg(REG_FIFO_ADDR_PTR, 0x00);
    // set dio0 function
    setDio0Mapping(DIO0_FN_RX_DONE);

    // put module in continuous rx mode
    writeReg(REG_OP_MODE, LORA_RX_MODE);

    // launch timer thread
    ret = pthread_create(&timerThread, NULL, timerThreadFunction, ((void *)timeout));
    if(ret)
    {
        writeReg(REG_OP_MODE, LORA_STANDBY_MODE);
        return -1;
    }

    return 0;
}

int Lora::setInContinuousReceiveMode()
{
    // set RF rx block parameters
    writeReg(REG_PA_RAMP, 0x09);
    // set max lna gain
    writeReg(REG_LNA, 0x23);
    // set fifo address pointer at 0x00
    writeReg(REG_FIFO_ADDR_PTR, 0x00);
    // set dio0 function to rxDone
    setDio0Mapping(DIO0_FN_RX_DONE);

    // clear all irq flags
    writeReg(REG_IRQ_FLAGS, 0x00);

    // put module in continuous rx mode
    writeReg(REG_OP_MODE, LORA_RX_MODE);
    return 0;
}

int Lora::getReceivedData(uint8_t *data)
{
    uint8_t tmp = readReg(REG_IRQ_FLAGS);
    if ((tmp & IRQ_PAYLOAD_CRC_ERROR_MASK) != 0)
    {
        return -1;
    }

    // read number of bytes received
    int payloadLen = readReg(REG_RX_NB_BYTES);
    if(payloadLen == 0)
    {
        // if the number of bytes received is zero,
        // there is a problem.
        return -2;
    }
    // read addr of the last received packet
    int lastAddr = readReg(REG_FIFO_RX_CURRENT_ADDR);
    // set pointer to the last addr
    writeReg(REG_FIFO_ADDR_PTR, lastAddr);
    // read the number of bytes received and put into buffer
    readBuf(REG_FIFO, data, payloadLen);

    return 0;
}

int Lora::getSNR()
{
    int tmp = (int)(readReg(REG_PKT_SNR_VALUE));
    _snr = tmp/4;
    return _snr;
}

int Lora::getPktRSSI()
{
    int tmp = (int)(readReg(REG_PKT_RSSI_VALUE));
    _pktRssi = tmp - 137;
    return _pktRssi;
}

int Lora::getRSSI()
{
    int tmp = (int)(readReg(REG_RSSI_VALUE_LORA));
    _rssi = tmp - 137;
    return _rssi;
}

void Lora::setDio0Mapping(uint8_t fn)
{

    uint8_t previousMode = readReg(REG_OP_MODE);
    writeReg(REG_OP_MODE, LORA_STANDBY_FSK_REGS_MODE);
    uint8_t tmp = readReg(REG_DIO_MAPPING1);
    switch (fn)
    {
    case DIO0_FN_RX_DONE:
        tmp &= 0x3F;
        break;
    case DIO0_FN_TX_DONE:
        tmp &= 0x3F;
        tmp |= 0x40;
        break;
    default:
        tmp &= 0x3F;
        break;
    }
    writeReg(REG_DIO_MAPPING1, tmp);
    writeReg(REG_OP_MODE, previousMode);
}

void Lora::writeReg(uint8_t reg, uint8_t data)
{
    uint8_t tmp = reg | 0x80;
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(0, &tmp, 1);
    wiringPiSPIDataRW(0, &data, 1);
    digitalWrite(CS_PIN, HIGH);
}

void Lora::writeBuf(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t tmp = reg | 0x80;
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(0, &tmp, 1);
    wiringPiSPIDataRW(0, buf, len);
    digitalWrite(CS_PIN, HIGH);
}

uint8_t Lora::readReg(uint8_t reg)
{
    uint8_t tmp = reg & 0x7F;
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(0, &tmp, 1);
    tmp = 0x00;
    wiringPiSPIDataRW(0, &tmp, 1);
    digitalWrite(CS_PIN, HIGH);
    return tmp;
}

void Lora::readBuf(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t tmp = reg & 0x7F;
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(0, &tmp, 1);
    wiringPiSPIDataRW(0, buf, len);
    digitalWrite(CS_PIN, HIGH);
}

void Lora::onDIO0Interrupt()
{
    uint8_t previousState = readReg(REG_OP_MODE);
    writeReg(REG_OP_MODE, LORA_STANDBY_MODE);
    uint8_t tmp = readReg(REG_IRQ_FLAGS);
    if((tmp & IRQ_RX_DONE_MASK) != 0 && _rxDone != NULL)
    {
        _rxDone();
    }
    else if((tmp & IRQ_TX_DONE_MASK) != 0 && _txDone != NULL)
    {
        _txDone();
    }
    // clear interrupt flags
    writeReg(REG_IRQ_FLAGS, 0xFF);
    writeReg(REG_OP_MODE, previousState);
}

void Lora::reset()
{
    pinMode(RST_PIN, OUTPUT);
    digitalWrite(RST_PIN, LOW);
    delay(1);
    pinMode(RST_PIN, INPUT);
    delay(10);
}
