Real-time C++ engine architecture for games and interactivity
=============================================================

First scratches began with a Games in Windows course, but it all started in 2012 when me and a friend wanted to develop an RTS game. However, we didn't want to just make a game, but also an architecture around it: a game engine. After the summer of 2012 I continued using what we had started on in courses at school and developing it further in my spare time. 

During the spring of 2014 I used it in my specialization project at school where the Network solution was [re-worked a bit](http://focus.gscept.com/2014ip08/).

Later on during spring/summer of 2014 it was used again as the basis for my [thesis work](https://pure.ltu.se/portal/en/studentthesis/interaktiv-projektionskartlaggning%28bcca6075-4774-466c-9990-4f3cd91343d4%29.html) at Bosch Sensortec, and has been used since for various demonstrations of image-based interactivity.


Projects
---------------
*	[Tetris](https://github.com/erenik/tetris), an example implementation of the iconic game.

Older projects
--------------
Besides those mentioned above (SpaceRace for networking, IPM for interactive projection mapping), I played around with making a music-player, worked on a side-scroller-jumping game which never really took off, a Tetris game, and a re-work of an old school project called TIFS (The invader from space).

I just recently began branching of the old projects into own git repos and updating the CMake code so that it is easy to work on several projects simultaneously, so I might post more updates shortly.

Usage
-----
Free for educational and private purposes. Usage for commercial purposes can be negotiated. If you have anything to contribute, send me a PM.

Building
--------
- Clone this git. Clone or create a project-specific directory of code next to it.
- Run CMake with the project dir as source
	- Point out engine dir to where you checked this out.
	- In LIBS_DIR, specify directory to headers and binaries for external depdendencies. E.g. D:/libs. If you need the dependencies installed, you can try marking the INSTALL_DEPENDENCIES and CMake may try to download and unpack some of them, but you may need to compile them yourself or download binaries as needed.
	- Choose which dependencies you want. Many can be omitted as they were mostly experimental.
	- in PROJ_SPEC_DIR enter the path to the source code for your project. E.g. D:/git/engine-games/MORPG 
- You will need to download and build the relevant dependencies as well. Hit the checkbox within the CMake config to download the zips.



