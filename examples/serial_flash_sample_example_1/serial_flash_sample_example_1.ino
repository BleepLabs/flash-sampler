#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform3;      //xy=207,310
AudioSynthWaveform       waveform2;      //xy=234,256
AudioSynthWaveform       waveform1;      //xy=281,214
AudioMixer4              mixer1;         //xy=504,248
AudioOutputI2S           i2s1;           //xy=675,271
AudioConnection          patchCord1(waveform3, 0, mixer1, 2);
AudioConnection          patchCord2(waveform2, 0, mixer1, 1);
AudioConnection          patchCord3(waveform1, 0, mixer1, 0);
AudioConnection          patchCord4(mixer1, 0, i2s1, 0);
AudioConnection          patchCord5(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=436,79
// GUItool: end automatically generated code


#define FLASH_CS_PIN    6    //These three are necessary to read the flash chip 
#define FLASH_MOSI_PIN  7
#define FLASH_SCK_PIN   14

float freq[16] = {};
int rev;
int button1_pin = 32;
int button2_pin = 28;

int bu[8]; // for reading buttons
int pbu[8];

uint32_t cm, prev[4];

void setup() {
  AudioNoInterrupts(); //turn off the audio library so we can set things up

  AudioMemory(20);

  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);

  SPI.setMOSI(FLASH_MOSI_PIN);   //These three are necessary to read the flash chip
  SPI.setSCK(FLASH_SCK_PIN);
  SerialFlash.begin(FLASH_CS_PIN);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6); 

  waveform1.begin(1, 0, SERIAL_FLASH_SAMPLE);  //same as other waveforms (amplitue, frequency, type)
  waveform1.sample_begin(32768, 441009);   //(start location, length) get these numbers from "list files on flash.ino"
  
  waveform2.begin(1, 0, SERIAL_FLASH_SAMPLE);
  waveform2.sample_begin(914944, 24231);

  waveform3.begin(1, 0, SERIAL_FLASH_SAMPLE);
  waveform3.sample_begin(963584, 44101);

  mixer1.gain(0, .3);
  mixer1.gain(1, .3);
  mixer1.gain(2, .3);


  waveform1.sample_loop(1);   // (1) makes it loop, (0) plays once
  waveform2.sample_loop(1);
  waveform3.sample_loop(1);

  waveform1.sample_play();   //start playing. If it's already playing this will bring it back to the start position
  waveform2.sample_play();
  waveform3.sample_play();

  AudioInterrupts();

}



void loop() {


  cm = millis();

  pbu[0] = bu[0];
  bu[0] = digitalRead(32);

  pbu[1] = bu[1];
  bu[1] = digitalRead(28);


  if (pbu[0] == 1 && bu[0] == 0) {
    rev = !rev;
      //this will play the sample back in reverse if rev==1, starting from the end of sample_play_len to sample_start_pos
      waveform1.sample_reverse(rev);
  }


  if (pbu[1] == 1 && bu[1] == 0) {

    int is_it_playing = waveform1.sample_status(); //returns a 1 if it's playing and a 0 is not.

    if (is_it_playing == 1) {
      waveform1.sample_stop();
    }

    if (is_it_playing == 0) {
      waveform1.sample_play();
    }

  }

  // the freqeuncy acts differntly for the samples.
  // 86.1679 plays it back at the recorded rate.
  // here were dividing the 1023 max by 3 to get a max of 341
  // which would be about 4 times speed. You can go aster but iy takes up a lot of processor

  freq[1] = analogRead(A16) / 3.00;

  // This means about 9/10 of the pot is usied for playing is back fast and only 1/10 for slow
  // Here i've made a function to fix that
  // It will play back the sample at normal speed when at 12:00. As you trun right it goes up to 5x speed. As you turn left it goes all the way down to 0
  freq[2] = sample_freq(A15);
  freq[3] = sample_freq(A14);


  waveform1.frequency(freq[1]);
  waveform2.frequency(freq[2]);
  waveform3.frequency(freq[3]);


  // you can use these to control where the sample start and how long it will play before stopping or looping
  // a sample_start_pos of .5 means it will start halfway through the entire length of the sample
  // sample_play_len of .25 means it will play for a wauter of the length of the sample starting at the previously defined sample_start_pos
  // if the length hits the end of the file it will stop or loop.

  //waveform1.sample_start_pos(.5);
  //waveform1.sample_play_len(.25);

  //if you wanted to chage these with a pot youd :
  // sp1=analogRead(?)/1023.00 //making sure the varible is a float



  if (cm - prev[0] > 250 && 1 == 1) {
    prev[0] = cm;
    Serial.print("processor usage: ");
    Serial.print(AudioProcessorUsageMax());  //go over 90% and you'll start to glitch out
    Serial.print("%   memory usage: ");
    Serial.println(AudioMemoryUsageMax()); //if this number is bigger than what you entered in  AudioMemory(??) in setup make it bigger
    Serial.print("freq[3] ");
    Serial.println(freq[3]);
    Serial.println();

    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
  }

}


float sample_freq(int pin) {
  int pot_max = 1024;
  float norm = 86.1679;
  float max_speed_mult = 4;
  float r1 = analogRead(pin);
  float out;
  if (r1 >= pot_max / 2) {
    float t1 = (r1 - (pot_max / 2));
    out = (t1 * (max_speed_mult / (pot_max / 2))) + 1;
    out *= norm;
  }
  if (r1 < pot_max / 2) {
    out = (r1 / (pot_max / 2));
    out *= norm;
  }

  return out;
}



