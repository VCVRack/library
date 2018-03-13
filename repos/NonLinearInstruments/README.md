<h1>NLNRI_VCVRackPlugins</h1>
<p>Nonlinear instruments as VCV Rack plugins<br/></p>

<table width="100%"><tr><td><h3>Luci modules and Luci8x8 example patch</h3></td></tr>
  <tr><td>
<img src="https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins/blob/master/res/Luci8x8_shot.jpg"/><br/><br/>
    <p>Luci is based on the ideas exposed by Jose M. Berenguer in his book "Luci, sin nombre y sin memoria." (2007). Please find more here: http://www.sonoscop.net/jmb/masluci/lucingles.html<br/><br/>The main idea is that each Luci cell has an internal signal rising from 0 to 1 at a speed determined by the Frequency parameter. When this internal signal reaches 1, it goes back to zero and triggers an impulse thru its four output ports. Each Luci cell also has four inputs to receive trigger impulses coming from connected nearby cells. When she receives one of this incoming triggers, Luci advances her phase in an amount determined by the Influence parameter.  So, aside from playing with different parameter values, we can connect many Luci cells in different topologies to experiment with the interesting synced behaviors arising.<br/>br/>In this release, a patch is delivered with a simple squared topology of 8 cells per side, where each cell is connected to the four closer cells siting on her north, east, south and west. Cells on the extreme of rows and columns are connected side to side, giving a kind of “infinite” or “closed” network of triggers.<br/><br/>User is suggested to modify such topology, connecting diferent quantities of cells in different arrangements. That is the reason why we did not offer a single module with a fixed configuration of cells (whose topology could then only be changed by code) and instead we supply discrete cells modules and complementary parameter adjusting, signal routing and mixing modules for you to experiment with; using the cabling features of VCV.<br/><br/>After installing the package, user is suggested to Load the <a href="https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins/blob/master/res/Luci8x8.vcv" target="_blank">Luci8x8.vcv patch found here</a><br/><br/>Please find <a href="https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins/blob/master/res/Luci8x8_notes.pdf" target="_blank"><b>here the user manual</b></a></p></td></tr>
</table>
 

<table width="100%"><tr><td width="20%"></td><td width="80%"><h3>BallisticENV module</h3></td></tr>
  <tr><td>
<img src="https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins/blob/master/res/BallisticENV_shot_1.jpg"/></td><td><p>BallisticENV implements a ballistic model, or projectile motion. The trajectory is delivered both as CV signal 0~10v and as an oscillator with symmetric parabola waveform in the +/-5v range.<br/><br/>Please find <a href="https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins/blob/master/res/BallisticENV_notes.pdf" target="_blank"><b>here the user manual</b></a></p>
    <p>please find <a href="https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins/blob/master/res/BallisticENV_examples.vcv" target="_blank">here a simple .vcv patch</a> with a couple suggested uses for BallisticENV.</p><p>( Panel .svg and logo by Alfredo Santamaría http://www.hakken.com.mx/gui.php )</p></td></tr>
  </table>
 
 <table width="100%"><tr><td width="20%"></td><td width="80%"><h3>QU4DiT module</h3></td></tr>
  <tr><td>
<img src="https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins/blob/master/res/QU4DiT_shot.jpg"/></td><td>
<p>The Quadratic iterator, also known as Logistic map (after the Verhulst's logistic equation) is a simple one-dimensional model with chaotic behavior. It has a single parameter (here labeled "Chaos").<br/><br/>
This module explores the region for parameter values above roughly 3.5, where chaos happens.<br/><br/>
There are two quadratic iterators running, one for the X output, the other for the Y output.<br/><br/>
Offset allows to desplace C parameter for the Y iterator.<br/><br/>
  cv~C input expects bipolar +/-5v signal.</p><p>( Panel .svg and logo by Alfredo Santamaría http://www.hakken.com.mx/gui.php )</p></td></tr>
  </table>
  

