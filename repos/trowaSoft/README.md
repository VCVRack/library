# trowaSoft-VCV
<div>
<img src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2fdemo_screenshot.jpg" />
</div>

trowaSoft Modules plugin for [VCV Rack](https://github.com/VCVRack/Rack) v0.5.0. Current pack includes [trigSeq &amp; trigSeq64](#trigseq--trigseq64), [voltSeq](#voltseq), and [multiScope](#multiscope).

For more information about these modules, please visit:
http://www.geekasaurusrex.net/page/trowaSoft-Sequencer-Modules-for-VCV-Rack.aspx.

For more information about Rack, please visit:
https://vcvrack.com/.

## Binaries/Builds
Any builds that are currently available are at [Github Releases page](https://github.com/j4s0n-c/trowaSoft-VCV/releases) and [geeksaurusrex](http://www.geekasaurusrex.net/page/trowaSoft-Sequencer-Modules-for-VCV-Rack.aspx). 
**2018-02-17**: The latest version is [v0.5.5.2](https://github.com/j4s0n-c/trowaSoft-VCV/releases/tag/v0.5.5.2).

To build for your platform, please visit the [VCV rack documentation](https://github.com/VCVRack/Rack#setting-up-your-development-environment).

## Sequencers
Currently there are three (3) sequencer modules.

### trigSeq &amp; trigSeq64
<div>
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2ftrigSeq_screenshot.jpg" />
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2ftrigSeq64_screenshot.jpg" />
</div>


These are basic boolean on/off pad step sequencers (0V or 10V), based off the [Fundamentals SEQ3 sequencer](https://github.com/VCVRack/Fundamental).
+ **trigSeq** is 16-step; **trigSeq64** is 64-step.
+ Now 64 patterns. ~~16 patterns.~~
+ 16 channels (outputs).
+ Output modes: **TRIG** (trigger), **RTRG** (retrigger), **GATE** (continuous) (0 or 10V).
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.
+ Open Sound Control (OSC) interface (as of v.0.5.5.1). [more info](https://github.com/j4s0n-c/trowaSoft-VCV/wiki/Open-Sound-Control-(OSC)-Interface)
+ Advanced Randomization options (as of v.0.5.5.2) for all patterns, current edit pattern, or only the displayed channel. Chose from 'normal random' or 'structured' random patterns.
+ Hold mouse down and set multiple pads by dragging.

### voltSeq
<div>
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2fvoltSeq_screenshot.jpg" />
</div>


Variable voltage output step sequencer (-10V to +10V), based off the [Fundamentals SEQ3 sequencer](https://github.com/VCVRack/Fundamental).
+ **voltSeq** is 16-step.
+ Now 64 patterns. ~~16 patterns.~~
+ 16 channels (outputs).
+ Output modes:
    + **VOLT** - Voltage (-10V to +10V): Output whatever voltage you want.
    + **NOTE** - Midi Note (-4V to +6V) ~~(-5V to +5V)~~: Output notes (12 notes per 1 V; 10 octaves). [Base pitch (0V)](https://github.com/j4s0n-c/trowaSoft-VCV/issues/9) is now C4 (as of v0.5.5.2).
    + **PATT** - Pattern (-10V to +10V): To control the currently playing Pattern (or Length) on another **trigSeq** or **voltSeq**. (Now 1 to 64 in range).  
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.
+ Open Sound Control (OSC) interface (as of v.0.5.5.1). [more info](https://github.com/j4s0n-c/trowaSoft-VCV/wiki/Open-Sound-Control-(OSC)-Interface)
+ Advanced Randomization options (as of v.0.5.5.2) for all patterns, current edit pattern, or only the displayed channel. Chose from 'normal random' or 'structured' random patterns.
+ Shift Values (as of v0.5.5.2): +/- 1 Volt or 1 Octave or 1 Pattern for all patterns, current edit pattern, or only the displayed channel.

## Other
### multiScope
<div>
<img width="700" src="https://github.com/j4s0n-c/trowaSoft-VCV/blob/master/screenshots/multiScope_screenshot_02.png?raw=true" />
</div>

**multiScope** is a visual effects scope, with lissajous mode, that allows three (3) waveforms to be drawn on the same screen/canvas. (code based on [JW Modules FullScope](https://github.com/jeremywen) and [Fundamental Scope](https://github.com/VCVRack/Fundamental))

**WARNING**: New version seems to crash on Mac OS. If anyone more adept at OSX programming wants to figure out why, it would be **MUCH** appreciated. Otherwise, whenever there is time we will try to figure it out, but the multiScope module has been degraded to low priority.

+ CV Inputs per Channel:
    + **X** - X-value (horizontal component).
    + **Y** - Y-value (vertical component).
    + **C** - Color/hue (0V to +5V).
    + **A** - Alpha channel (0V to +5V).
    + **BLANK** - Blank ON or OFF. By default, Blank is off. ON is any input <=0 (really < 0.1V), otherwise it will be OFF.  
    You can use a **trigSeq** (in **CONT** mode, synchronized with a **voltSeq**) to control / hide lines that you do not wish to be shown. �
	+ **FC** - Fill Color hue (0V to +5V)
    + **FA** - Fill alpha channel (0V to +5V).
    + **R** - Rotation (-10V to +10V). Will either be a rotational rate or if the **ABS** button is on, it will be the absolute angular position.
    + **T** - Time.
    + **TH** - Line Thickness.
  
+ User Controls per Channel:
    + **X** - Offset (OFF) & Scale (SCL) knobs.
    + **Y** - Offset (OFF) & Scale (SCL) knobs.
    + **LNK** - (Toggle) Link the X-scale and Y-scale knobs together so they will change together (have the same value).
    + **C** - Color knob. If an input is active on the Color port, this is ignored. Highest setting will yield White now.
    + **A** - Alpha channel knob. If an input is active on the Alpha port, this is ignored.
    + **FC** - Fill Color knob. If an input is active on the Fill Color port, this is ignored. Highest setting yield give White now.
    + **Fill Color** - (Toggle) Fill on/off.
    + **FA** - Fill alpha channel knob. If an input is active on the Alpha port, this is ignored.
    + _Rotation Controls_:
        + **R** - Rotation knob. If an input is active on the Rotation port, this is ignored.
        + **ABS** - (Toggle) Turning ABS on will make the rotation inputs control the absolute angular position instead of a rate.		
    + **T** - Time adjustment knob. Will be used along with the Time input port.
    + **TH** - Line Thickness. If an input is active on the Thickenss port, this is ignored.
	+ **EFFECT** - Effect knob.
    + **X*Y** - (Toggle) Toggle lissajous mode on / off (default is on).

+ User Controls for entire module:
    + **INFO** - (Toggle) Toggle input parameter information on / off (default is on). Located on the right-hand-side (RHS) bar.
	+ **BG COLOR** - (Toggle) Toggle on-screen Background Color picker on / off (default is on). Located on the right-hand-side (RHS) bar.
	+ **Background Color Picker** - Displayed on screen. Hue-Saturation-Light (HSL) sliders to pick the background color.
