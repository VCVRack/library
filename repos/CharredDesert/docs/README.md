# Charred Desert Manual

## DTMF

Plays DTMF (telephone) tones as notes on the scale, starting at Octave 5, C.

![DTMF](images/dtmf.png)

Plays the following tones:

```
1 2 3 A
4 5 6 B
7 8 9 C
* 0 # D
```

### Input

* CV - triggers at 1.7, determines whether the note should be played
* V/Oct - Chooses which of the 16 notes to play

### Output

* Out - waveform

## Noise

Generates white or pink noise.

![Noise](images/noise.png)

Switchable between white and pink noise.

### Input

* CV - triggers at 1.7, determines whether the note should be played
* Switch - switches between white and pink noise

### Output

* Out - waveform

## CVSeq

"Sequences" CV values with an input clock

![CVSeq](images/cvseq.png)

Allows for 4 steps.

### Input

* CV - triggers at 1.7, acts as the clock
* Knobs - a single knob for each step, with outputs between 0 and 10

### Output

* Out - CV values ranging from 0 to 10

## Not

Switchable Not module

![Not](images/not.png)

Outputs the opposite of the input.

### Input

* In - switchable input between logical (CV) and waveform
* Switch - switches modes between logical and waveform

### Output

* Out - outputs on/off (0 or 1.7) or inverted (-5 to 5) value of the input

## Pan

Pans input between two outputs based on a waveform coming in via the `pan`
input

![Pan](images/pan.png)

### Input

* Input - waveform or CV to be panned between two outputs
* Pan - waveform (-5 to 5) where -5 sends 100% to first output, and 5 sends 100% to second output, with values in between

### Output

* Output 1 - output biased from negative pan
* Output 2 - output biased from positive pan

## Shift

Shifts input by -5 to +5, controllable by a knob

![Shift](images/shift.png)

### Input

* Input - waveform or CV to be shifted
* Switch - clipped output (-5 to 5) or natural output
* Knob - amount to shift, between -5 to +5

### Output

* Output - waveform or CV that has been shifted
