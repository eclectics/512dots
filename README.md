## 512Dots

These are some sketches for the Freetronics monochrome dot matrix display,
a large 16 x 32 LED display which is easily interfaced with Arduino. 

All use the [Freetronics DMD library](https://github.com/freetronics/DMD)

### dmdtank
An old school tank battle game.

This also uses 2 of the duinotech mini joysticks ($6 from Jaycar). They have
3 outputs, analog x/y and a digital out for button press.

The world wraps for tanks but not for bullets. Tanks can also pass through
each other for now. 

### dmdpong
Pong. 

I'm using the same joysticks as above, but just looking at the Y value. 

### life
John Conway's Game of Life. Totally basic, no optimisation, but runs
sufficiently quickly (at least until I add another display).

The world wraps.

### patchwork
Using an int to represent a 4x4 grid, then repeating that grid across the
display. 
