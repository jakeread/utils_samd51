/*
utils_samd51/clock_utils.h

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2019

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the squidworks and ponyo
projects. Copyright is retained and must be preserved. The work is provided as
is; no warranty is provided, and users accept all liability.
*/

#ifndef CLOCKS_D51_H_
#define CLOCKS_D51_H_

#include <Arduino.h>

#define MHZ_XTAL_GCLK_NUM 9

class D51ClockUtils {
    private:
        static D51ClockUtils* instance;
    public:
        D51ClockUtils();
        static D51ClockUtils* getInstance(void);
        // xtal
        volatile boolean mhz_xtal_is_setup = false;
        uint32_t mhz_xtal_gclk_num = 9;
        void setup_16mhz_xtal(void);
        // uses TC0 and TC1 as 32 bit TC
        // pickup TC0_Handler(void){}
        // do in handler: 
        // TC0->COUNT32.INTFLAG.bit.MC0 = 1;
        // TC0->COUNT32.INTFLAG.bit.MC1 = 1;
        // us: requested timer period 
        void start_ticker_a(uint32_t us);
        void set_ticker_a_priority(uint32_t prio);
        // uses TC2 and TC3 as 32 bit TC 
        // pickup on TC2_Handler(void){}
        void start_ticker_b(uint32_t us);
};

extern D51ClockUtils* d51ClockUtils;

#endif 