// Justin Shumate - MAR 2023
// There is a lot of duplicated code and the design of this could be greatly improved, although I am not sure how much time I will spend on it because it is working
// Funny I am not thinking of optimizing this when this is literally an optimization course

// Writing an instruction decoder in C
#include <stdio.h>
#include <stdlib.h> // Used for strtol function
#include <string.h> // Used for memset function

// Function to convert our bytes into 8 binary digits with a space added at the end for formatting by byte for debugging
void printbinchar(char c){
    for ( int i = 7; i >= 0; i-- ) {
        printf( "%d", ( c >> i ) & 1);
    }
    printf(" ");
}

// Returns a character version of a string representation of a binary number
// This function solely exists so I don't have to type 3 arguments every time I want a character
unsigned char btoc(char* inputString){
    return strtol(inputString, 0, 2);
}

int main(){

    // Register Encoding Table [W+REG or W+R_M]
    char *regEnc[16] = {
        "al",
        "cl",
        "dl",
        "bl",
        "ah",
        "ch",
        "dh",
        "bh",
        "ax",
        "cx",
        "dx",
        "bx",
        "sp",
        "bp",
        "si",
        "di"
    };
    // Effective Address Table
    char *effAdd[8] = {
        "bx + si",
        "bx + di",
        "bp + si",
        "bp + di",
        "si",
        "di",
        "bp",
        "bx"
    };
    // Create a data structure to store arguments so they only have to be computed as they come up, rather than on every loop iteration
    // opArgs[0] = d;
    // opArgs[1] = w;
    // opArgs[2] = mod;
    // opArgs[3] = reg;
    // opArgs[4] = r_m;
    // opArgs[5] = disp/addr lo;
    // opArgs[5] = data lo;
    // opArgs[5] = SR;
    long opArgs[8];
    memset(opArgs, 0, sizeof opArgs);
    
    // This will contain the assembly file with our binary instructions
    FILE *fptr;
    fptr = fopen("listing_0038_many_register_mov","rb");

    // Beginning of file print
    printf("bits 16\n\n");

    // Main loop that gets 1 byte at a time from assembly file and processes it
    unsigned char readByte, byteStore[6];
    memset(byteStore, 0, sizeof byteStore);

    int insByte = 1, totBytes = 1;
    while (1){        
        
        // Get the byte and increment the file pointer
        // If we are at the 'end of file', break out of the loop
        readByte = fgetc(fptr);
        if ( feof(fptr) )
            break;
        
        // Debug intermediate byte printing
        //printbinchar(readByte);
        //printf("%d ", readByte);

        // Depending on what byte of an instruction we are on, we may need to do different things
        // Instructions to decode can be from 1 to 6 bytes total
        switch (insByte){
            case 1:
                if (0) {}
                // MOV register/memory to/from register
                else if ( ( readByte & btoc("11111100") ) == btoc("10001000") ){             
                    printf("mov ");
                    
                    // At a minimum, we now know we have a 2 Byte instruction to decode
                    totBytes = 2;

                    // Store the first byte into the instruction array to refer to on later iterations
                    byteStore[0] = readByte;
                }
                // MOV immediate to register : mov ax, 12 -> (1011)(1)(000) 00001100 00000000
                else if ( ( readByte & btoc("11110000") ) == btoc("10110000") ){             
                    printf("mov ");

                    char w, wreg;
                    w    = ( readByte & btoc("00001000") ) >> 3;
                    wreg = ( readByte & btoc("00001111") );

                    // Register the immediate will be stored into
                    printf("%s, ",   regEnc[wreg]);

                    // Number of bytes depends on size of register
                    if      (w == 0)
                        totBytes = 2;
                    else if (w == 1)
                        totBytes = 3;

                    // Store the first byte into the instruction array to refer to on later iterations
                    byteStore[0] = readByte;
                }
                break;
            case 2:
                if (0) {}
                // MOV register/memory to/from register
                else if ( (byteStore[0] & btoc("11111100")) == btoc("10001000") ){
                    
                    // Arguments for this instruction
                    char d, w, mod, reg, r_m;
                    mod  = (  readByte     & btoc("11000000")) >> 6;
                    d    = (  byteStore[0] & btoc("00000010")) >> 1;
                    w    =    byteStore[0] & btoc("00000001");
                    reg  = ( (readByte     & btoc("00111000")) >> 3 );
                    r_m  = (  readByte     & btoc("00000111"));

                    // Change behavior based on mode
                    // One extra byte to scan
                    // MOV AX,[75] means load from memory address 75 into AX (76th element of the 65536K memory array)
                    // Because these are 16 byte registers, it will load both [75] and [76]
                    // NOTE: AX specifically has some shorter calls so this might not evaluate to the same assembly we expect
                    // Note special case of R/M being 110 which uses a separate lookup
                    if      (mod == 0){
                        // Special case for direct addressing with 16 bit displacement
                        if (r_m == 6){
                            totBytes = 4;

                        }
                        else{
                            // D == 0, source     is register
                            if      (d == 0){
                                printf("[%s], ", effAdd[r_m]);
                                printf("%s"    , regEnc[(w << 3) + reg]);
                            }
                            // D == 1, destination is register
                            else if (d == 1){
                                printf("%s, ",   regEnc[(w << 3) + reg]);
                                printf("[%s]",   effAdd[r_m]);
                            }
                        }
                    }
                    else if (mod == 1){
                        totBytes = 3;

                        byteStore[1] = readByte;
                    }
                    else if (mod == 2){
                        totBytes = 4;
                        
                        byteStore[1] = readByte;
                    }
                    // Register Mode
                    else if (mod == 3){
                        // totBytes does not need to be increased in this mode because we only have 2 bytes for this instruction

                        // direction bit
                        if      (d == 0){
                            printf("%s, ", regEnc[(w << 3) + r_m]);
                            printf("%s",   regEnc[(w << 3) + reg]);
                        }
                        else if (d == 1){
                            printf("%s, ", regEnc[(w << 3) + reg]);
                            printf("%s",   regEnc[(w << 3) + r_m]);
                        }
                    }
                }
                // MOV immediate to register : mov ax, 12 -> (1011)(1)(000) 00001100 00000000
                else if ( ( byteStore[0] & btoc("11110000") ) == btoc("10110000") ){
                    
                    // Check if we are ready to print depending on the total number of bytes
                    if      (totBytes == 2){
                        printf("%u", (long) readByte);
                    }
                    else if (totBytes == 3){
                        byteStore[1] = readByte;
                    }
                }
                break;
            case 3:
                if (0) {}
                // MOV register/memory to/from register
                else if ( (byteStore[0] & btoc("11111100")) == btoc("10001000") ){
                    if      (totBytes == 3){
                        char d, w, reg, r_m;
                        d    = (  byteStore[0] & btoc("00000010")) >> 1;
                        w    =    byteStore[0] & btoc("00000001");
                        reg  = ( (byteStore[1] & btoc("00111000")) >> 3 );
                        r_m  = (  byteStore[1] & btoc("00000111"));

                        long disp;
                        disp = (long) readByte;

                        char dispStr[256];
                        sprintf(dispStr, " + %ld", disp);

                        // D == 0, source     is register
                        if      (d == 0){
                            printf("[%s%s], ", effAdd[r_m], dispStr);
                            printf("%s"    , regEnc[(w << 3) + reg]);
                        }
                        // D == 1, destination is register
                        else if (d == 1){
                            printf("%s, ",   regEnc[(w << 3) + reg]);
                            printf("[%s%s]",   effAdd[r_m], dispStr);
                        }
                    }
                    else if (totBytes == 4){
                        byteStore[2] = readByte;
                    }
                }
                // MOV immediate to register : mov ax, 12 -> (1011)(1)(000) 00001100 00000000
                else if ( ( byteStore[0] & btoc("11110000") ) == btoc("10110000") ){
                    // This is the last step of this instruction
                    printf("%u", ((long) readByte << 8 ) + byteStore[1]);
                }

                break;
            case 4:
                if (0) {}
                // MOV register/memory to/from register
                else if ( (byteStore[0] & btoc("11111100")) == btoc("10001000") ){
                        char d, w, reg, r_m;
                        d    = (  byteStore[0] & btoc("00000010")) >> 1;
                        w    =    byteStore[0] & btoc("00000001");
                        reg  = ( (byteStore[1] & btoc("00111000")) >> 3 );
                        r_m  = (  byteStore[1] & btoc("00000111"));

                        long disp;
                        disp = ((long) readByte << 8 ) + byteStore[2];

                        char dispStr[256];
                        sprintf(dispStr, " + %ld", disp);

                        // D == 0, source     is register
                        if      (d == 0){
                            printf("[%s%s], ", effAdd[r_m], dispStr);
                            printf("%s"    , regEnc[(w << 3) + reg]);
                        }
                        // D == 1, destination is register
                        else if (d == 1){
                            printf("%s, ",   regEnc[(w << 3) + reg]);
                            printf("[%s%s]",   effAdd[r_m], dispStr);
                        }
                }
                break;
            case 5:
                break;
            case 6:
                break;
        }

        // Reset the byte counter/byte storage if we have reached the total number of bytes and we are finished decoding
        if (insByte == totBytes){
            insByte = 1;
            totBytes = 1;
            
            // If we store our computed arguments in opArgs, we might not even need the byteStore anymore
            memset(byteStore, 0, sizeof byteStore);
            memset(opArgs, 0, sizeof opArgs);
            
            printf("\n");
        }
        // Otherwise, increment the counter
        else
            insByte += 1;

        //printf("\n");            
    }
    fclose(fptr);
  
    return 0;
}