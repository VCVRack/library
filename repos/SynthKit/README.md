
# SynthKit

A series of modules for VCVRack.  This are meant to be basic building blocks
of synthesis, that will help you create bigger and more complicated
synthesizers.

### Current Status

These modules are currently under development, but are currently in a working
state.

#### Addition

Takes two inputs, adds them, clips them, and outputs the result.

#### Subtraction

Takes two inputs, subtracts the second one from the first, clips them, and
outputs the result.

#### And

Takes two inputs, calculates a logical AND, and outputs the result.

#### Or

Takes two inputs, calculates a logical OR, and outputs the result.

#### 1x8 Splitter

Takes an input, outputs the same input across 8 outputs.

#### 1x8 Splitter, CV Controlled

Takes an input, outputs the same input across any of the outputs, if the CV
trigger is active.  LED will light up for every active output.

#### Clock Divider

Takes a clock input, outputs a 1/2 width pulse across 16 pulses:

```
    1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
/1  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x
/2     x     x     x     x     x     x     x     x
/3        x        x        x        x        x
/4           x           x           x           x
/5              x              x              x
/6                 x                 x
/7                    x                    x
/8                       x                       x
```

#### Rotating Clock Divider

Works the same as the Clock Divider, except that when it is triggered, the outputs
rotate.  For each trigger, the output sequences increment one: output 1 changes to
output 2, etc.

#### Prime Clock Divider

Works the same as the Clock Divider, but only triggers on prime numbered pulses:

```
     1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19
/2      x
/3         x
/5                x
/7                     x
/11                                 x
/13                                       x
/17                                                   x
/19                                                         x
```

#### Fibonacci Clock Divider

Works the same as the Prime Clock Divider, but only triggers on fibonacci numbered pulses.

#### Simple 4-Step Sequencer

Takes a clock input, and outputs a simple 4-step sequence across one octave,
selectable.

#### DevKit

Development kit (still under development).  Currently displays minimum and
maximum values, CV trigger status (1.7v), number of triggers.

## Building

Building requires [SynthDevKit](https://github.com/JerrySievert/SynthDevKit),
which will be checked out as part of the build initialization.

```
$ git clone https://github.com/JerrySievert/SynthKit
$ cd SynthKit
$ git submodule init
$ git submodule update
$ make
```
