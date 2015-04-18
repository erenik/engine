#!/bin/sh
# Shell script to build the project

mkdir out > nul 2> nul
(cd out && cmake ../) > nul 2> nul
(cd out && make -j 4 -Wfatal-errors)

# cp bin/SpaceRace ~/Dropbox/engine/SpaceRace/SpaceRaceLinux
