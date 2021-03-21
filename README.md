### Use [this](https://github.com/BleepLabs/Serial_flash_sampler_for_Teensy_Audio) instead  

For dadageek 181

1 - Upload new libraries
We need to replace two libraries with modified versions. We can’t use “add zip library” unfortunately.

Windows:
Go to C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries\
Delete the “Audio” and “serialflash” directories
Download  new audio and serial flash library changes and extract them to that folder.
Download the examples we'llusing for the rest of the steps.

Mac:
Go to the arduino app, most probably in applications, and control click it. Select show package contents
Contents>Java>hardware>teensy>avr>libraries
Delete the “Audio” and “serialflash” directories
Download the new audio and serial flash libraries and extract them to that folder.
Download the code we’ll be using for the rest of the steps.
2 - Prepare files in audacity
Get audacity here

Convert to mono if necessary
Tracks > stereo track to mono
Remove one of the track by clicking the X in its the top left corner

Export the sample as raw data
File > export audio
Save as type : Other uncompressed files
Click “options…” and make sure it is set to:
header: “RAW(header-less)”
Encoding: “Signed 16 bit PCM”
Save the file to the SD card. The name should only be letters and numbers and less than 8 characters long


3 - Copy files to serial flash
The serial flash chip is a were the files need to be stored so the teensy can access them quickly.

Make sure only the files you want to copy are on the flash card. No directories or anything, just your .raw files.

Insert the card into the audio shield SD card adapter. 

Open and upload “Sd_to_flash_-_erase_copy_list”
This file will erase everything on the serial flash chip and copy the samples from the SD card to serial flash. 

Once it’s uploaded open the serial monitor and the process will begin.


4 - Set up the waveform objects
At the end of the upload you’ll see something like this 

Files on Serial Flash chip:
  Test1.raw    start location= 32768     length= 441009
  2.RAW    start location= 914944     length= 24231
  beep.RAW    start location= 963584     length= 44101
remember these numbers!

You’ll need to input these start and length numbers in your code so it knows where the files are in the flash memory. 
If you need this info again “list_files_on_flash.ino” will print it out without erasing or loading anything.

 waveform1.begin(1, 0, SERIAL_FLASH_SAMPLE);  
 waveform1.sample_begin(32768, 441009);     

Here we’ve set up waveform 1 to playback the “Test1.raw” file
See “serial_flash_sample_example_1.ino” for more info 



