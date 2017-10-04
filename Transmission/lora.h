#ifndef LORA_H
#define LORA_H

#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <pthread.h>

// raspberry pi specific defines
#define        RST_PIN                          11
#define        CS_PIN                           22
#define        DIO0_PIN                         7
#define        SPI_SPEED                        7000000
#define        SPI_MODE                         0

#define        REG_FIFO        					0x00
#define        REG_OP_MODE        				0x01
#define        REG_BITRATE_MSB    				0x02
#define        REG_BITRATE_LSB    				0x03
#define        REG_FDEV_MSB   					0x04
#define        REG_FDEV_LSB    					0x05
#define        REG_FRF_MSB    					0x06
#define        REG_FRF_MID    					0x07
#define        REG_FRF_LSB    					0x08
#define        REG_PA_CONFIG    				0x09
#define        REG_PA_RAMP    					0x0A
#define        REG_OCP    						0x0B
#define        REG_LNA    						0x0C
#define        REG_RX_CONFIG    				0x0D
#define        REG_FIFO_ADDR_PTR  				0x0D
#define        REG_RSSI_CONFIG   				0x0E
#define        REG_FIFO_TX_BASE_ADDR 		    0x0E
#define        REG_RSSI_COLLISION    			0x0F
#define        REG_FIFO_RX_BASE_ADDR   			0x0F
#define        REG_RSSI_THRESH    				0x10
#define        REG_FIFO_RX_CURRENT_ADDR   		0x10
#define        REG_RSSI_VALUE_FSK	    		0x11
#define        REG_IRQ_FLAGS_MASK    			0x11
#define        REG_RX_BW		    			0x12
#define        REG_IRQ_FLAGS	    			0x12
#define        REG_AFC_BW		    			0x13
#define        REG_RX_NB_BYTES	    			0x13
#define        REG_OOK_PEAK	    				0x14
#define        REG_RX_HEADER_CNT_VALUE_MSB  	0x14
#define        REG_OOK_FIX	    				0x15
#define        REG_RX_HEADER_CNT_VALUE_LSB  	0x15
#define        REG_OOK_AVG	 					0x16
#define        REG_RX_PACKET_CNT_VALUE_MSB  	0x16
#define        REG_RX_PACKET_CNT_VALUE_LSB  	0x17
#define        REG_MODEM_STAT	  				0x18
#define        REG_PKT_SNR_VALUE	  			0x19
#define        REG_AFC_FEI	  					0x1A
#define        REG_PKT_RSSI_VALUE	  			0x1A
#define        REG_AFC_MSB	  					0x1B
#define        REG_RSSI_VALUE_LORA	  			0x1B
#define        REG_AFC_LSB	  					0x1C
#define        REG_HOP_CHANNEL	  				0x1C
#define        REG_FEI_MSB	  					0x1D
#define        REG_MODEM_CONFIG1	 		 	0x1D
#define        REG_FEI_LSB	  					0x1E
#define        REG_MODEM_CONFIG2	  			0x1E
#define        REG_PREAMBLE_DETECT  			0x1F
#define        REG_SYMB_TIMEOUT_LSB  			0x1F
#define        REG_RX_TIMEOUT1	  				0x20
#define        REG_PREAMBLE_MSB_LORA  			0x20
#define        REG_RX_TIMEOUT2	  				0x21
#define        REG_PREAMBLE_LSB_LORA  			0x21
#define        REG_RX_TIMEOUT3	 				0x22
#define        REG_PAYLOAD_LENGTH_LORA		 	0x22
#define        REG_RX_DELAY	 					0x23
#define        REG_MAX_PAYLOAD_LENGTH 			0x23
#define        REG_OSC		 					0x24
#define        REG_HOP_PERIOD	  				0x24
#define        REG_PREAMBLE_MSB_FSK 			0x25
#define        REG_FIFO_RX_BYTE_ADDR 			0x25
#define        REG_PREAMBLE_LSB_FSK 			0x26
#define		   REG_MODEM_CONFIG3				0x26
#define        REG_SYNC_CONFIG	  				0x27
#define        REG_SYNC_VALUE1	 				0x28
#define        REG_SYNC_VALUE2	  				0x29
#define        REG_SYNC_VALUE3	  				0x2A
#define        REG_SYNC_VALUE4	  				0x2B
#define        REG_SYNC_VALUE5	  				0x2C
#define        REG_SYNC_VALUE6	  				0x2D
#define        REG_SYNC_VALUE7	  				0x2E
#define        REG_SYNC_VALUE8	  				0x2F
#define        REG_PACKET_CONFIG1	  			0x30
#define        REG_PACKET_CONFIG2	  			0x31
#define        REG_DETECT_OPTIMIZE	  			0x31
#define        REG_PAYLOAD_LENGTH_FSK			0x32
#define        REG_NODE_ADRS	  				0x33
#define        REG_BROADCAST_ADRS	 		 	0x34
#define        REG_FIFO_THRESH	  				0x35
#define        REG_SEQ_CONFIG1	  				0x36
#define        REG_SEQ_CONFIG2	  				0x37
#define        REG_DETECTION_THRESHOLD 			0x37
#define        REG_TIMER_RESOL	  				0x38
#define        REG_TIMER1_COEF	  				0x39
#define        REG_TIMER2_COEF	  				0x3A
#define        REG_IMAGE_CAL	  				0x3B
#define        REG_TEMP		  					0x3C
#define        REG_LOW_BAT	  					0x3D
#define        REG_IRQ_FLAGS1	  				0x3E
#define        REG_IRQ_FLAGS2	  				0x3F
#define        REG_DIO_MAPPING1	  				0x40
#define        REG_DIO_MAPPING2	  				0x41
#define        REG_VERSION	  					0x42
#define        REG_AGC_REF	  					0x43
#define        REG_AGC_THRESH1	  				0x44
#define        REG_AGC_THRESH2	  				0x45
#define        REG_AGC_THRESH3	  				0x46
#define        REG_PLL_HOP	  					0x4B
#define        REG_TCXO		  					0x58
#define        REG_PA_DAC		  				0x5A
#define        REG_PLL		  					0x5C
#define        REG_PLL_LOW_PN	  				0x5E
#define        REG_FORMER_TEMP	  				0x6C
#define        REG_BIT_RATE_FRAC	  			0x70

//FREQUENCY CHANNELS:
#define        CH_10_868                        ((uint32_t)0xD84CCC) // channel 10, central freq = 865.20MHz
#define        CH_11_868                        ((uint32_t)0xD86000) // channel 11, central freq = 865.50MHz
#define        CH_12_868                        ((uint32_t)0xD87333) // channel 12, central freq = 865.80MHz
#define        CH_13_868                        ((uint32_t)0xD88666) // channel 13, central freq = 866.10MHz
#define        CH_14_868                        ((uint32_t)0xD89999) // channel 14, central freq = 866.40MHz
#define        CH_15_868                        ((uint32_t)0xD8ACCC) // channel 15, central freq = 866.70MHz
#define        CH_16_868                        ((uint32_t)0xD8C000) // channel 16, central freq = 867.00MHz
#define        CH_17_868                        ((uint32_t)0xD90000) // channel 16, central freq = 868.00MHz

//LORA BANDWIDTH:
#define        BW_125                           ((uint8_t)0x07)
#define        BW_250                           ((uint8_t)0x08)
#define        BW_500		                    ((uint8_t)0x09)

//LORA CODING RATE:
#define        CR_5                             ((uint8_t)0x01)	// CR = 4/5
#define        CR_6                             ((uint8_t)0x02)	// CR = 4/6
#define        CR_7                             ((uint8_t)0x03)	// CR = 4/7
#define        CR_8                             ((uint8_t)0x04)	// CR = 4/8

//LORA SPREADING FACTOR:
#define        SF_7                             ((uint8_t)0x07)
#define        SF_8                             ((uint8_t)0x08)
#define        SF_9                             ((uint8_t)0x09)
#define        SF_10                            ((uint8_t)0x0A)
#define        SF_11                            ((uint8_t)0x0B)
#define        SF_12                            ((uint8_t)0x0C)

//LORA MODES:
#define        LORA_SLEEP_MODE                  ((uint8_t)0x80)
#define        LORA_STANDBY_MODE                ((uint8_t)0x81)
#define        LORA_FREQ_TX_SYNTH_MODE          ((uint8_t)0x82)
#define        LORA_TX_MODE                     ((uint8_t)0x83)
#define        LORA_FREQ_RX_SYNTH_MODE          ((uint8_t)0x84)
#define        LORA_RX_MODE                     ((uint8_t)0x85)
#define        LORA_RX_SINGLE_MODE              ((uint8_t)0x86)
#define        LORA_CAD_DETECTION_MODE          ((uint8_t)0x87)
#define        LORA_STANDBY_FSK_REGS_MODE       ((uint8_t)0xC1)

//DIVERS MASKS
#define        MAX_DATA_LEN                     256
#define        PREAMBLE_LEN                     0x0008
#define        IRQ_RX_TIMEOUT_MASK              ((uint8_t)0x80)
#define        IRQ_RX_DONE_MASK                 ((uint8_t)0x40)
#define        IRQ_PAYLOAD_CRC_ERROR_MASK       ((uint8_t)0x20)
#define        IRQ_VALID_HEADER_MASK            ((uint8_t)0x10)
#define        IRQ_TX_DONE_MASK                 ((uint8_t)0x08)
#define        IRQ_CAD_DONE_MASK                ((uint8_t)0x04)
#define        IRQ_FHSS_CHANGE_CHANNEL_MASK     ((uint8_t)0x02)
#define        IRQ_CAD_DETECT_MASK              ((uint8_t)0x01)
#define        DIO0_FN_RX_DONE                  ((uint8_t)0x00)
#define        DIO0_FN_TX_DONE                  ((uint8_t)0x01)

typedef void (*callback)(void);

class Lora
{
public:

    lora();

    int on();
    int off();

    int set_main_parameters(uint32_t ch, uint8_t bw, uint8_t cr, uint8_t sf);
    int enable_crc_check(bool state);
    int enable_implicit_header(bool state);
    int set_output_power(int pwrdB);

    int set_mode(uint8_t mode);
    int get_mode();

    int set_callbacks(void (*tx_done)(), void (*rx_done)(), void (*timeout)());

    int send(uint8_t *buf, uint8_t length);
    int listen_timeout(uint32_t timeout);
    int listen();
    int stop_listening();
    int get_received_data(uint8_t * data);

    int get_snr();
    int get_pkt_rssi();
    int get_rssi();

    void set_dio0_mapping(uint8_t fn);
    static void on_dio0_interrupt(void);
    static void write_reg(uint8_t reg, uint8_t data);
    static void write_buf(uint8_t reg, uint8_t * buf, uint8_t len);
    static uint8_t read_reg(uint8_t reg);
    void read_buf(uint8_t reg, uint8_t * buf, uint8_t len);
    void reset();

private:
    pthread_t       timer_thread;
    static void *   timer_thread_fn(void * timeout);
    uint32_t        ch;
    uint8_t         bw;
    uint8_t         cr;
    uint8_t         sf;
    bool            crc_check_enabled;
    bool            implicit_header_enabled;
    int             pwr_db;
    bool            listening;
    int             spi_fd;
    int             symb_rate;

    int             snr;
    int             pkt_rssi;
    int             rssi;

    static void     (*tx_done)(void);
    static void     (*rx_done)(void);
    static void     (*timeout)(void);

    uint8_t         last_recv_buf[MAX_DATA_LEN];
    uint8_t         last_sent_buf[MAX_DATA_LEN];
};



#endif // LORA_H
