#!/bin/sh
# Shell script to build the project

(./Build.sh) && (cp bin/SideScroller /home/erenik/Dropbox/engine/SideScroller/SideScroller) && (./Valgrind.sh)


