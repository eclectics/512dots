/*--------------------------------------------------------------------------------------
    DMDPong
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

#define X 0
#define Y 1
#define H 2

//globals
byte ball[3]; //x,y,heading
int paddle[2][2]={ {7,0},{7,0}};//curr y, change
byte i,j;
byte score[2]={0,0};
byte returned=1;
int controls[2]={A1,A3}; //y on joystick
int ctrlstate[2]; //reading controls
long events[4]={1,1,1,1}; //timeouts for next event for objects, deps on speed
long timeouts[4]={40,40,60,25}; //read control much more often than actually move
long start;

void ScanDMD()
  //Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  //called at the period set in Timer1.initialize();
{ 
  dmd.scanDisplayBySPI();
}


void setup(void) {

   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
   start=millis();
   randomSeed(analogRead(0));
   ball[X]=16;
   ball[Y]=random(16);
   ball[H]=random(8);
    for (i=0;i<4;i++){
        dmd.writePixel(0,7+i,GRAPHICS_NORMAL,1);
        dmd.writePixel(31,7+i,GRAPHICS_NORMAL,1);
    }
    //net
    for (i=0;i<16;i+=2){
        dmd.writePixel(16,i,GRAPHICS_NORMAL,1);
    }
}

void loop(void) {
   start=millis();

  for (i=0;i<4;i++){ //paddles move
    if (events[i] && start>events[i]){
        switch (i){
            case 2: //ball
                if (ball[X]!=0 && ball[X]!=16 && ball[X]!=31){
                    dmd.writePixel(ball[X],ball[Y],GRAPHICS_NORMAL,false);
                }

                ball[X]=newx(ball[X],ball[H]);
                ball[Y]=newy(ball[Y],ball[H]);
                //bounce off walls
                //when there's paddles, will just bounce off Y, X is point!
                returned=true;
                if (ball[X]==0) { //poss of score
                   if (!(paddle[0][0]<=ball[Y] && (paddle[0][0]+3)>=ball[Y])){ 
                    returned=false;
                    score[1]++;
                   } 
                } else if (ball[X]==31){
                   if (!(paddle[1][0]<=ball[Y] && (paddle[1][0]+3)>=ball[Y])){ 
                    returned=false;
                    score[0]++;
                   } 
                }
                if (returned){
                    if (ball[X]==0 || ball[X]==31){
                        switch (ball[H]){
                            case 2:case 6:
                                ball[H]=(ball[H]+4)%8;
                                break;
                            case 1:case 5:
                                ball[H]=(ball[H]+6)%8;
                                break;
                            case 7:case 3:
                                ball[H]=(ball[H]+2)%8;
                                break;
                        }
                    }
                if(ball[Y]<0 || ball[Y]>15){
                    switch (ball[H]){
                        case 0:case 4: 
                            ball[H]=(ball[H]+4)%8;
                            break;
                        case 1: case 5: 
                            ball[H]=ball[H]+2;
                            break;
                        case 3:case 7:
                            ball[H]=ball[H]-2;
                                break;
                        } 
                }
                events[i]=start+timeouts[i];
                } else {
                    events[2]=start+2000;
                    ball[X]=16;
                    ball[Y]=random(16);
                }
                if (ball[X]!=16){
                    dmd.writePixel(ball[X],ball[Y],GRAPHICS_NORMAL,true);
                }
                break;
            case 3: //read controls
                for (j=0;j<2;j++){
                    ctrlstate[j]=analogRead(controls[j]); 
                    paddle[j][1]=0;
                    if (ctrlstate[j]==1023) paddle[j][1]=-1;
                if (ctrlstate[j]==0) paddle[j][1]=1;
            }
            events[i]=start+timeouts[i];
            break;
        case 0: case 1://move paddles
            if (paddle[i][1]==1 && paddle[i][0]<12){
                dmd.writePixel(i?31:0,paddle[i][0],GRAPHICS_NORMAL,0); //get rid of top
                paddle[i][0]++;
                dmd.writePixel(i?31:0,paddle[i][0]+3,GRAPHICS_NORMAL,1); //get rid of top
            } 
            if (paddle[i][1]==-1 && paddle[i][0]>0){
                dmd.writePixel(i?31:0,paddle[i][0]+3,GRAPHICS_NORMAL,0);//get rid of bottom 
                paddle[i][0]--;
                dmd.writePixel(i?31:0,paddle[i][0],GRAPHICS_NORMAL,1); 
            }
            events[i]=start+timeouts[i];
            break; 
        }
    }
}


}


//DMD
/*
//writePixed(bX,bY,bGraphicsMode,bPixel)
mode GRAPHICS_NORMAL - bPixel sets
GRAPHICS INVERSE - bPixel revers sets
GRAPHCIS TOGGLE - bPixedl togges
Graphics_OR - only sets on
NOR - only turn soff
*/


boolean hit(byte c){
    byte i;
    return false;
}



byte posx(byte n){
    if (n<0) return 32+n;
    return n%32;
}
byte posy(byte n){
    if (n<0) return 16+n;
    return n%16;
}

byte newx(byte x,byte h){
    switch (h) { //heading
        case 1:
            x+=1;
            break;
        case 2:
            x+=1;
            break;
        case 3:
            x+=1;
            break;
        case 5:
            x-=1;
            break;
        case 6:
            x-=1;
            break;
        case 7:
            x-=1;
            break;
       }
    return x;
}
byte newy(byte y,byte h){
    switch (h) { //heading
        case 0:
            y-=1;
            break;
        case 1:
            y-=1;
            break;
        case 3:
            y+=1;
            break;
        case 4:
            y+=1;
            break;
        case 5:
            y+=1;
            break;
        case 7:
            y-=1;
            break;
       }
    return y;
}
