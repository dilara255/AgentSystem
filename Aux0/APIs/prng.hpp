#pragma once

#include "miscStdHeaders.h"

namespace AZ{
    //PERFORMANCE: on a phenon II x4 820, 2,8 Ghz, 8gb DDR2 1333Mhz ram (for large n):
    //(times to draw numbers, multiply by a float and assign to an array)
    //x86 debug: ~29 - 33 (with load) nanos per PRN
    //x64 debug: ~13,5 - 14,5 (with load) nanos per PRN
    //x86 release: ~11,5 - 12,5 (with load) nanos per PRN
    //x64 release: ~2,75 - 2,8 (with load) nanos per PRN
	void draw4spcg32s(uint64_t* s0, uint64_t* s1, uint64_t* s2, uint64_t* s3,
                      uint32_t* dest0, uint32_t* dest1, uint32_t* dest2, uint32_t* dest3){ 
        uint64_t m = 0x9b60933458e17d7d;
        uint64_t a = 0xd737232eeccdf7ed;

        *s0 = *s0 * m + a;
        *s1 = *s1 * m + a;
        *s2 = *s2 * m + a;
        *s3 = *s3 * m + a;
        
        int shift0 = 29 - (*s0 >> 61);
        int shift1 = 29 - (*s1 >> 61);
        int shift2 = 29 - (*s2 >> 61);
        int shift3 = 29 - (*s3 >> 61); 

        *dest0 = *s0 >> shift0;
        *dest1 = *s1 >> shift1;
        *dest2 = *s2 >> shift2;
        *dest3 = *s3 >> shift3;
    }


    //from https://nullprogram.com/blog/2017/09/21/
    //on test system debug x64 tends to about 16,5 nanos per prn
	uint32_t spcg32(uint64_t* s) {
        uint64_t m = 0x9b60933458e17d7d;
        uint64_t a = 0xd737232eeccdf7ed;
        *s = *s * m + a;
        int shift = 29 - (*s >> 61);
        return *s >> shift;
    }

}


