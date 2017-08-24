# LASER FOREST


How to make presets.


Start on default.xml. Do this by pressing the little "l" next to the "Laser Forest" heading in the UI and choose "default.xml" - this contains all the base settings for the laser calibration etc.

Now you can fiddle with the settings, then save by pressing the little "s" next to the "Laser Forest" heading, press "<CREATE NEW>" then give it a name - don't put a ".xml" on the name.


Video playback:

Expand the "animation" section then expand "laser" inside that. The first item is a dropdown that lets you choose a video - these need to be in openframeworks/apps/mlf/MLF-Forest/bin/data/animations/laser/

You must select the loop option in the GUI to make it loop.


Random sparkle:

To make the lasers randomly come on and off, expand the "laser" sub-section of the main GUI, and select "random" - there are settings for how they behave in this section.


Always on:

In the same laser section, there's an option called "alwaysOn" - this will turn all the lasers on all the time.