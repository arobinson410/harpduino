<h1>Harpduino</h1>
<hr />
<p>This repository contains code and datasheets for Mercer Unversity's ECE 428 class of Spring 2019. The Harpduino code is modified from the Trackduino base. The changes mades to the code were nescessary due to the usage of an Arduino MEGA as opposed to the UNO. The revised code is available for download in this repository.</p>
<p><h3>To use this code you need:</h3>
<ul>
  <li>An Arduino MEGA</li>
  <li>HX1 - VHF Narrow Band FM APRS Transmitter (144.39Mhz - NA)</li>
  <li>SenMod 5PCS Micro SD Card Micro SDHC Mini TF Card Adapter Reader Module</li>
  <li>4-Pin Common Annode RGB LED</li>
  <li>3 - 10k Ohm Resistors </li>
  <li>Adafruit Ultimate GPS Breakout Board</li>
  <li>Jumper Wires</li>
  <li>2 - 9.6 in Wire Segments</li>
</ul>
</p>
<p><h3>Wiring is as follows:</h3>
  <ul>
    <li>GPS RX  -> Arduino TX1</li>
    <li>GPS TX  -> Arduino RX1</li>
    <li>HX1 EN  -> Arduino D4</li>
    <li>LED B   -> Arduino D6</li>
    <li>LED R   -> Arduino D7</li>
    <li>LED G>  -> Arduino D8</li>
    <li>HX1 TXD -> Arduino D9</li>
    <li>SD MISO -> Arduino D50</li>
    <li>SD MOSI -> Arduino D51</li>
    <li>SD SCK  -> Arduino D52</li>
    <li>SD CS   -> Arduino D53</li>
  </ul>
</p>
  
    
