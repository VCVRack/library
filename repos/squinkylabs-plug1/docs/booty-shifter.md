# Booty Shifter frequency shifter
Booty Shifter is a frequency shifter inspired by the Moog/Bode frequency shifter module.

![boooty shifter image](./booty-shifter.png)

The name "Booty Shifter" is a nod to the original analog module, as well as to a black cat named Booty.

Booty Shifter  will take an audio input and shift its frequency up or down. This is not like a pitch shift where harmonics will remain in tune, it is an absolute frequency shift in Hz, so in general harmonics will go way out of tune.
## Common uses
Feed in music and shift the frequency a good amount.

Feed in speech or radio and shift it.

Shift drums up or down a little bit to re-tune them without the usual pitch shifting artifacts.

Small shifts in conjunction with delays can make a chorus-like effect to thicken music.
## Inputs and outputs
IN is the audio input.

CV is the pitch shift control voltage. -5V will give minimum shift, +5 will give maximum.

DN is the down-shifted output.

UP is the up-shifted output.
## Controls
RANGE sets the total shift range in Hz. For example, the 50 hz. Setting means that the minimum shift is 50 Hz down, and the maximum is 50 hz up.

Range value Exp is different, here minimum shift is 2 hz, maximum is 2 kHz, with an exponential response.

Shift AMT is added to the control voltage, with a range or -5..5.
## Oddities and limitations
If you shift the frequency up too far, it will alias. There is no anti-aliasing, so if the highest input frequency + shift amount > sample_rate / 2, you will get aliasing. The Bode original of course did not alias.

If you shift the input down a lot, frequencies will go below zero and wrap around. Taken far enough this will completely reverse the spectrum of the input. This was a prized feature of the Bode original.

As you shift the input down, you may start to generate a lot of subsonic energy. High Pass filter may clean this up.

The down shift frequency fold-over, while true to the original, does cause problems when trying to pitch drum tracks down a lot. Carefully high pass filtering the input before it is down-shifted can control this.


