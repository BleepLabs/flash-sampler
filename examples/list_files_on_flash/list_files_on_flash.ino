#include <SerialFlash.h>
#include <SD.h>
#include <SPI.h>


const int FlashChipSelect = 6; // digital pin for flash chip CS pin
byte in8[300];

const int SDchipSelect = 10;    // Audio Shield has SD card CS on pin 10


void setup() {
  SPI.setSCK(14);  // Audio shield has SCK on pin 14
  SPI.setMOSI(7);  // Audio shield has MOSI on pin 7


  Serial.begin(9600);

  // wait for Arduino Serial Monitor
  while (!Serial) ;
  delay(100);

  if (!SerialFlash.begin(FlashChipSelect)) {
    error("Unable to access SPI Flash chip");
  }


  Serial.println("Files on Serial Flash chip:");

  SerialFlash.opendir();
  while (1) {
    char filename[64];
    uint32_t filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
      Serial.print("  ");
      Serial.print(filename);
      Serial.print(" start location= ");
      Serial.print(SerialFlash.location(filename));

      Serial.print("     length= ");
      Serial.print(filesize / 2);

      Serial.println();
    } else {
      break; // no more files
    }
  }
  Serial.println("remember these numbers!");

}


void loop() {
}

void error(const char *message) {
  while (1) {
    Serial.println(message);
    delay(2500);
  }
}

float eraseBytesPerSecond(const unsigned char *id) {
  if (id[0] == 0x20) return 152000.0; // Micron
  if (id[0] == 0x01) return 500000.0 / 2; // Spansion
  if (id[0] == 0xEF) return 419430.0; // Winbond
  if (id[0] == 0xC2) return 279620.0; // Macronix
  return 320000.0; // guess?
}


bool compareFiles(File &file, SerialFlashFile &ffile) {
  file.seek(0);
  ffile.seek(0);
  unsigned long count = file.size();
  while (count > 0) {
    char buf1[128], buf2[128];
    unsigned long n = count;
    if (n > 128) n = 128;
    file.read(buf1, n);
    ffile.read(buf2, n);
    if (memcmp(buf1, buf2, n) != 0) return false; // differ
    count = count - n;
  }
  return true;  // all data identical
}


