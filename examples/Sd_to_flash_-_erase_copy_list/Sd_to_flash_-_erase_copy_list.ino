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
  Serial.println("Initializing SD card...");
  if (!SD.begin(SDchipSelect)) {
    error("Unable to access SD card. Insert card into audio shield and restart");
  }

  if (!SerialFlash.begin(FlashChipSelect)) {
    error("Unable to access SPI Flash chip");
  }




  Serial.println("Files on SD card:");
  File root = SD.open("/");
  uint32_t total_bytes;
  uint32_t total_seconds;
  int bcnt;
   while(true) {
     
     File entry =  root.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<0; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
     } else {
       // files have sizes, directories do not
       uint32_t ab = entry.size();
       total_bytes += ab;
        bcnt++;

       Serial.print("\t");
       Serial.println(ab, DEC);
     }
     entry.close();
   }  

  Serial.print(total_bytes);      Serial.println(" total bytes on card.");
  Serial.print("This is about ");
  int32_t sd_seconds = total_bytes / 44100 / 2;
  Serial.print(sd_seconds);
  Serial.println(" seconds of audio.");
  Serial.println();

  unsigned long startMillis = millis();
  unsigned char id[5];
  SerialFlash.readID(id);
  unsigned long size = SerialFlash.capacity(id);


  Serial.print("Flash Memory has ");
  Serial.print(size);
  Serial.println(" bytes.");
  Serial.print(" This will give about ");
  int32_t flash_seconds = size / 44100 / 2;
  Serial.print(flash_seconds);
  Serial.println(" seconds of audio.");
  int32_t dif = flash_seconds - sd_seconds;
  Serial.print(dif);
  Serial.println(" space left over in serial flash");
    Serial.println(" ");
    Serial.println(" ");

  int ok = 1;
  if (dif < 1) {
    ok = 0;
    Serial.println("Not enough space. Remove some files");
    Serial.println("Flash not erased");
  }

  if (ok == 1) {
    Serial.println("NOW ERASING EVERYTHING IN SERIAL FLASH");
    Serial.print(" (SD card is not changed)");
    delay(2000);
    Serial.println("IN : ");
    Serial.println("5");
    delay(1000);
    Serial.println("4");
    delay(1000);
    Serial.println("3");
    delay(1000);
    Serial.println("2");
    delay(1000);
    Serial.println("1");
    delay(1000);
    Serial.println("0");
    delay(1000);



    Serial.println("Erasing ALL Flash Memory:");
    // Estimate the (lengthy) wait time.
    Serial.print("  estimated wait: ");
    int seconds = (float)size / eraseBytesPerSecond(id) + 0.5;
    Serial.print(seconds);
    Serial.println(" seconds.");
    Serial.println("  Yes, full chip erase is kinda slow");
    SerialFlash.eraseAll();
    unsigned long dotMillis = millis();
    unsigned char dotcount = 0;
    while (SerialFlash.ready() == false) {
      if (millis() - dotMillis > 500) {
        dotMillis = dotMillis + 500;
        Serial.print(".");
        dotcount = dotcount + 1;
        if (dotcount >= 60) {
          Serial.println();
          dotcount = 0;
        }
      }
    }
    if (dotcount > 0) Serial.println();
    Serial.println("Erase completed");
    unsigned long elapsed = millis() - startMillis;
    Serial.print("  actual wait: ");
    Serial.print(elapsed / 1000ul);
    Serial.println(" seconds.");


    ////////////////////////////////////////////////////////////////////////////////////////


    Serial.println("Copy all files from SD Card to SPI Flash");

    File rootdir = SD.open("/");
    while (1) {
      // open a file from the SD card
      Serial.println();
      File f = rootdir.openNextFile();
      if (!f) break;
      const char *filename = f.name();
      Serial.print(filename);
      Serial.print("    ");
      unsigned long length = f.size();
      Serial.println(length);

      // check if this file is already on the Flash chip
      if (SerialFlash.exists(filename)) {
        Serial.println("  already exists on the Flash chip");
        SerialFlashFile ff = SerialFlash.open(filename);
        if (ff && ff.size() == f.size()) {
          Serial.println("  size is the same, comparing data...");
          if (compareFiles(f, ff) == true) {
            Serial.println("  files are identical :)");
            f.close();
            ff.close();
            continue;  // advance to next file
          } else {
            Serial.println("  files are different");
          }
        } else {
          Serial.print("  size is different, ");
          Serial.print(ff.size());
          Serial.println(" bytes");
        }
        // delete the copy on the Flash chip, if different
        Serial.println("  delete file from Flash chip");
        SerialFlash.remove(filename);
      }

      // create the file on the Flash chip and copy data
      if (SerialFlash.create(filename, length)) {
        SerialFlashFile ff = SerialFlash.open(filename);
        if (ff) {
          Serial.print("  copying");
          // copy data loop
          unsigned long count = 0;
          unsigned char dotcount = 9;
          while (count < length) {
            char buf[256];
            unsigned int n;
            n = f.read(buf, 256);
            ff.write(buf, n);
            count = count + n;
            if (count % 25 == 0) {
              Serial.print(".");
              dotcount++;
            }
            if (dotcount > 75) {
              Serial.println();
              dotcount = 0;
            }
          }
          ff.close();
          if (dotcount > 0) Serial.println();
        } else {
          Serial.println("  error opening freshly created file!");
        }
      } else {
        Serial.println("  unable to create file");
      }
      f.close();
    }
    rootdir.close();
    delay(10);
    Serial.println("Finished copying all files");
    Serial.println();
    Serial.println();


    ////////////////////////////////////////////////////////////////////////////////////////



    Serial.println("Files now on SPI Flash chip:");

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


