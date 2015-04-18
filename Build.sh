#!/bin/sh
# Shell script to build the project

mkdir out > nul 2> nul
(cd out && cmake ../)
(cd out && make -j 4)

# cp bin/SpaceRace ~/Dropbox/engine/SpaceRace/SpaceRaceLinux
