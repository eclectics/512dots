/*--------------------------------------------------------------------------------------
    DMDGame
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
#define X 0 
#define Y 1
#define H 2
#define S 3
//globals
int controls[2][3]={{A0,A1,4},
                    {A2,A3,3}};
int ctrlstate[2][3]={{0,0,0},{0,0,0}};

byte tanks[2][4]={ {5,7,2,0},
    {27,7,6,0} };
byte cannonballs[2][4] ={{0,0,0,0},{0,0,0,0}};
byte tl[2]; //top left of block
int speeds[4]={200,400,300,150};//timeouts for next action; 0 is still, 1 is moving and rotating, 2 is full speed ahead
long events[4]={1,1,0,0}; //timeouts for next event for objects, deps on speed
long start;
int i,j,dx,dy,t,action,dir;
String mesg;
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
   pinMode(4,INPUT_PULLUP); //needed for x/y joystick, revereses values though
   pinMode(3,INPUT_PULLUP);
}

void loop(void) {
   start=millis();

   for (i=0;i<2;i++){ //tanks move
        if (events[i] && start>events[i]){
           draw(i,false);
           
            //read and respond to controls; needs to be done as move, else need to keep old position for redraw
            for (j=0;j<2;j++){
                ctrlstate[i][j]=analogRead(controls[i][j]);
            }

            //speed - forward is fast, forward and rotating is slower 
            tanks[i][S]=0; //stopped
            dir=1;
            if (ctrlstate[i][1]==1023 ) tanks[i][S]=3; //full speed ahead  unless also rotating
            if (ctrlstate[i][0]==1023 || ctrlstate[i][0]==0) tanks[i][S]=2;//rotating or reversing
            if (ctrlstate[i][1]==0){ //reverse
                tanks[i][S]=1;//reversing
                dir=-1;
            }
        
               //tanks wrap
               //move or rotate, else get artifacts left about
            if (ctrlstate[i][0]==0){
                tanks[i][H]=(tanks[i][H]+1)%8;
            } else if (ctrlstate[i][0]==1023){
                tanks[i][H]=(tanks[i][H]-1)<0?7:tanks[i][H]-1;
            }
           if (ctrlstate[i][1]==1023 || ctrlstate[i][1]==0){ //can rotate in place
               tanks[i][X]=posx(newx(tanks[i][X],tanks[i][H],dir)); 
               tanks[i][Y]=posy(newy(tanks[i][Y],tanks[i][H],dir)); 
           } 
           draw(i,true);
           events[i]=start+speeds[tanks[i][S]];
/*
        action=random(500);
        if (action<10){
           draw(i,false);
            tanks[i][H]= (tanks[i][H]+1)%8;
        } else if (action <20){
           draw(i,false);
            tanks[i][H]= (tanks[i][H]==0)?7:tanks[i][H]-1;
        } else if (action <30){
            tanks[i][S]=(tanks[i][S]+1)%4;
        } else if (action <40){
            tanks[i][S]=(tanks[i][S]==0)?3:tanks[i][S]-1;
        } else if (action <100){
            shoot(i);
        }
    */
       }
    }

   for (i=0;i<2;i++){ //cannonballs move faster
        if (events[i+2] && start>events[i+2]){
            //canonball moves, will it hit something
            dmd.writePixel(cannonballs[i][X],cannonballs[i][Y],GRAPHICS_NORMAL,false);
            cannonballs[i][X]=newx(cannonballs[i][X],cannonballs[i][H],1);
            cannonballs[i][Y]=newy(cannonballs[i][Y],cannonballs[i][H],1);
            if (cannonballs[i][X]<0 || cannonballs[i][X]>31 || cannonballs[i][Y]<0 || cannonballs[i][Y]>15){
                //misses
                events[i+2]=0; //allow another shot
                continue;
            }
            if (!hit(i)){
                dmd.writePixel(cannonballs[i][X],cannonballs[i][Y],GRAPHICS_NORMAL,true);
                events[i+2]=start+25;
            }
        }
   }
//read controls for shot
    for (i=0;i<2;i++){
        if (!digitalRead(controls[i][2])) shoot(i);
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

void shoot(byte t){
//t shootsunless it's already fired
//shoot ahead of the gun, if shoot from the end, the front falls off
if (events[t+2]==0){
cannonballs[t][X]=posx(newx(tanks[t][X],tanks[t][H],1));
cannonballs[t][Y]=posy(newy(tanks[t][Y],tanks[t][H],1));
cannonballs[t][H]=tanks[t][H]; //shoots in direction it's pointing
        events[t+2]=start+25; //traverse the board in 1.5 seconds?
        dmd.writePixel(cannonballs[t][X],cannonballs[t][Y],GRAPHICS_NORMAL,true);
    }
}

boolean hit(byte c){
    byte i;
//is the cannonball going to hit something?
    //check tanks. It's possible to hit yourself
    for (i=0;i<2;i++){ //have to hit the body of the tank
        switch (tanks[i][H]){
            case 0:
                if (tanks[i][X]-1<=cannonballs[c][X] &&
                   tanks[i][X]+1>=cannonballs[c][X] &&
                   tanks[i][Y]+1<=cannonballs[c][Y] &&
                   tanks[i][Y]+3>=cannonballs[c][Y])  goto boom;
               break;
            case 1: //easier to hit if diagonal
                if (tanks[i][X]-3<=cannonballs[c][X] &&
                   tanks[i][X]>=cannonballs[c][X] &&
                   tanks[i][Y]<=cannonballs[c][Y] &&
                   tanks[i][Y]+3>=cannonballs[c][Y]) goto boom;
               break;
            case 2:
                if (tanks[i][X]-3<=cannonballs[c][X] &&
                   tanks[i][X]-1>=cannonballs[c][X] &&
                   tanks[i][Y]-1<=cannonballs[c][Y] &&
                   tanks[i][Y]+1>=cannonballs[c][Y])  goto boom;
               break;
            case 3:
                if (tanks[i][X-3]<=cannonballs[c][X] &&
                   tanks[i][X]>=cannonballs[c][X] &&
                   tanks[i][Y]-3<=cannonballs[c][Y] &&
                   tanks[i][Y]>=cannonballs[c][Y]) goto boom;
               break;
            case 4:
                if (tanks[i][X]-1<=cannonballs[c][X] &&
                   tanks[i][X]+1>=cannonballs[c][X] &&
                   tanks[i][Y]-3<=cannonballs[c][Y] &&
                   tanks[i][Y]-1>=cannonballs[c][Y]) goto boom; 
               break;
            case 5:
                if (tanks[i][X]<=cannonballs[c][X] &&
                   tanks[i][X]+3>=cannonballs[c][X] &&
                   tanks[i][Y]-3<=cannonballs[c][Y] &&
                   tanks[i][Y]>=cannonballs[c][Y]) goto boom;
                   break;
            case 6:
                if (tanks[i][X]+1<=cannonballs[c][X] &&
                   tanks[i][X]+3>=cannonballs[c][X] &&
                   tanks[i][Y]-1<=cannonballs[c][Y] &&
                   tanks[i][Y]+1>=cannonballs[c][Y]) goto boom;
               break;
            case 7:
                if (tanks[i][X]<=cannonballs[c][X] &&
                   tanks[i][X]+3>=cannonballs[c][X] &&
                   tanks[i][Y]<=cannonballs[c][Y] &&
                   tanks[i][Y]+3>=cannonballs[c][Y]) goto boom;
               break;
        }
    }
    goto noboom;

    boom:
        events[c+2]=0;
        destroy(i);
        return true;
    noboom:
        for (i=0;i<2;i++){ //hit other bullet?
            if (i==c) continue; //skip yourself
            if (cannonballs[c][X]==cannonballs[i][X] && cannonballs[c][Y]==cannonballs[i][Y]){
                events[2+c]=0; //nothing more happens
                return true;
                //don't have to draw because will be overwritten; it's a collision after all!
            }
        }
    return false;

}

boolean destroy(byte t){

for (i=0;i<8;i++){
    draw(t,false);
    delay(50);
    draw(t,true);
    delay (50);
}

//explosion and erase
//score
//respawn
return true; //for use of calling fn
}

void draw(byte t,byte state){ //undraw current, draw new
    byte i,j;
    dmd.writePixel(tanks[t][X],tanks[t][Y],GRAPHICS_NORMAL,state);
    switch (tanks[t][H]){
        case 0:
            tl[X]=posx(tanks[t][X]-1);
            tl[Y]=posy(tanks[t][Y]+1);
            dx=0;dy=3;
            break;
        case 2:
            tl[X]=posx(tanks[t][X]-3);
            tl[Y]=posy(tanks[t][Y]-1);
            dx=-3;dy=0;
            break;
        case 4:
            tl[X]=posx(tanks[t][X]-1);
            tl[Y]=posy(tanks[t][Y]-3);
            dx=0;dy=-3;
            break;
        case 6:
            tl[X]=posx(tanks[t][X]+1);
            tl[Y]=posy(tanks[t][Y]-1);
            dx=3;dy=0;
            break;
        case 1:
            tl[X]=posx(tanks[t][X]-2);
            tl[Y]=tanks[t][Y];
            dx=-2;
            dy=2;
            break;
        case 3:
            tl[X]=posx(tanks[t][X]-2);
            tl[Y]=posy(tanks[t][Y]-2);
            dx=-2;
            dy=-2;
            break;
        case 5: 
            tl[X]=posx(tanks[t][X]); 
            tl[Y]=posy(tanks[t][Y]-2);
            dx=2;                    
            dy=-2;
            break;
        case 7:
            tl[X]=posx(tanks[t][X]);
            tl[Y]=posy(tanks[t][Y]);
            dx=2;
            dy=2;
            break;
    }

    for (i=0;i<3;i++){
        for (j=0;j<3;j++){
            dmd.writePixel(posx(tl[X]+i),posy(tl[Y]+j),GRAPHICS_NORMAL,state);
        }
    }
    if (tanks[t][H]%2==0){
        if (state) dmd.writePixel(posx(tanks[t][X]+dx),posy(tanks[t][Y]+dy),GRAPHICS_NORMAL,!state); //flip last bit
    } else { //fix up diagonal ones
            dmd.writePixel(posx(tanks[t][X]+dx+dx/2),posy(tanks[t][Y]+dy),GRAPHICS_NORMAL,state); //flip last bit
            dmd.writePixel(posx(tanks[t][X]+dx),posy(tanks[t][Y]+dy+dy/2),GRAPHICS_NORMAL,state); //flip last bit
        if (state){
            dmd.writePixel(posx(tanks[t][X]+dx),posy(tanks[t][Y]),GRAPHICS_NORMAL,!state); //flip last bit
            dmd.writePixel(posx(tanks[t][X]),posy(tanks[t][Y]+dy),GRAPHICS_NORMAL,!state); //flip last bit
        }
    }
}


byte posx(byte n){
    if (n<0) return 32+n;
    return n%32;
}
byte posy(byte n){
    if (n<0) return 16+n;
    return n%16;
}

byte newx(byte x,byte h,byte d){
    switch (h) { //heading
        case 1:
            x+=d;
            break;
        case 2:
            x+=d;
            break;
        case 3:
            x+=d;
            break;
        case 5:
            x-=d;
            break;
        case 6:
            x-=d;
            break;
        case 7:
            x-=d;
            break;
       }
    return x;
}
byte newy(byte y,byte h,byte d){
    switch (h) { //heading
        case 0:
            y-=d;
            break;
        case 1:
            y-=d;
            break;
        case 3:
            y+=d;
            break;
        case 4:
            y+=d;
            break;
        case 5:
            y+=d;
            break;
        case 7:
            y-=d;
            break;
       }
    return y;
}
