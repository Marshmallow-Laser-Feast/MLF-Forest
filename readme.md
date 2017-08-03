# LASER FOREST
Copyright © 2013, Marshmallow Laser Feast


This is the source code for our LASER FOREST project
<https://vimeo.com/64652497>
<https://vimeo.com/64662736>
<https://vimeo.com/67980452> 


# Warning
The source code is released as is, warts and all, with all last minute fixes hacked together on the plane / train, or 4am the night before opening etc. 


# Dependencies
Current version built on OSX 10.9.3

Written mainly in C++ with [openFrameworks](http://openframeworks.cc)
(current version built with v0.8.2)

Uses [SuperCollider](http://supercollider.sourceforge.net/) for audio synthesis

Requires D2xx FTDI drivers for RS485 comms. v1.4.4 included in repo. Alternatively download latest drivers from <http://www.ftdichip.com/Drivers/D2XX.htm>. (they're also included in the repo)

In order for them to work, you need to disable your own FTDI drivers - you can install D2xxHelper in ftdi/installers or you can also do this manually on the command line by typing:

sudo kextunload -b com.apple.driver.AppleUSBFTDI

then to reload it, it's:

sudo kextload -b com.apple.driver.AppleUSBFTDI



# License
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.