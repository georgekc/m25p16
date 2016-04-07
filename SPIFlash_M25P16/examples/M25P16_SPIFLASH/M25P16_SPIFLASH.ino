#include <Arduino.h>
#include "SPIFlash.h"

#define FLASH_CS      8


SPIFlash flash(FLASH_CS);

flash_identification m25p16Variable;
void setup() {
  Serial.begin(115200);

  flash.flash_read_identification(&m25p16Variable);


  Serial.println("M25P16 SPI FLASH!!");


  Serial.print("manufacturer ID :");
  Serial.println(m25p16Variable.manufacturer, HEX);
  Serial.print("memory_type :");
  Serial.println(m25p16Variable.memory_type, HEX);
  Serial.print("memory_capacity :");
  Serial.println(m25p16Variable.memory_capacity, HEX);
  Serial.print("cfd_length :");
  Serial.println(m25p16Variable.cfd_length, HEX);

}

void loop() {
  // put your main code here, to run repeatedly:

}
