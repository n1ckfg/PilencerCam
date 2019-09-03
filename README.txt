Pilencer is an RPi camera app that sends a start command when it detects motion, and a stop command after a timeout. 
Based on: https://www.pyimagesearch.com/2015/05/25/basic-motion-detection-and-tracking-with-python-and-opencv/
...and the work of @jvcleave and @orgicus. 

Pilencer can use Bonjour (aka Zeroconf) to send OSC without configuring IP addresses.

Setup instructions:
https://learn.adafruit.com/bonjour-zeroconf-networking-for-windows-and-linux/overview

More discussion:
https://www.raspberrypi.org/forums/viewtopic.php?f=66&t=18207

TOOLS
Use PilencerViewer to test input from an RPi running Pilencer.