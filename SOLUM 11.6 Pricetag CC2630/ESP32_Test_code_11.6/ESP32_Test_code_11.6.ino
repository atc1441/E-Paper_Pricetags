
#include <SPI.h>

SPIClass * vspi = NULL;

#define epd_clk 15
#define epd_mosi 23
#define epd_cs 5
#define epd_dc 17
#define epd_rst 16
#define epd_busy 4

void epd_cmd(uint8_t data) {
  vspi->beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(epd_dc, LOW);
  digitalWrite(epd_cs, LOW);
  vspi->transfer(data);
  digitalWrite(epd_cs, HIGH);
  vspi->endTransaction();
}

void epd_data(uint8_t data) {
  vspi->beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(epd_dc, HIGH);
  digitalWrite(epd_cs, LOW);
  vspi->transfer(data);
  digitalWrite(epd_cs, HIGH);
  vspi->endTransaction();
}

void wait_busy() {
  delay(1);
  while (digitalRead(epd_busy)) {}
}

void setup() {
  Serial.begin(115200);
  vspi = new SPIClass(VSPI);
  vspi->begin();


  pinMode(epd_cs, OUTPUT);
  pinMode(epd_dc, OUTPUT);
  pinMode(epd_rst, OUTPUT);
  pinMode(epd_busy, INPUT);
  digitalWrite(epd_cs, HIGH);
  digitalWrite(epd_rst, HIGH);
  delay(100);
  digitalWrite(epd_rst, LOW);
  delay(100);
  digitalWrite(epd_rst, HIGH);
  delay(100);

  epd_cmd(0x12);//Software Reset
  wait_busy();

  epd_cmd(0x0C);
  epd_data(0xAE);
  epd_data(0xC7);
  epd_data(0xC3);
  epd_data(0xC0);
  epd_data(0x40);
  epd_cmd(0x01);
  epd_data(0x7F);
  epd_data(0x02);
  epd_data(0x00);
  epd_cmd(0x11);
  epd_data(0x02);
  epd_cmd(0x44);
  epd_data(0xBF);
  epd_data(0x03);
  epd_data(0x00);
  epd_data(0x00);
  epd_cmd(0x45);
  epd_data(0x00);
  epd_data(0x00);
  epd_data(0x7F);
  epd_data(0x02);
  epd_cmd(0x3C);
  epd_data(0x01);
  epd_cmd(0x18);
  epd_data(0x80);
  epd_cmd(0x22);
  epd_data(0xB1);
  epd_cmd(0x20);
  wait_busy();

  epd_cmd(0x1B);

  epd_cmd(0x14);
  epd_data(0x80);

  epd_cmd(0x4E);//Ram window set
  epd_data(0xBF);
  epd_data(0x03);
  epd_cmd(0x4F);
  epd_data(0x00);
  epd_data(0x00);
  epd_cmd(0x24);//BLack COLOR
  //Here many 0xFF
  Serial.println("OK");
  for (int r = 0; r <= 76800; r++)
  {
    epd_data(r);
  }
  
  epd_cmd(0x4E);//Ram window set
  epd_data(0xBF);
  epd_data(0x03);
  epd_cmd(0x4F);
  epd_data(0x00);
  epd_data(0x00);
  epd_cmd(0x26);//Red COLOR
  //Here many 0x00
  Serial.println("OK");
  for (int r = 0; r <= 76800; r++)
  {
    epd_data(r);
  }

  epd_cmd(0x22);
  epd_data(0xC7);
  epd_cmd(0x20);
  wait_busy();

  Serial.println("OK");
}

void loop() {
}
