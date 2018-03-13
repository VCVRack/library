<img src="https://github.com/djpeterso23662/MrLumps/blob/master/res/MrLumps_Family.jpg" width="100%">

MrLumps VCV Rack Modules
========================

This is a fork of [MrLumps](https://github.com/MrLumps) 'Fundamental' repo, and
has been changed so that it ends up as a plugin called MrLumps for use in [VCV
Rack](vcvrack.com), with both the euclidean sequencer and VCS modules.

All credit is due to Ian Kerr (MrLumps). Daniel Iel organized the three modules
into one GitHub repository and tidied up a bit. Nik Jewell’s Mac Build Monitor
script helped move the code to VCV Rack 0.4.0. Jeff Tsukuru’s script helped move
the code to VCV Rack 0.5.0. David Peterson fixed the lights and tidied up a
little more. Thanks to Martin Lueders for identifying a sample rate change bug
in SEQ-Euclid and for proposing a solution with onSampleRateChange().

David Peterson added a High Contrast option, replaced the PNG panel images with
SVG images, fixed an onSampleRateChange bug, and made MrLumps compliant with VCV
Rack’s Community Package manager in release 0.5.2.

Building
--------

This has been built against VCV Rack 0.5.1. on Linux, MacOS, and Windows.

Just type `make `in the folder. The version is now included in the Makefile
script and does not need to be given on the command line.

SEQ-Euclid
----------

A 4 channel Euclidian sequencer for [VCV Rack](vcvrack.com) based on the SEQ3
built in sequencer.

This uses ideas from the paper [The Euclidean algorithm generates traditional
musical rhythms](http://cgm.cs.mcgill.ca/~godfried/rhythm-and-mathematics.html)
from Proceedings of BRIDGES: Mathematical Connections in Art, Music, and Science
by Godfried T. Toussaint.

If you are interested in trying this yourself I would recommend looking at the
Bjorklund paper The Theory of Rep-Rate Pattern Generation in the SNS Timing
System
[SNS-NOTE-CNTRL-99](https://www.google.ca/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0ahUKEwjnms7w0vPWAhWlx4MKHafnCJQQFggpMAA&url=https%3A%2F%2Fpdfs.semanticscholar.org%2Fc652%2Fd0a32895afc5d50b6527447824c31a553659.pdf&usg=AOvVaw1CzsXZMPaPY938Z1PG5zBC)
for implementation information.

### Usage

The basic idea is that you select a bank to use, enter a pattern fill amount, a
pattern length, a probability amount and wire the output to something that needs
gates or triggers.

The algorithm created by Bjorklun that's created these patterns will take
pattern length and evenly place fill amount of beats in it.

For example 5 and 7 will result in a pattern of 1011011 while 5 and 12 results
with 100101001010.

If fill is greater than length the sequencer will output nothing but beats at
the given BPM eg 1111111....

You can make things more interesting by using the probabilty and jog controls.
The probably knob at far right will allow all beats to pass, at 12 noon 50% of
beats and far left 0 beats. The jog control will allow you to step that bank's
pattern forward by one step to allow you to offset patterns.

#### BPM

Top left is a BMP indicator and control knob. Use to set speed.

#### Clock In

You can wire up an external clock source to the input under the BPM next to the
clock icon

#### Reset

Reset either by trigger signal or button push will reset all internal counters
to 0. This has the effect of starting all banks off at the beginning of their
sequences. This is useful if you want to be sure your patterns are lined up as
you expect.

#### Gate Length

This will allow you to change the length of the gates sent by the sequencer. At
full right close beats eg patterns like 11101 the gate signals will bleed into
each other. Dial back for individual beats. I found this handy for driving the
modal synth.

#### Sequencer Banks

##### Fill Display

Shows current fill amount

##### Fill Control

Allows changing the fill amount from 0 to 256

##### Length Display

Shows current fill amount

##### Length Control

Allows changing the length amount from 0 to 256

##### Probability Control

Allows changing the % chance that a beat will be sent out. Far left 0% far right
100%.

##### Gate Out

Sends gate signals out

##### Trigger Out

Sends trigger signals out

#### Summed Outputs

##### Gate Sum Out

If there is any gate active in banks 1 through 4 a gate will be sent.

##### Trigger Sum Out

If there is any trigger active in banks 1 through 4 a gate will be sent.

#### Blinky Light

This will blink for the duration of each gate signal.

#### Context Menu Options

##### High Contrast

Right-click (Windows/Linux) or Control-Click (Mac) on the panel to access the
context menu. Click High Contrast to select easy-to-read, black-on-white colors
for the numeric display panels.

VCS - Voltage Controlled Switches for VCV Rack
----------------------------------------------

Some voltage controlled switches for [VCV Rack](vcvrack.com).

Usage
-----

The 1x8 and 2x4 modules both work the same way. You apply a trigger to the
trigger input and a signal in to the signal input.

Then you wire up as many outputs as you'd like.

Recieving trigger events will cause the module to switch to the next active
output plug.

The 2x8 module has 1 trigger input and 2 signal inputs and 4 outputs per input.
