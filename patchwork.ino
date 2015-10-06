/*--------------------------------------------------------------------------------------
   Patchwork

Define a 4x4 block, repeat it

   Using the Freetronics DMD, a 512 LED matrix display
   panel arranged in a 32 x 16 layout.

 See http://www.freetronics.com/dmd for resources and a getting started guide.


--------------------------------------------------------------------------------------*/
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

//array indexes for position, heading, speed
long start;
void ScanDMD()
  //Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  //called at the period set in Timer1.initialize();
{ 
  dmd.scanDisplayBySPI();
}

unsigned int pattern;
byte i,j;
void setup(void) {

   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
pattern=0;
}

void loop(void) {
   pattern=random(65536);
   /*
   for (i=0;i<16;i++){
        if (random(2)) bitSet(pattern,i);
        else bitClear(pattern,i);
    }
    */
    for (i=0;i<32;i++){
        for (j=0;j<15;j++){
            dmd.writePixel(i,j,GRAPHICS_NORMAL,bitRead(pattern,4*(j%4)+i%4));
        }
    }
    delay(1000);
}

