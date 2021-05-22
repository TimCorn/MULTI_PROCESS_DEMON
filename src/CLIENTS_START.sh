#!/bin/bash




xterm    -geometry 80x25+0+0       -e  ./SClient localhost  12345 CLIENT_FILES/infile_1.txt  &

xterm    -geometry 80x25+950+0     -e  ./SClient localhost  12345 CLIENT_FILES/GCC.tgz  &

xterm    -geometry 80x25+0+600     -e  ./SClient localhost  12345 CLIENT_FILES/Opera_scripts.rtf  &

xterm    -geometry 80x25+950+600   -e  ./SClient localhost  12345 CLIENT_FILES/TUPLE.pdf  &

xterm    -geometry 80x25+700+400   -e  ./SClient localhost  12345 CLIENT_FILES/GNUMake-3.1.3.zip  &



