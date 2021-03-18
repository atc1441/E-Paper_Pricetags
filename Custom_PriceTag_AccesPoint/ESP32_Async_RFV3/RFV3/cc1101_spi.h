#pragma once

void init_spi();
void spi_start();
void spi_end();
uint8_t spi_putc(uint8_t data);
void spi_write_strobe(uint8_t spi_instr);
uint8_t spi_read_register(uint8_t spi_instr);
void spi_read_burst(uint8_t spi_instr, uint8_t *pArr, uint8_t length);
void spi_write_register(uint8_t spi_instr, uint8_t value);
void spi_write_burst(uint8_t spi_instr, uint8_t *pArr, uint8_t length);
void send_radio_tx_burst(uint8_t buffer[], uint8_t length);
