# VCV Rack plugins

![screenshot](https://github.com/Aepelzen/AepelzensModules/blob/master/images/screenshot.png)

## GateSeq

A Gate Sequencer with pattern support intended for polyrhythms. Every channel has it's own clock input and length. There is also a global clock input and an internal clock. Furthermore each channel has a probability setting that sets the probability that an active beat will be sent out.

To set the length for a channel hit the length button. It will turn red to indicate that you are now in length-mode. In this mode every channel has a red step button (or a yellow one if that step is also active) which indicates the last step in the sequence. Just press another step to change the length. To leave length-mode push the length button again. The length settings are tied to the pattern and will get copied if you copy a pattern.

To copy a pattern, select the pattern you want to copy first. Push the copy button to enable copy-mode, switch to the target pattern and hit the copy button again to paste your pattern. This will overwrite the target pattern.

The switch over the pattern input determines wheater all channel positions should be reset when switching patterns (i. e. start the pattern from the beginning). This might be useful to realign the channels when switching from a pattern that uses different lengths per channel. When inactive the pattern will just keep running.

### Pattern Merging (work in progress)

You can create new patterns out of existing ones by merging them. The workflow is similar to copying. Select the base-pattern first and push the merge button. Now select another pattern and this new pattern will get merged with the base-pattern according to the merge-mode (selected with the knob below the merge button). The base pattern defines the channel lengths (this might change in future versions). Currently the following merge modes are available:
* OR
* AND
* XOR
* NOR
* Random (choose one of the two patterns randomly for each step and channel)

## QuadSeq

A four channel sequencer (The knobs are made by bogaudio). Like GateSeq each channel has it's own clock input (the 4 inputs on the bottom left) and length. The mode parameter sets one of the following playback modes:
* Forward
* Backwad
* Alternating
* Random Neighbour
* Random

## New Module: Dice

Another sequencer. This one is built around probabilities and comes with all the usual goodies (per
channel clock, length and playback modes). All unconnected clock inputs are normalised to the first
clock input. The knobs set the probability for a step to be active. Unfortunately there is not
enough space for labels but this should be simple enough to work without. The upper knob in the gray
area sets the channel length, the lower one the playback mode (these work like in QuadSeq). The
button in the upper left corner resets the playback positions.

## Burst

A Burst generator. For every received trigger a number of triggers and an accompanying CV signal is sent out.
Repetitions and time set the number of triggers and the time between them. Acceleration shortens the time between subsequent triggers, jitter shifts them randomly. The switch at the bootom controls the output mode (trigger or gate).

The last trigger also triggers the EOC (end of cycle) output. You can connect this to the trigger input and turn up jitter and/or acceleration to get an irregular clock or chain multiple burst generators together.
The mode button selects the mode for the CV output, that can be used to modulate pitch or filter cutoff for delay style effects etc. Currently the following modes are supported:
* Up: cv increases in even steps
* Down: cv decreases in even steps
* Alternating1: output alternates between positive and negative values. The magnitude is only increased after 2 pulses (so you get 0, 1, -1, 2, -2 ...)
* Alternating2: output alternates between positive and negative values. The magnitude is increased after every pulse (so you get 0, 1, -2, 3, -4 ...)
* Randomp: positive random values
* Randomn: negative  random values
* Random

If you have ideas for additional CV-Modes or any other suggestions please let me know.

## Manifold

A wavefolder. Works best with simple input signals like sine or triangle waves. The fold and symmetry inputs work well with CV and audio signals. The output becomes pretty noisy for high frequency modulators but produces very interesting sounds at low/mid frequency ranges. There is an alternative folding algorithm that can be switched via context menu. That one does all the folding in a single pass and therefore the stages button does nothing if this mode is selected. It also responds different to the symmetry parameter, especially with a high number of folds.

Note: this module shifts the phase of the input-signal (because of the upsampling)

## Walker

A CV generator that simulates a random walk. At every step the CV output changes by either plus or minus stepsize. The decision is affected by the Symmetry parameter. At 12 o'clock both directions are equally likely, fully ccw all steps move downward, full cw all steps move upward. The Switch controls the behaviour at the range boundaries. There are 3 possible modes:
* Clip the signal at the boundary and just wait for it to eventually get back into the allowed range (depending on the symmetry parameter this might not happen or take a long time)
* Reset to zero
* Reset to random value within ± range/2 (this is also affected by the Symmetry parameter)

## Erwin

A 4-channel scale-quantiser with user-definable scales. All inputs/outputs use the same scale, however you can transpose every channel seperately by ±4 octaves (the 4 small trimpots). You can also transpose all outputs by 4 octaves and/or 12 semitonses via CV inputs.

# Building

The wavefolder uses libsamplerate for the upsampling. That shouldn't be a problem because Rack depends on libsamplerate anyway but you might have to install the header-files if you don't build Rack yourself.
