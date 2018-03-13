# And Module

![And Module](images/and.png)

The and module accepts two waveform inputs, executes an AND against both
waveforms, and outputs the resulting waveform.

In order to accomplish this, the voltage is multiplied by 10,000, and converted
to an integer, and then divided by 10,000 before returning the waveform.
