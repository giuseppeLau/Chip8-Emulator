#include "chip8.h"
#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {

    chip8 chip;
    chip.init();
    const char* path = "pong2.c8";
    chip.loadGame(path);

    bool run = true;
    char input;
    while(run == true){
        /*
        cin >> input;
        if(input == 'a'){
            chip.emulate();
        } else {
            run = false;
        }
        */
        chip.emulate();
    }
    return 0;
}

