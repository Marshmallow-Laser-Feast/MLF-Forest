# LASER FOREST
Copyright © 2017, Marshmallow Laser Feast

## How to set up the laser forest. 

First, create an image of the birds-eye view of the installation: Create a png file with a black background and white circles where the rods should be. (look at bin/data/layout for examples of roughly what this should look like). The program will scan this picture to work out the layout. Don't need to make it massive. If you change the file later on down the line, you will probably need to reassign the rods again.

Give your image a nice name and put it in bin/data/layout. Note that if the png has an alpha channel I don't think it'll work - in order to remove an alpha channel, open the image in Preview and save as png, untick the transparency option.

Open MLF-Forest.app in bin. 

Look at the UI down the left hand side, and look for the sub-heading "layout" (it should be 4th down the list under "Laser Forest".

Expand it pressing the "+" button. At the bottom of the expanded "layout" section, there should be a black box saying "image: ../../[some file name]". Click on that and select the name of your file, then press the little 's' button at the top of the "layout" section - this will save your settings.

The next step is to map the physical rods to their positions in the layout png.

Each board has a "deviceId" stored on it. The program can tell which deviceId's are on the network but needs to map them to "indexes" in the program. It makes up the index number by enumerating all the white circles it can see in the diagram. You can view the indexes or deviceId's by expanding the "display" menu and changing the "idDisplayType" dropdown.

Initially the "deviceId"s will all be wrong, the job is to make them all right.

This mapping from index to deviceId is stored in bin/data/layout/rodMap.txt. There is a "learn mode" in the program that allows you to not have to interact with this file but it's good to know how it works in case you have to fix a mistake. 

The rodMap.txt file is a list - each mapping on a new line, of deviceId to index. So each line starts with a deviceId, followed by a colon (:), followed by the index.

e.g.

127:4

means deviceId 127 corresponds to index 4 in the diagram.

There should be an entry on the list for each rod.

## LEARN MODE

If you scroll right to the bottom of the UI, there's a section called "comms". If you expand it, you'll see there is a checkbox called "learnMode"

WARNING: all rods need to be vertical for this to work.

The idea is, in learn mode, you choose a rod with the mouse on the screen, then hit the corresponding rod in the physical space, and the program will look for the rod that seems to be wobbling the most (according to its accelerometer), and map the deviceId of that board to the currently selected rod in the gui. If there are any really wobbly rods, it could break this. 

This is how to do the mapping:

1. enable the learnMode checkbox.

2. Select a rod by clicking it in the gui. You need to click it at the bottom of the rod to properly select it (it should go yellow, and you should see a text summary of it in the bottom right hand corner of the program)

3. Go and give that rod a wallop.

4. Check that the deviceId has changed.

5. Go to number 2 and select the next rod. You can '.' and ',' keys to select next or previous rod instead of using the mouse if you like.

6. press 's' to save the configuration, and make backups of rodMap.txt along the way.

7. Disable learnMode. press 's' to save.


