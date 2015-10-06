/*--------------------------------------------------------------------------------------
    Life
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

byte state[16][32];
byte newstate[16][32];
byte n,row,c1,c2,i,j;

void setup(void) {

   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
   start=millis();
   randomSeed(analogRead(0));
    for (i=0;i<32;i++){ //iniialise
        for (j=0;j<16;j++){
            state[j][i]=0;
            if (random(3)==0){
                state[j][i]=1;
                dmd.writePixel(i,j,GRAPHICS_NORMAL,1);
            }
        }
    }
}

void loop(void) {
   start=millis();

   //calculate new state

    //<2 dies
    //2-3 ok
    //3 repro
    //3+ overpop
    for (i=0;i<32;i++){ //iniialise
        for (j=0;j<16;j++){
           switch (neighbours(i,j)){
            case 2:
                newstate[j][i]=state[j][i];
                break;
            case 3:
                newstate[j][i]=1;
                break;
            default:
                newstate[j][i]=0;
                break;
           }
        }
     }
    for (i=0;i<32;i++){ //iniialise
        for (j=0;j<16;j++){
                state[j][i]=newstate[j][i];
                dmd.writePixel(i,j,GRAPHICS_NORMAL,state[j][i]);
            }
        }
}


byte neighbours(byte x,byte y){
    n=0;
    //how many neighours
    row=((y-1)<0)?15:y-1;
    c1=((x-1))<0?31:x-1;
    c2=(x+1)%32;
    n=state[row][c1]+state[row][x]+state[row][c2];
    n+=state[y][c1]+state[y][c2];
    row=(y+1)%16;
    n+=state[row][c1]+state[row][x]+state[row][c2];
    return n;
}
