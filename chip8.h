#ifndef CHIP8_H
#define CHIP8_H

#include "stdint.h"

#define MEMORY_SIZE 0xFFF
#define STARTING_MEMORY_ADDRESS 0x200

class chip8 {

    private:
        unsigned char memory[MEMORY_SIZE], keyboard[16], display[64*32];
        uint16_t V[16], stack[16], I, VF, PC;
        uint8_t delay_timer, sound_timer, SP;
    
    public:
        chip8();
        chip8(const chip8& orig);
        virtual ~chip8();
        int loadGame(const char* path);
        void init();
        void emulate();

};

#endif /* CHIP8_H */

