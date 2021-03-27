#include <Arduino.h>
#include "RFV3.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "logger.h"
#include "main_variables.h"

uint8_t cc1101_frequency_list[73 * 3] = {
    0x21, 0x63, 0xF0, 0x21, 0x66, 0xE4, 0x21, 0x6B, 0xD0, 0x21, 0x71, 0x7A,
    0x21, 0x74, 0xAD, 0x21, 0x33, 0x33, 0x21, 0x37, 0x23, 0x21, 0x3B, 0x13,
    0x21, 0x3F, 0x03, 0x21, 0x42, 0xF4, 0x22, 0xB6, 0x27, 0x22, 0xBB, 0x13,
    0x22, 0xC0, 0x00, 0x22, 0xC4, 0xEC, 0x22, 0xC9, 0xD8, 0x22, 0xCE, 0xC4,
    0x22, 0xD3, 0xB1, 0x22, 0xD8, 0x9D, 0x22, 0xDD, 0x89, 0x22, 0xE2, 0x76,
    0x22, 0xE7, 0x62, 0x22, 0xEC, 0x4E, 0x22, 0xF1, 0x3B, 0x22, 0xF6, 0x27,
    0x22, 0xFB, 0x13, 0x23, 0x00, 0x00, 0x23, 0x04, 0xEC, 0x23, 0x09, 0xD8,
    0x23, 0x0E, 0xC4, 0x23, 0x13, 0xB1, 0x23, 0x18, 0x9D, 0x23, 0x1D, 0x89,
    0x23, 0x22, 0x76, 0x23, 0x27, 0x62, 0x23, 0x2C, 0x4E, 0x23, 0x31, 0x3B,
    0x23, 0x36, 0x27, 0x23, 0x3B, 0x13, 0x23, 0x40, 0x00, 0x23, 0x44, 0xEC,
    0x23, 0x49, 0xD8, 0x23, 0x4E, 0xC4, 0x23, 0x53, 0xB1, 0x23, 0x58, 0x9D,
    0x23, 0x5D, 0x89, 0x23, 0x62, 0x76, 0x23, 0x67, 0x62, 0x23, 0x6C, 0x4E,
    0x23, 0x71, 0x3B, 0x23, 0x76, 0x27, 0x23, 0x7B, 0x13, 0x23, 0x80, 0x00,
    0x23, 0x84, 0xEC, 0x23, 0x89, 0xD8, 0x23, 0x8E, 0xC4, 0x23, 0x93, 0xB1,
    0x23, 0x98, 0x9D, 0x23, 0x9D, 0x89, 0x23, 0xA2, 0x76, 0x23, 0xA7, 0x62,
    0x23, 0xAC, 0x4E, 0x21, 0x3C, 0x0F, 0x21, 0x3E, 0x07, 0x21, 0x40, 0x00,
    0x21, 0x41, 0xF8, 0x21, 0x43, 0xF0, 0x21, 0x6A, 0x56, 0x21, 0x6C, 0x4E,
    0x23, 0x5E, 0x07, 0x23, 0x5F, 0x04, 0x23, 0x60, 0x00, 0x23, 0x60, 0xFC,
    0x23, 0x61, 0xF8};

int curr_freq = -1;
int curr_net_id = -1;

uint8_t init_radio()
{
  log_verbose("Radio init");
  digitalWrite(SS_PIN, LOW);
  delayMicroseconds(10);
  digitalWrite(SS_PIN, HIGH);
  delayMicroseconds(40);
  spi_write_strobe(CC1101_CMD_SRES);
  delay(10);

  uint8_t version_cc1101 = spi_read_register(CC1101_STATUS_VERSION);
  log_verbose("Radio version: " + String(version_cc1101));
  if (version_cc1101 == 0x00 || version_cc1101 == 0xFF)
  {
    return 1;
  }

  if (cc1101_test_gpio(CC1101_REG_IOCFG2, GDO2))
  {
    return 2;
  }

  spi_write_register(CC1101_REG_IOCFG2, 0x06);  // GPIO2 Interrupt on SYNC send or received
  spi_write_register(CC1101_REG_IOCFG0, 0x0E);  // GPIO0 Interrupt on threshold RSSI for LBT
  spi_write_register(CC1101_REG_FIFOTHR, 0x00); // GPIO2 Interrupt threshold
  spi_write_register(CC1101_REG_PKTLEN, 0xFF);
  spi_write_register(CC1101_REG_PKTCTRL1, 0b00001110); //CRC_flush,Append_RSSI_&_LQI,Address_&_Broadcast_check
  spi_write_register(CC1101_REG_PKTCTRL0, 0b00000101); //CRC_EN,packet_len_first_byte_after_sync
  spi_write_register(CC1101_REG_CHANNR, 0x00);
  spi_write_register(CC1101_REG_FSCTRL1, 0x06);
  spi_write_register(CC1101_REG_FSCTRL0, 0x00);
  spi_write_register(CC1101_REG_MDMCFG4, 0xCA);
  spi_write_register(CC1101_REG_MDMCFG3, 0x83);
  spi_write_register(CC1101_REG_MDMCFG2, 0x13);
  spi_write_register(CC1101_REG_MDMCFG1, 0x22);
  spi_write_register(CC1101_REG_MDMCFG0, 0xf8);
  spi_write_register(CC1101_REG_DEVIATN, 0x34);
  spi_write_register(CC1101_REG_MCSM1, 0b00000010); //After_RX=IDLE,After_TX=Stay_TX
  spi_write_register(CC1101_REG_MCSM0, 0b00001000); //Manual calibrate,PO_Timeout=64
  spi_write_register(CC1101_REG_FOCCFG, 0x16);
  spi_write_register(CC1101_REG_BSCFG, 0x6c);
  spi_write_register(CC1101_REG_AGCCTRL2, 0x43);
  spi_write_register(CC1101_REG_AGCCTRL1, 0x4B);
  spi_write_register(CC1101_REG_AGCCTRL0, 0x91);
  spi_write_register(CC1101_REG_FREND1, 0x56);
  spi_write_register(CC1101_REG_FREND0, 0x10);
  spi_write_register(CC1101_REG_FSCAL3, 0xE9);
  spi_write_register(CC1101_REG_FSCAL2, 0x2A);
  spi_write_register(CC1101_REG_FSCAL1, 0x00);
  spi_write_register(CC1101_REG_FSCAL0, 0x1F);
  spi_write_register(CC1101_REG_FSTEST, 0x59);
  spi_write_register(CC1101_REG_TEST2, 0x81);
  spi_write_register(CC1101_REG_TEST1, 0x35);
  spi_write_register(CC1101_REG_TEST0, 0x09);
  spi_write_register(CC1101_REG_PATABLE, 0xC0);

  cc1101_idle();
  CC1101_set_freq(get_wu_channel());
  CC1101_set_net_id(get_network_id());

  return 0;
}

uint8_t cc1101_test_gpio(uint8_t GPIO_CONFIG_REG, uint8_t pin)
{
  pinMode(pin, INPUT);
  spi_write_register(GPIO_CONFIG_REG, 0b01101001); //Active_LOW,CHIP_RDY_n
  delay(5);
  if (!digitalRead(pin))
    return 1;
  spi_write_register(GPIO_CONFIG_REG, 0b00101001); //Active_HIGH,CHIP_RDY_n
  delay(5);
  if (digitalRead(pin))
    return 2;
  return 0;
}

void CC1101_set_freq(uint8_t freq)
{
  uint8_t *s = (uint8_t *)&cc1101_frequency_list[(3 * freq)];
  if (freq >= 73)
  {
    log_normal("ERROR: Freq too high " + String(freq));
    return;
  }
  if (curr_freq != freq)
  {
    curr_freq = freq;
    spi_write_register(CC1101_REG_FREQ2, s[0]);
    spi_write_register(CC1101_REG_FREQ1, s[1]);
    spi_write_register(CC1101_REG_FREQ0, s[2]);
    uint32_t IF = (s[0] << 16) + (s[1] << 8) + s[2]; // 24-bit value
    float f = (26000000.0f / 65536.0f) * (float)(IF);
    log_verbose("Radio set to base freq: (F=" + String(freq) + ") " + String(f / 1000000.0f) + " Mhz ");
  }
}

uint8_t _freq_offset = 0;
void CC1101_set_freq_offset(uint8_t freq_offset)
{
  _freq_offset = freq_offset;
  spi_write_register(CC1101_REG_FSCTRL0, freq_offset);
  log_verbose("Radio base freq offset: " + String(freq_offset));
}

uint8_t get_freq_offset()
{
  return _freq_offset;
}

void CC1101_set_net_id(uint8_t id)
{
  if (curr_net_id != id)
  {
    curr_net_id = id;
    spi_write_register(CC1101_REG_ADDR, id);
    log_verbose("Radio set id done " + String(id));
  }
}

void cc1101_flush_buffers()
{
  spi_write_strobe(CC1101_CMD_SFTX);
  delayMicroseconds(100);
  spi_write_strobe(CC1101_CMD_SFRX);
  delayMicroseconds(100);
}

void cc1101_idle()
{
  spi_write_strobe(CC1101_CMD_SIDLE);

  while ((spi_read_register(CC1101_STATUS_MARCSTATE) & 0x1F) != 0x01)
  {
  }

  delayMicroseconds(100);
  cc1101_flush_buffers();
}

int broadcast_mode = -1;
int threshold_mode = -1;
void cc1101_rx(uint8_t broadcast_rx)
{
  if (broadcast_rx)
  {
    if (broadcast_mode == 0)
    {
      spi_write_register(CC1101_REG_PKTCTRL1, 0b00001100); //CRC_flush,Append_RSSI_&_LQI,No Address check
      broadcast_mode = 1;
    }
  }
  else
  {
    if (broadcast_mode == 1)
    {
      spi_write_register(CC1101_REG_PKTCTRL1, 0b00001110); //CRC_flush,Append_RSSI_&_LQI,Address_&_Broadcast_check
      broadcast_mode = 0;
    }
  }
  if (threshold_mode == 1)
  {
    spi_write_register(CC1101_REG_FIFOTHR, 0x0E); // Set RX threshold to 64byte's
    threshold_mode = 0;
  }
  spi_write_strobe(CC1101_CMD_SIDLE);
  cc1101_flush_buffers();
  spi_write_strobe(CC1101_CMD_SRX);

  while ((spi_read_register(CC1101_STATUS_MARCSTATE) & 0x1F) != 0x0D)
  {
  }

  delayMicroseconds(100);
}

void cc1101_tx()
{
  if (threshold_mode == 0)
  {
    spi_write_register(CC1101_REG_FIFOTHR, 0x00); // Set TX threshold to 61byte's
    threshold_mode = 1;
  }
  spi_write_strobe(CC1101_CMD_STX);
  delayMicroseconds(100);
}

uint8_t cc1101_prepaire_tx(uint8_t input_freq, uint8_t input_net_id)
{

  cc1101_idle();
  CC1101_set_freq(input_freq);
  CC1101_set_net_id(input_net_id);
  spi_write_strobe(CC1101_CMD_SCAL);

  long last_100_millis = millis();
  uint8_t temp_read_cali = 0;
  while (temp_read_cali != 1)
  {
    temp_read_cali = spi_read_register(CC1101_STATUS_MARCSTATE);
    if (millis() - last_100_millis > 100)
    {
      return 1; //Calibration 1 timeout
    }
  }
  spi_write_strobe(CC1101_CMD_SRX);
  while (temp_read_cali != 0x0D)
  {
    temp_read_cali = spi_read_register(CC1101_STATUS_MARCSTATE);
    if (millis() - last_100_millis > 100)
    {
      return 2; //Calibration 2 timeout
    }
  }
  cc1101_idle();
  return 0;
}

void cc1101_tx_fill(uint8_t buffer[], uint8_t length)
{
  spi_start();
  spi_putc(0x3F | 0x40);
  spi_putc(length);
  for (uint8_t i = 0; i < length; i++)
  {
    spi_putc(buffer[i]);
  }
  spi_end();
}

int cc1101_read_fifo(uint8_t buffer[])
{
  uint8_t fifo_len = spi_read_register(CC1101_STATUS_RX_FIFO_LEN);

  if ((fifo_len & 0x7F) && !(fifo_len & 0x80))
  {
    spi_read_burst(0x3F, buffer, fifo_len + 1);
    uint8_t rssi = buffer[fifo_len - 1];
    uint8_t lqi = buffer[fifo_len];
    log_normal("RSSI: " + String(rssi) + " LQI: " + String(lqi & 0x7F));
    return fifo_len - 1;
  }
  return -1;
}