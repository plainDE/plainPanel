#include "volume.h"

#include <cstdlib>
#include <iostream>
using std::string;
using std::to_string;

void volumeApplet::setVolume(int newVolume) {
    system("amixer -q -D default sset Master 60");
}
