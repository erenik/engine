#!/bin/sh
# Shell script to build the project

(cd out && cmake ../)
(cd out && make SpaceRace)

cp bin/SpaceRace ~/Dropbox/engine/SpaceRace/SpaceRaceLinux
