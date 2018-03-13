# The XOR
Plugins for your joy

# Klee
A rather complete Klee Sequencer

![Klee](/res/klee.png?raw=true "The Klee")

# M581
Inspired by the System 100 Sequencer

![M581](/res/m581.png?raw=true "M581")

# Renato
X/Y Sequencer, dedicated to the great Renato!

![Renato!](/res/renato.png?raw=true "Renato")

# Z8K
4 colors for 4 sequencers

![Z8K](/res/z8k.png?raw=true "Z8K")

# Spiralone
Hypnotic!

![Z8K](/res/spiralone.png?raw=true "Spiralone")

Most of these sequencers have their gate outputs dependent
from the clock pulse cycle; for this reason there is a clock generator
with controllable PWM (pwmClockGen).

0.6.0:
- Updated for VCV API 0.6
- Renato received a Reset Input
- pwmClockGen a Reset and PWM input

OSC!!!!!
For Microsoft Windows users only, there is the possibility to control the sequencers through OSC. 
To do so, you must compile the plugins with the command:

make -f makefile_osc

and compile oscServer, a background process that acts like a bridge between VCV and OSC (find id
@ https://github.com/The-XOR/oscServer).
You know that oscServer is connected because the 'DIGITAL' led in the sequencers goes lit.
oscServer is written in C# and needs at least Microsoft .NET Framerwork 4.5; in RackPlugins\digitalExt\osc\Repository
there are some Liine Lemur patches (jzml). If you want to program your own OSC patches, 
all the mappings are listed in RackPlugins\digitalExt\osc\RepositoryControl List.txt .
What else? Ah, yes, oscServer can control ALL the sequencers SIMULTANEOUSLY ;-) so you can program
(and send to me) a mega-patch able to control... everything. 
THIS FEATURE IS HIGHLY EXPERIMENTAL, I couldn't test it thoroughly and it could (and will) be buggy.
