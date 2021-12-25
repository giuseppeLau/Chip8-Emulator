#define _CRT_SECURE_NO_DEPRECATE
#include "chip8.h"
#include "fcntl.h"
#include <cstdlib>
#include "time.h"
#include <iostream>

using namespace std;

chip8::chip8() {
}

chip8::chip8(const chip8& orig) {
}

chip8::~chip8() {
}

void chip8::init(){
    
    for(int i = 0; i < MEMORY_SIZE; i++) memory[i] = 0;
    for(int i = 0; i < 16; i++) V[i] = 0;
    for(int i = 0; i < 16; i++) stack[i] = 0;
    
    srand(time(NULL));
    I = 0x0;
    PC = 0x200;
    SP = 0;
    delay_timer = 0;
    sound_timer = 0;
}

int chip8::loadGame(const char* path) {

    FILE* ROM = fopen(path, "rb");
    if(ROM == NULL) {
        cout << "Errore nell'apertura della Rom." << endl;
        return -1;
    }
    
    fseek(ROM, 0, SEEK_END);
    long fsize = ftell(ROM);
    rewind(ROM);
    
    //char* buffer = (char*)malloc(sizeof(char) * fsize);
    char* buffer = new char[sizeof(char) * fsize];
    
    size_t result = fread (buffer, 1, fsize, ROM);
    if (result != fsize) {
	cout << "Errore nella lettura della Rom." << endl; 
	return -1;
    }
    
    for(int i = 0; i < fsize; i++) { 
        memory[i+STARTING_MEMORY_ADDRESS] = buffer[i]; 
    }
    
    fclose(ROM);
    free(buffer);
    cout << "Rom aperta con successo." << endl; 
    return 1;
}

void chip8::emulate(){

    uint16_t opcode = (memory[PC] << 8) | memory[PC+1];                 
                                                               
    switch(opcode & 0xF000){                                      
                                                                
        case 0x0000: {     
            
            switch(opcode & 0x000F){ 
                
                case 0x0000: {  // Opcode 00E0 Resetta il display
                    PC+=2;
                    cout << "Opcode 0x00E0: Display Reset" << endl;
                    break;
                }

                case 0x000E: { // Opcode 00EE Ritorna da una subroutine 
                    SP--;
                    PC = stack[SP];
                    PC += 2;
                    cout << "Opcode 0x00EE: Returned to " << PC << endl;
                    break;
                }

                default: {
                    cout << "Opcode non supportato" << endl;
                    exit(0);
                    break;
		        }
            }
            break;
        }

        case 0x1000: { // Opcode 1nnn Jump to location nnn
            uint16_t nnn = opcode & 0x0FFF;
            PC = opcode & 0x0FFF;
            cout << "Opcode 0x1000: Jumped to location " << nnn << endl;
            break;
        }

        case 0x2000: { // Opcode 2nnn Call subroutine at nnn
            uint16_t nnn = opcode & 0x0FFF;
            SP++;
            stack[SP] = PC;
            PC = opcode & 0x0FFF;
            cout << "Opcode 0x2000: called from " << nnn << endl;
            break;
        }
        
        case 0x3000: { // Opcode 3xkk Skip next instruction if Vx == kk
            uint16_t x = (opcode >> 8) & 0xF;
            uint16_t kk = opcode & 0x00FF;
            cout << "Opcode 0x3xkk: ";
            if(V[(opcode >> 8) & 0xF] == opcode & 0x00FF) {
                PC += 4;
                cout << "Register " << x << " == " << kk << ": Next instruction will be skipped" << endl;
            } else { 
                PC += 2; 
                cout << "Register " << x << " != " << kk << ": Next instruction will not be skipped" << endl;
            }
            break;
        }
        
        case 0x4000: { // Opcode 4xkk Skip next instruction if Vx != kk
            uint16_t x = (opcode >> 8) & 0xF;
            uint16_t kk = opcode & 0x00FF;
            cout << "Opcode 0x4xkk: ";
            if(V[(opcode >> 8) & 0xF]!= opcode & 0x00FF) { 
                PC += 4;
                cout << "Register " << x << " != " << kk << ": Next instruction will be skipped" << endl;
            } else {
                PC += 2; 
                cout << "Register " << x << " == " << kk << ": Next instruction will not be skipped" << endl;
            }
            break;
        }  
        
        case 0x5000: { // Opcode 5xy0 Skip next instruction if Vx == Vy
            uint16_t x = (opcode >> 8) & 0xF;
            uint16_t y = (opcode >> 4) & 0xFF;
            cout << "Opcode 0x5xy0: ";
            if(V[(opcode >> 8) & 0xF] == V[(opcode >> 4) & 0xFF]) {
                PC += 4;
                cout << "Register " << x << " == " << "Register " << y << ": Next instruction will be skipped" << endl;
            } else {
                PC += 2;
                cout << "Register " << x << " != " << "Register " << y << ": Next instruction will not be skipped" << endl;
            }
            break;
        }
        
        case 0x6000: { // Opcode 6xkk Set Vx == kk 
            uint16_t x = (opcode >> 8) & 0xF;
            uint16_t kk = opcode & 0xFF;
            V[(opcode >> 8) & 0xF] = (opcode & 0xFF);
            PC += 2;
            cout << "Opcode 0x6xkk: " << kk << " copied into register " << x << endl;
            break;
        }
        
        case 0x7000: { // Opcode 7xkk Set Vx += kk
            uint16_t x = (opcode >> 8) & 0xF;
            uint16_t kk = opcode & 0xFF;
            V[(opcode >> 8) & 0xF] += (opcode & 0xFF);
            PC += 2;
            cout << "Opcode 0x7xkk: " << kk << " added to register " << x << endl;
            break;
        }
        
        case 0x8000: {
            
            switch(opcode & 0x000F) {
          
                case 0x0000: { // Opcode 8xy0 Set Vx == Vy
                    uint16_t x = (opcode >> 8) & 0xF;
                    uint16_t y = (opcode >> 4) & 0xF;
                    V[(opcode >> 8) & 0xF] = V[(opcode >> 4) & 0xF];
                    PC += 2;
                    cout << "Opcode 0x8xy0: Register " << x << " set == to " << y << endl;
                    break;
                }
                
                case 0x0001: { // Opcode 8xy1 Set Vx == Vx OR Vy
                    uint16_t x = (opcode >> 8) & 0xF;
                    uint16_t y = (opcode >> 4) & 0xF;
                    V[(opcode >> 8) & 0xF] |= V[(opcode >> 4) & 0xF];
                    PC += 2;
                    cout << "Opcode 0x8xy1: Register " << x << " set |= to " << y << endl;
                    break;
                }
                
                case 0x0002: { // Opcode 8xy2 Set Vx == Vx AND Vy
                    uint16_t x = (opcode >> 8) & 0xF;
                    uint16_t y = (opcode >> 4) & 0xF;
                    V[(opcode >> 8) & 0xF] &= V[(opcode >> 4) & 0xF];
                    PC += 2;
                    cout << "Opcode 0x8xy2: Register " << x << " set &= to " << y << endl;
                    break;
                }
                
                case 0x0003: { // Opcode 8xy3 Set Vx == Vx XOR Vy
                    uint16_t x = (opcode >> 8) & 0xF;
                    uint16_t y = (opcode >> 4) & 0xF;
                    V[(opcode >> 8) & 0xF] ^= V[(opcode >> 4) & 0xF];
                    cout << "Opcode 0x8xy3: Register " << x << " set ^= to " << y << endl;
                    PC += 2;
                    break;
                }
                
                case 0x0004: { // Opcode 8xy4 Set Vx == Vx + Vy. Se il risultato > 255 setta V[15] a 1, se no, 0.
                    uint16_t x = (opcode >> 8) & 0xF; 
                    uint16_t y = (opcode >> 4) & 0xF;
                    uint16_t last8bits = (V[(opcode >> 8) & 0xF] += V[(opcode >> 4) & 0xF]) & 0xFF;
                    V[(opcode >> 8) & 0xF] += V[(opcode >> 4) & 0xF];
                    cout << "Opcode 0x8xy4: Register " << x << " set to " << x << " + " << y << endl;
                    if(last8bits > 255) {
                        V[15] = 1;
                        cout << "Result bigger than 255. Register VF set to 1." << endl;
                    } else {
                        V[15] = 0;
                        cout << "Result less than 255. Register VF set to 0." << endl;
                    }
                    PC+=2;
                    break;
                }
                
                case 0x0005: { // Opcode 8xy5 Set Vx == Vx - Vy. Se V[x] > V[y] setta V[15] a 1, se no, 0.
                    uint16_t x = (opcode >> 8) & 0xF; 
                    uint16_t y = (opcode >> 4) & 0xF;
                    if(V[(opcode >> 8) & 0xF] > V[(opcode >> 4) & 0xF]) {
                        V[15] = 1;
                        cout << "Register " << x << " bigger than register " << y << ". Register VF set to 1." << endl;
                    } else {
                        V[15] = 0;
                        cout << "Register " << x << " smaller than register " << y << ". Register VF set to 1." << endl;
                    }
                    V[(opcode >> 8) & 0xF] -= V[(opcode >> 4) & 0xF];
                    cout << "Opcode 0x8xy5: Register " << x << " set to " << x << " - " << y << endl;
                    PC+=2;
                    break;
                }
                
                case 0x0006: { // Opcode 8xy6: Se il LSB di V[x] == 1, V[15] = 1, se no = 0. Poi fai V[x]/2
                    uint16_t x = (opcode >> 8) & 0xF;
                    V[15] = V[(opcode >> 8) & 0xF] & 0x1;
                    uint16_t temp = V[15];
                    uint16_t LSB = temp & 1;
                    if(LSB == 1) {
                        cout << "Register's " << x << " LSB is 1. Register VF set to 1." << endl;
                    } else {
                        cout << "Register's " << x << " LSB is 0. Register VF set to 1." << endl;
                    }
                    V[(opcode >> 8) & 0xF] >>= 1;
                    cout << "Opcode 0x8xy6: V[" << x << "] divided by 2." << endl;
                    PC += 2;
                    break;
                }
                
                case 0x0007: { // Opcode 8xy7 Set Vy == Vy - Vx. Se V[y] > V[x] setta V[15] a 1, se no, 0.
                    uint16_t x = (opcode >> 8) & 0xF; 
                    uint16_t y = (opcode >> 4) & 0xF;
                    if(V[(opcode >> 8) & 0xF] < V[(opcode >> 4) & 0xF]) {
                        V[15] = 1;
                        cout << "Register " << y << " bigger than register " << x << ". Register VF set to 1." << endl;
                    } else {
                        V[15] = 0;
                        cout << "Register " << y << " smaller than register " << x << ". Register VF set to 1." << endl;
                    }
                    V[(opcode >> 8) & 0xF] = V[(opcode >> 4) & 0xF] - V[(opcode >> 8) & 0xF];
                    cout << "Opcode 0x8xy7: Register " << x << " set to " << y << " - " << x << endl;
                    PC+=2;
                    break;
                }
                
                case 0x000E: { // Opcode 8xyE: Se il MSB di V[x] == 1, V[15] = 1, se no = 0. Poi fai V[x]*2
                    uint16_t x = (opcode >> 8) & 0xF;
                    V[15] = V[(opcode & 0x0F00) >> 8] >> 7;
                    uint16_t temp = V[15];
                    uint16_t MSB = temp & 1;
                    if(MSB == 1) {
                        cout << "Register's " << x << " MSB is 1. Register VF set to 1." << endl;
                    } else {
                        cout << "Register's " << x << " MSB is 0. Register VF set to 1." << endl;
                    }
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    cout << "Opcode 0x8xy6: V[" << x << "] multiplied by 2." << endl;
                    PC += 2;
                    break;
                }
                
                default: {
                    cout << "Opcode non supportato" << endl;
                    exit(0);
                    break;
                }
            }
            break;
        }
        
        case 0x9000: { // Opcode 9xy0: Skip next instruction if Vx == kk
            if(V[(opcode >> 8) & 0xF] != V[(opcode >> 4) & 0xFF]) {
                PC += 4;
                cout << "Opcode 0x9xy0: Skip next instruction if Vx == kk, instruction skipped." << endl;
            }
            else {
                PC += 2;
                cout << "Opcode 0x9xy0: Skip next instruction if Vx == kk. Instruction not skipped." << endl;
            }
            break;
        }
        
        case 0xA000: { // Opcode Annn: Set I to nnn
            uint16_t i = opcode & 0xFFF;
            V[I] = opcode & 0xFFF;
            PC+=2;
            cout << "Opcode 0xAnnn: Register I set to: " << i << endl;
            break;
        }
        
        case 0xB000: { // Opcode Bnnn: Jump to address V[0] plus nnn.
            PC = V[0] + (opcode & 0xFFF);
            cout << "Opcode 0xBnnn: Jump to address V[0] plus nnn." << endl;
            break;
        }
        
        case 0xC000: { // Opcode Cxkk set to a random number AND nn.
            uint16_t x = (opcode >> 8) & 0xF;
            uint16_t nn = (opcode & 0xFF);
            V[(opcode >> 8) & 0xF] = (rand()% 256) & (opcode & 0xFF);
            PC += 2;
            cout << "Opcode 0xCxkk: V[ " << x << "] set to" << (rand()%256) << " AND " << nn << endl;
            break;
        }
        
        case 0xD000: {
            cout << "0xD000: Opcode non ancora implementato" << endl;

            uint16_t x = (opcode & 0x0F00) >> 8;
            uint16_t y = (opcode & 0x00F0) >> 4;
            uint16_t n = opcode & 0x000F;

            PC+=2;
            break;
        }
          
        case 0xE000: {
            
            switch(opcode & 0x00FF) {
				
            case 0x009E: {

                uint16_t X = ((opcode & 0x0F00) >> 8);
                if (keyboard[V[X]] == 1) {
                    PC += 4;
                }
                else {
                    PC += 2;
                }
                cout << "Opcode 0x009E: if keyboard[" << keyboard[V[X]] << "] is pressed, next instruction will be skipped" << endl;
                break;
            }
		    case 0x00A1: {

                uint16_t X = ((opcode & 0x0F00) >> 8);
                if (keyboard[V[X]] == 0) {
                    PC += 4;
                }
                else {
                    PC += 2;
                }
                cout << "Opcode 0x00A1: if keyboard[" << keyboard[V[X]] << "] is not pressed, next instruction will be skipped" << endl;
                break;
            }
		    default:
                    cout << "Opcode non supportato" << endl;
                    exit(0);
                    break;
		    }
            break;
        }
          
        case 0xF000: {
        
            switch(opcode & 0xFF){
                
                case 0x0007: {
                    uint16_t X = ((opcode & 0x0F00) >> 8);
                    V[X] = delay_timer;
                    cout << "Opcode 0x0007: the value of V[" << X << "] is set to the value of delay_timer" << endl;
                    PC += 2;
                    break;
                }
                
                case 0x000A: {

                    int hasBeenPressed = 0;
                    uint16_t X = ((opcode & 0x0F00) >> 8);
                    int i = 0;

                    cout << "Opcode 0x000A: waiting for a key to be pressed" << endl;

                    while (hasBeenPressed != 1) {

                        if (keyboard[i] == 1) {
                            V[X] = i;
                            hasBeenPressed = 1;
                        }
                        else {
                            if (i < 16) {
                                i++;
                            } else {
                                i = 0;
                            }
                        }
                    }
                    cout << "Opcode 0x000A: a key has been pressed, finished waiting" << endl;
                    PC += 2;
                    break;
                }

                case 0x0015: {

                    uint16_t X = (opcode & 0x0F00) >> 8;
                    delay_timer = V[X];
                    cout << "Opcode 0x0015: delay_timer = " << X << endl;
                    PC += 2;
                    break;
                }
                case 0x0018: {

                    uint16_t X = (opcode & 0x0F00) >> 8;
                    sound_timer = V[X];
                    cout << "Opcode 0x0018: sound_timer = " << X << endl;
                    PC += 2;
                    break;
                }
                case 0x001E: {

                    uint16_t X = (opcode & 0x0F00) >> 8;
                    I = I + V[X];
                    cout << "Opcode 0x001E: I = I + V[" << X << "]" << endl;
                    PC += 2;
                    break;
                }
                case 0x0029: {
                    cout << "0x0029: Opcode non ancora implementato" << endl;
                    PC += 2;
                    break;
                }
                case 0x0033: {
                    cout << "0x0033: Opcode non ancora implementato" << endl;
                    PC += 2;
                    break;
                }
                case 0x0055: {

                    uint16_t X = ((opcode & 0x0F00) >> 8);
                    for (int i = 0; i < X; i++) {
                        memory[I + i] = V[i];
                    }

                    cout << "Opcode 0x0055: copying values from V[0] to V[" << X << "]" << "to memory location " << I << endl;
                    PC += 2;
                    break;
                }
                case 0x0065: {

                    uint16_t X = ((opcode & 0x0F00) >> 8);
                    for (int i = 0; i < X; i++) {
                        V[i] = memory[I + i];
                    }

                    PC += 2;
                    cout << "Opcode 0x0065: copying values from memory location " << I << " to V[0] to V[" << X << "]" << endl;
                    break;
                }
                default: {
                    cout << "Opcode non supportato." << endl;
                    exit(0);
                    break;
                }   
            }
        }
    
        default: {
            cout << "Opcode non supportato." << endl;
            exit(0);
            break;
        }
    }
}
