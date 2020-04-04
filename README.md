## Starting with Melty Blood stage modding ##
- Use the ".p" file extractor to dump the "0000.p" file which is in MBAACC folder,
- In your MBAACC folder, go to "out" folder, cut the "bg" folder inside of it into the MBAACC folder
- Delete the now empty "out" folder and delete the "0000.p" file, or rename it to something else so the game does not read it.

[Download the binary from the releases page](https://github.com/sugozu/MBCC_StageTools/releases/latest)
### Foreground object remover ###
This tool removes grass, bamboos and flowers that are in the foreground layer of a stage.
Doing so prevents the characters/projectiles from being hidden, making the stages more fair/playable.
- Place the executable into the stage file folder (bg) and run it to toggle the layers on or off.

### Stage converter ###
This tool converts 24/32bit BMP files to melty background images and other way around.
 **Make sure the bmp image is 1184 pixels wide and 912 pixels high**

- Inside "bg" folder you have files called "bgXX.dat", copy the "bg12.dat" file into the same folder as the converter executable
- Now drag and drop any BMP files that are 24 or 32 bits in depth into the exe file and it will convert it to a .dat file that works in Melty

- You can do either:
 1. Replace an existing file with the new .dat file, so for example rename the .dat file to "bg30.dat" and replace the already existing file inside "MBAACC/bg" folder with it.
 2. Move your new .dat file into the "MBAACC/bg" folder, and edit the "bglist.txt" file to include it into the stage list. You just need to add a new entry for it at the end of the file, it's quite easy. Use other stages in the file as reference.
