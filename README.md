# Arrow

A simple game made with [libnds](https://github.com/blocksds/libnds) from the [BlocksDS](https://github.com/blocksds) toolchain and [NightFoxLib](https://github.com/knightfox75/nds_nflib) for the Nintendo DS(i).

## Generating the NDS file
- Set up the BlocksDS environment and install NightFoxLib.
- Start from the root directory and run `./assets/convert.sh` to convert all assets to a DS-friendly file format.
- Run `make` to generate the executable.

## Running the NDS file
- Any common emulator such as [MelonDS](https://melonds.kuribo64.net/) or [DeSmuME](http://desmume.org/) can run the file as is.
- For running on actual hardware, the system must be [modded](https://dsi.cfw.guide/). In that case, simply place the file on the SD card and the application should show up on the menu.