#pragma once

uint8_t init_radio();
uint8_t cc1101_test_gpio(uint8_t GPIO_CONFIG_REG, uint8_t pin);
void CC1101_set_freq(uint8_t freq);
void CC1101_set_freq_offset(uint8_t freq_offset);
void CC1101_set_net_id(uint8_t id);
void cc1101_idle();
void cc1101_rx(uint8_t broadcast_rx);
void cc1101_tx();
uint8_t cc1101_prepaire_tx(uint8_t input_freq, uint8_t input_net_id);
void cc1101_tx_fill(uint8_t buffer[], uint8_t length);
int cc1101_read_fifo(uint8_t buffer[]);

// CC1101 registers
// ----------------
#define CC1101_REG_IOCFG2 0x00   // GDO2 output pin configuration
#define CC1101_REG_IOCFG1 0x01   // GDO1 output pin configuration
#define CC1101_REG_IOCFG0 0x02   // GDO0 output pin configuration
#define CC1101_REG_FIFOTHR 0x03  // RX FIFO and TX FIFO thresholds
#define CC1101_REG_SYNC1 0x04    // Sync word, high byte
#define CC1101_REG_SYNC0 0x05    // Sync word, low byte
#define CC1101_REG_PKTLEN 0x06   // Packet length
#define CC1101_REG_PKTCTRL1 0x07 // Packet automation control
#define CC1101_REG_PKTCTRL0 0x08 // Packet automation control
#define CC1101_REG_ADDR 0x09     // Device address
#define CC1101_REG_CHANNR 0x0A   // Channel number
#define CC1101_REG_FSCTRL1 0x0B  // Frequency synthesizer control
#define CC1101_REG_FSCTRL0 0x0C  // Frequency synthesizer control
#define CC1101_REG_FREQ2 0x0D    // Frequency control word, high byte
#define CC1101_REG_FREQ1 0x0E    // Frequency control word, middle byte
#define CC1101_REG_FREQ0 0x0F    // Frequency control word, low byte
#define CC1101_REG_MDMCFG4 0x10  // Modem configuration
#define CC1101_REG_MDMCFG3 0x11  // Modem configuration
#define CC1101_REG_MDMCFG2 0x12  // Modem configuration
#define CC1101_REG_MDMCFG1 0x13  // Modem configuration
#define CC1101_REG_MDMCFG0 0x14  // Modem configuration
#define CC1101_REG_DEVIATN 0x15  // Modem deviation setting
#define CC1101_REG_MCSM2 0x16    // Main Radio Cntrl State Machine config
#define CC1101_REG_MCSM1 0x17    // Main Radio Cntrl State Machine config
#define CC1101_REG_MCSM0 0x18    // Main Radio Cntrl State Machine config
#define CC1101_REG_FOCCFG 0x19   // Frequency Offset Compensation config
#define CC1101_REG_BSCFG 0x1A    // Bit Synchronization configuration
#define CC1101_REG_AGCCTRL2 0x1B // AGC control
#define CC1101_REG_AGCCTRL1 0x1C // AGC control
#define CC1101_REG_AGCCTRL0 0x1D // AGC control
#define CC1101_REG_WOREVT1 0x1E  // High byte Event 0 timeout
#define CC1101_REG_WOREVT0 0x1F  // Low byte Event 0 timeout
#define CC1101_REG_WORCTRL 0x20  // Wake On Radio control
#define CC1101_REG_FREND1 0x21   // Front end RX configuration
#define CC1101_REG_FREND0 0x22   // Front end TX configuration
#define CC1101_REG_FSCAL3 0x23   // Frequency synthesizer calibration
#define CC1101_REG_FSCAL2 0x24   // Frequency synthesizer calibration
#define CC1101_REG_FSCAL1 0x25   // Frequency synthesizer calibration
#define CC1101_REG_FSCAL0 0x26   // Frequency synthesizer calibration
#define CC1101_REG_RCCTRL1 0x27  // RC oscillator configuration
#define CC1101_REG_RCCTRL0 0x28  // RC oscillator configuration
#define CC1101_REG_FSTEST 0x29   // Frequency synthesizer cal control
#define CC1101_REG_PTEST 0x2A    // Production test
#define CC1101_REG_AGCTEST 0x2B  // AGC test
#define CC1101_REG_TEST2 0x2C    // Various test settings
#define CC1101_REG_TEST1 0x2D    // Various test settings
#define CC1101_REG_TEST0 0x2E    // Various test settings
#define CC1101_REG_PATABLE 0x3E  // power amplifier table
// CC1101 commands
// ---------------
#define CC1101_CMD_SRES 0x30    // Reset chip
#define CC1101_CMD_SFSTXON 0x31 // Enable/calibrate freq synthesizer
#define CC1101_CMD_SXOFF 0x32   // Turn off crystal oscillator.
#define CC1101_CMD_SCAL 0x33    // Calibrate freq synthesizer & disable
#define CC1101_CMD_SRX 0x34     // Enable RX.
#define CC1101_CMD_STX 0x35     // Enable TX.
#define CC1101_CMD_SIDLE 0x36   // Exit RX / TX
#define CC1101_CMD_SAFC 0x37    // AFC adjustment of freq synthesizer
#define CC1101_CMD_SWOR 0x38    // Start automatic RX polling sequence
#define CC1101_CMD_SPWD 0x39    // Enter pwr down mode when CSn goes hi
#define CC1101_CMD_SFRX 0x3A    // Flush the RX FIFO buffer.
#define CC1101_CMD_SFTX 0x3B    // Flush the TX FIFO buffer.
#define CC1101_CMD_SWORRST 0x3C // Reset real time clock.
#define CC1101_CMD_SNOP 0x3D    // No operation.
// CC1101 status register values
// -----------------------------
#define CC1101_STATUS_PARTNUM 0xF0      // Part number
#define CC1101_STATUS_VERSION 0xF1      // Current version number
#define CC1101_STATUS_FREQEST 0xF2      // Frequency offset estimate
#define CC1101_STATUS_LQI 0xF3          // Demodulator estimate for link quality
#define CC1101_STATUS_RSSI 0xF4         // Received signal strength indication
#define CC1101_STATUS_MARCSTATE 0xF5    // Control state machine state
#define CC1101_STATUS_WORTIME1 0xF6     // High byte of WOR timer
#define CC1101_STATUS_WORTIME0 0xF7     // Low byte of WOR timer
#define CC1101_STATUS_PKTSTATUS 0xF8    // Current GDOx status and packet status
#define CC1101_STATUS_VCO_VC_DAC 0xF9   // Current setting from PLL cal module
#define CC1101_STATUS_TXBYTES 0xFA      // Underflow and # of bytes in TXFIFO
#define CC1101_STATUS_RXBYTES 0xFB      // Overflow and # of bytes in RXFIFO
#define CC1101_STATUS_RCCTRL1 0xFC      //Last RC Oscillator Calibration Result
#define CC1101_STATUS_RCCTRL0 0xFD      //Last RC Oscillator Calibration Result
#define CC1101_STATUS_RX_FIFO_LEN 0x3B  //Read length of data waiting in RX_FIFO
#define CC1101_STATUS_RX_FIFO_READ 0x3F //Rread data from RX_FIFO
