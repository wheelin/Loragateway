#include "lora.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "wiringPi.h"
#include "wiringPiSPI.h"
#include <pthread.h>
#include <unistd.h>

void (*lora::_txDone)() = 0;
void (*lora::_rxDone)() = 0;
void (*lora::_timeout)() = 0;

lora::lora()
{
    ch = CH_10_868;
    bw = BW_125;
    cr = CR_5;
    sf = SF_12;
    crc_check_enabled = true;
    implicit_header_enabled = true;
    pwr_db = 15;
    listening = false;
}

int lora::on()
{
    wiringPiSetupPhys();
    spi_fd = wiringPiSPISetupMode(0, SPI_SPEED, SPI_MODE);
    if(spi_fd < 0)
    {
        return -1;
    }
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    pinMode(RST_PIN, INPUT);
    wiringPiISR(DIO0_PIN, INT_EDGE_RISING, onDIO0Interrupt);

    memset(last_recv_buf, 0, MAX_DATA_LEN);
    memset(last_sent_buf, 0, MAX_DATA_LEN);

    reset();

    // put module in fsk ook sleep mode
    write_reg(REG_OP_MODE, 0x00);
    // put module in lora mode
    write_reg(REG_OP_MODE, 0x80);
    // put module in sleep mode, lora side
    write_reg(REG_OP_MODE, LORA_SLEEP_MODE);
    // set overcurrent protection on but leave overcurrent protection value default
    uint8_t tmp = read_reg(REG_OCP);
    write_reg(REG_OCP, tmp | 0x20);

    // set fifo base pointers to 0x00, we use the full fifo for each operation
    write_reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    write_reg(REG_FIFO_RX_BASE_ADDR, 0x00);

    // set preamble length to its custom default value
    write_reg(REG_PREAMBLE_MSB_LORA, (uint8_t)((PREAMBLE_LEN >> 8) & 0xFF));
    write_reg(REG_PREAMBLE_LSB_LORA, (uint8_t)(PREAMBLE_LEN & 0xFF));

    // disable frequency hopping
    write_reg(REG_HOP_PERIOD, 0x00);

    // set this node as static node, lna gain set by register
    write_reg(REG_MODEM_CONFIG3, 0x00);

    // set max payload length to 255
    write_reg(REG_MAX_PAYLOAD_LENGTH, 0xFF);

    return 0;
}

int lora::off()
{
    return 0;
}

int lora::set_main_parameters(uint32_t ch, uint8_t bw,
                            uint8_t cr, uint8_t sf)
{
    uint8_t tmp;

    set_mode(LORA_SLEEP_MODE);
    // check is chan argument is valid
    if(ch != CH_10_868 && ch != CH_11_868 &&
            ch != CH_12_868 && ch != CH_13_868 &&
            ch != CH_14_868 && ch != CH_15_868 &&
            ch != CH_16_868 && ch != CH_17_868)
    {
        return -1;
    }
    else
    {
        write_reg(REG_FRF_MSB, (uint8_t)((ch >> 16) & 0xFF));
        write_reg(REG_FRF_MID, (uint8_t)((ch >> 8)  & 0xFF));
        write_reg(REG_FRF_LSB, (uint8_t)(ch & 0xFF));

        this->ch = ch;
    }

    if(bw < BW_125 || bw > BW_500 || cr < 0x01 || cr > 0x04)
    {
        return -2;
    }
    else
    {
        tmp = read_reg(REG_MODEM_CONFIG1);
        tmp &= 0x0F;
        tmp |= (bw << 4);
        tmp &= 0xF1;
        tmp |= (codingRate << 1);
        write_reg(REG_MODEM_CONFIG1, tmp);

        this->bw = bw;
        this->cr = codingRate;
    }

    if(sf < SF_7 || sf > SF_12)
    {
        return -3;
    }
    else
    {
        tmp = read_reg(REG_MODEM_CONFIG2);
        tmp &= 0x0F;
        tmp |= (sf << 4);
        write_reg(REG_MODEM_CONFIG2, tmp);
        this->sf = sf;
    }

    switch (bandwidth) {
    case BW_125:
        symb_rate = 125000/pow(2, (double)(sf));
        break;
    case BW_250:
        symb_rate = 250000/pow(2, (double)(sf));
        break;
    case BW_500:
        symb_rate = 500000/pow(2, (double)(sf));
        break;
    default:
        return -4;
        break;
    }

    return 0;
}

int lora::enable_crc_check(bool state)
{
    uint8_t tmp = read_reg(REG_MODEM_CONFIG2);
    if(state == true)
    {
        tmp |= (1 << 2);
    }
    else
    {
        tmp &= ~(1 << 2);
    }
    write_reg(REG_MODEM_CONFIG2, tmp);
    crc_check_enabled = state;
    return 0;
}

int lora::enable_implicit_header(bool state)
{
    uint8_t tmp = read_reg(REG_MODEM_CONFIG1);
    if(state == true)
    {
        tmp |= 0x01;
    }
    else
    {
        tmp &= 0xFE;
    }
    write_reg(REG_MODEM_CONFIG1, tmp);
    implicit_header_enabled = state;
    return 0;
}

int lora::set_output_power(int pwrdB)
{
    uint8_t out, tmp;
    // we only use PA_BOOST
    if(pwrdB > 20)
    {
        pwrdB = 20;
    }
    out = (pwrdB - 2) & 0x0F;
    tmp = read_reg(REG_PA_CONFIG);
    // use PA_BOOST
    tmp |= 0x80;
    tmp &= 0xF0;
    tmp |= (out & 0x0F);
    write_reg(REG_PA_CONFIG, tmp);
    _pwrdB = pwrdB;
    return 0;
}

int lora::set_mode(uint8_t mode)
{
    if(mode != LORA_SLEEP_MODE && mode != LORA_STANDBY_MODE &&
            mode != LORA_TX_MODE && mode != LORA_RX_MODE &&
            mode != LORA_RX_SINGLE_MODE && mode != LORA_STANDBY_FSK_REGS_MODE)
    {
        return -1;
    }
    else
    {
        write_reg(REG_OP_MODE, mode);
        return 0;
    }
}

int lora::get_mode()
{
    uint8_t tmp = read_reg(REG_OP_MODE);
    return tmp & 0x87;
}

int lora::set_callbacks(void (*tx_done)(void), void (*rx_done)(void), void (*timeout)(void))
{
    lora::tx_done = txDone;
    lora::rx_done = rxDone;
    lora::timeout = timeout;
    return 0;
}

void *lora::timer_thread_fn(void *timeout)
{
    uint32_t tm = (uint32_t)timeout;
    usleep(tm * 1000);
    timeout();
    pthread_exit(0);
}

int lora::send(uint8_t *buf, uint8_t length)
{
    // go in standby mode to write the fifo
    write_reg(REG_OP_MODE, LORA_STANDBY_MODE);

    // clear all flags in interrupt register
    write_reg(REG_IRQ_FLAGS, 0xFF);

    // set buffer length
    write_reg(REG_PAYLOAD_LENGTH_LORA, length);

    // write data into the fifo at address 0x00
    write_reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    write_reg(REG_FIFO_ADDR_PTR, 0x00);
    writeBuf(REG_FIFO, buf, length);

    // set dio mapped to function txDone
    set_dio0_mapping(DIO0_FN_TX_DONE);

    // set module in tx mode
    set_mode(LORA_TX_MODE);

    return 0;
}

int lora::listen_timeout(uint32_t timeout)
{
    int ret;
    // set RF rx block parameters
    write_reg(REG_PA_RAMP, 0x09);
    // set max lna gain
    write_reg(REG_LNA, 0x23);
    // set fifo address pointer at 0x00
    write_reg(REG_FIFO_ADDR_PTR, 0x00);
    // set dio0 function
    set_dio0_mapping(DIO0_FN_RX_DONE);

    // put module in continuous rx mode
    write_reg(REG_OP_MODE, LORA_RX_MODE);

    // launch timer thread
    ret = pthread_create(&timerThread, NULL, timerThreadFunction, ((void *)timeout));
    if(ret)
    {
        write_reg(REG_OP_MODE, LORA_STANDBY_MODE);
        return -1;
    }

    return 0;
}

int lora::listen()
{
    // set RF rx block parameters
    write_reg(REG_PA_RAMP, 0x09);
    // set max lna gain
    write_reg(REG_LNA, 0x23);
    // set fifo address pointer at 0x00
    write_reg(REG_FIFO_ADDR_PTR, 0x00);
    // set dio0 function to rxDone
    setDio0Mapping(DIO0_FN_RX_DONE);

    // clear all irq flags
    write_reg(REG_IRQ_FLAGS, 0x00);

    // put module in continuous rx mode
    write_reg(REG_OP_MODE, LORA_RX_MODE);
    return 0;
}

int lora::get_received_data(uint8_t *data)
{
    uint8_t tmp = read_reg(REG_IRQ_FLAGS);
    if ((tmp & IRQ_PAYLOAD_CRC_ERROR_MASK) != 0)
    {
        return -1;
    }

    // read number of bytes received
    int payloadLen = read_reg(REG_RX_NB_BYTES);
    if(payloadLen == 0)
    {
        // if the number of bytes received is zero,
        // there is a problem.
        return -2;
    }
    // read addr of the last received packet
    int lastAddr = read_reg(REG_FIFO_RX_CURRENT_ADDR);
    // set pointer to the last addr
    write_reg(REG_FIFO_ADDR_PTR, lastAddr);
    // read the number of bytes received and put into buffer
    read_buf(REG_FIFO, data, payloadLen);

    return 0;
}

int lora::get_snr()
{
    int tmp = (int)(read_reg(REG_PKT_SNR_VALUE));
    snr = tmp/4;
    return snr;
}

int lora::get_pkt_rssi()
{
    int tmp = (int)(read_reg(REG_PKT_RSSI_VALUE));
    pkt_rssi = tmp - 137;
    return pkt_rssi;
}

int lora::get_rssi()
{
    int tmp = (int)(read_reg(REG_RSSI_VALUE_LORA));
    rssi = tmp - 137;
    return rssi;
}

void lora::set_dio0_mapping(uint8_t fn)
{

    uint8_t previousMode = read_reg(REG_OP_MODE);
    write_reg(REG_OP_MODE, LORA_STANDBY_FSK_REGS_MODE);
    uint8_t tmp = read_reg(REG_DIO_MAPPING1);
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
    write_reg(REG_DIO_MAPPING1, tmp);
    write_reg(REG_OP_MODE, previousMode);
}

void lora::write_reg(uint8_t reg, uint8_t data)
{
    uint8_t tmp = reg | 0x80;
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(0, &tmp, 1);
    wiringPiSPIDataRW(0, &data, 1);
    digitalWrite(CS_PIN, HIGH);
}

void lora::write_buf(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t tmp = reg | 0x80;
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(0, &tmp, 1);
    wiringPiSPIDataRW(0, buf, len);
    digitalWrite(CS_PIN, HIGH);
}

uint8_t lora::read_reg(uint8_t reg)
{
    uint8_t tmp = reg & 0x7F;
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(0, &tmp, 1);
    tmp = 0x00;
    wiringPiSPIDataRW(0, &tmp, 1);
    digitalWrite(CS_PIN, HIGH);
    return tmp;
}

void lora::read_buf(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t tmp = reg & 0x7F;
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(0, &tmp, 1);
    wiringPiSPIDataRW(0, buf, len);
    digitalWrite(CS_PIN, HIGH);
}

void lora::on_dio0_interrupt()
{
    //uint8_t previousState = read_reg(REG_OP_MODE);
    write_reg(REG_OP_MODE, LORA_STANDBY_MODE);
    uint8_t tmp = read_reg(REG_IRQ_FLAGS);
    if((tmp & IRQ_RX_TIMEOUT_MASK) != 0 && _timeout != NULL)
    {
        _timeout();
    }
    else if((tmp & IRQ_RX_DONE_MASK) != 0 && _rxDone != NULL)
    {
        _rxDone();
    }
    else if((tmp & IRQ_TX_DONE_MASK) != 0 && _txDone != NULL)
    {
        _txDone();
    }
    // clear interrupt flags
    write_reg(REG_IRQ_FLAGS, 0xFF);
    //write_reg(REG_OP_MODE, previousState);
}

void lora::reset()
{
    pinMode(RST_PIN, OUTPUT);
    digitalWrite(RST_PIN, LOW);
    delay(1);
    pinMode(RST_PIN, INPUT);
    delay(10);
}
