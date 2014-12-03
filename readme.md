INTRO

This is a small tool that you can use to view movie files augemented by an Rends Vorze
Cyclone SA under Linux. For now, it only works with the USB stick that came with your
Vorze; the specifics of the 'raw' BluetoothLE connection are unknown as of yet. The
dongle itself is a CP2104 USB->serial converter connected to a BVMCN5102 BTLE module
which uses a nRF51882 chip.

HOWTO

First of all, you'll need Mplayer. Grab it from http://mplayerhq.hu or maybe your distro has
it. You will need the -udp-master option: if you try and play a file with -udp-master
on the commmand line and it fails, please update your copy of MPlayer or download and 
compile one yourself.

You will also need the standard gcc stuff: make, gcc, ld. You'll probably have those installed
already.

COMPILING

Just run 'make' and you should end up with a 'vorzemplayer' executable

USING IT

- Turn on your Vorze
- Plug in the stick
- Start up a terminal for the rest of the commands
- Run 'load.sh'. It will ask for your sudo-password to load the driver for the
  chipset on the stick and bind it to the (non-standard) PID/VID it uses.
- Run dmesg. At the end, you should see a line like
  'usb 1-2: cp210x converter now attached to ttyUSB0'.
  Note the ttyUSB0. If it's different (eg ttyUSB1) you'll need to change the next
  command accordingly.
- Run the vorze tool with the csv file that goes with the movie you're about to watch and
  the port you just spotted. For example:
  ./vorzemplayer faphero_v1.csv /dev/ttyUSB0
- Run mplayer with the -udp-master option to play the movie:
  mplayer -udp-master faphero1.mp4
- Enjoy!



