/*
utils_samd51/clock_utils.h

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2019

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the squidworks and ponyo
projects. Copyright is retained and must be preserved. The work is provided as
is; no warranty is provided, and users accept all liability.
*/

#include "clock_utils.h"
#include "../indicators.h"

/*
// I used to have this singleton stuff here, but I think
// since I am using the extern... I have no need for it, 
D51ClockUtils* D51ClockUtils::instance = 0;

D51ClockUtils* D51ClockUtils::getInstance(void){
    if(instance == 0){
        instance = new D51ClockUtils();
    }
    return instance;
}

D51ClockUtils* D51ClockUtils = D51ClockUtils::getInstance();
*/

D51ClockUtils* d51ClockUtils;

D51ClockUtils::D51ClockUtils(){}

void D51ClockUtils::setup_16mhz_xtal(void){
    if(mhz_xtal_is_setup) return; // already done, 
    // let's make a clock w/ that xtal:
    OSCCTRL->XOSCCTRL[0].bit.RUNSTDBY = 0;
    OSCCTRL->XOSCCTRL[0].bit.XTALEN = 1;
    // set oscillator current..
    OSCCTRL->XOSCCTRL[0].reg |= OSCCTRL_XOSCCTRL_IMULT(4) | OSCCTRL_XOSCCTRL_IPTAT(3);
    OSCCTRL->XOSCCTRL[0].reg |= OSCCTRL_XOSCCTRL_STARTUP(5);
    OSCCTRL->XOSCCTRL[0].bit.ENALC = 1;
    OSCCTRL->XOSCCTRL[0].bit.ENABLE = 1;
    // make the peripheral clock available on this ch 
    GCLK->GENCTRL[MHZ_XTAL_GCLK_NUM].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_XOSC0) | GCLK_GENCTRL_GENEN;  // GCLK_GENCTRL_SRC_DFLL
    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL(MHZ_XTAL_GCLK_NUM)){
        //DEBUG2PIN_TOGGLE;
    };
    mhz_xtal_is_setup = true;
}

void D51ClockUtils::start_ticker_a(uint32_t us){
    //now using 120mHz main clock (gen(0)) instead of xtal, 
    //setup_16mhz_xtal();
    // ok
    TC0->COUNT32.CTRLA.bit.ENABLE = 0;
    TC1->COUNT32.CTRLA.bit.ENABLE = 0;
    // unmask clocks
    MCLK->APBAMASK.reg |= MCLK_APBAMASK_TC0 | MCLK_APBAMASK_TC1;
    // ok, clock to these channels...
    GCLK->PCHCTRL[TC0_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(0);//this->mhz_xtal_gclk_num);
    GCLK->PCHCTRL[TC1_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(0);//this->mhz_xtal_gclk_num);
    // turn them ooon...
    TC0->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV2 | TC_CTRLA_CAPTEN0;
    TC1->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV2 | TC_CTRLA_CAPTEN0;
    // going to set this up to count at some time, we will tune
    // that freq. with
    TC0->COUNT32.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
    TC1->COUNT32.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
    // allow interrupt to trigger on this event (overflow)
    TC0->COUNT32.INTENSET.bit.MC0 = 1;
    TC0->COUNT32.INTENSET.bit.MC1 = 1;
    // set the period,
    while (TC0->COUNT32.SYNCBUSY.bit.CC0);
    // 8 counts in here per us
    // nothing > 100kHz, ok? 
    if(us < 10) us = 10;
    // 120 / 2 -> 60 ticks per us, 
    TC0->COUNT32.CC[0].reg = 60 * us;
    // enable, sync for enable write
    while (TC0->COUNT32.SYNCBUSY.bit.ENABLE);
    TC0->COUNT32.CTRLA.bit.ENABLE = 1;
    while (TC0->COUNT32.SYNCBUSY.bit.ENABLE);
    TC1->COUNT32.CTRLA.bit.ENABLE = 1;
    // enable the IRQ
    NVIC_EnableIRQ(TC0_IRQn);
    NVIC_SetPriority(TC0_IRQn, 2);
}

void D51ClockUtils::set_ticker_a_priority(uint32_t prio){
    if(prio > 3) prio = 3;
    NVIC_SetPriority(TC0_IRQn, prio);
}

void D51ClockUtils::start_ticker_b(uint32_t us){
    setup_16mhz_xtal();
    // ok
    TC2->COUNT32.CTRLA.bit.ENABLE = 0;
    TC3->COUNT32.CTRLA.bit.ENABLE = 0;
    // unmask clocks
    MCLK->APBBMASK.reg |= MCLK_APBBMASK_TC2 | MCLK_APBBMASK_TC3;
    // ok, clock to these channels...
    GCLK->PCHCTRL[TC2_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(this->mhz_xtal_gclk_num);
    GCLK->PCHCTRL[TC3_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(this->mhz_xtal_gclk_num);
    // turn them ooon...
    TC2->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV2 | TC_CTRLA_CAPTEN0;
    TC3->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV2 | TC_CTRLA_CAPTEN0;
    // going to set this up to count at some time, we will tune
    // that freq. with
    TC2->COUNT32.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
    TC3->COUNT32.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
    // allow interrupt to trigger on this event (overflow)
    TC2->COUNT32.INTENSET.bit.MC0 = 1;
    TC2->COUNT32.INTENSET.bit.MC1 = 1;
    // set the period,
    while (TC2->COUNT32.SYNCBUSY.bit.CC0);
    // 8 counts in here per us
    // nothing > 1MHz, ok? 
    if(us < 8) us = 8;
    TC2->COUNT32.CC[0].reg = 8 * us;
    // enable, sync for enable write
    while (TC2->COUNT32.SYNCBUSY.bit.ENABLE);
    TC2->COUNT32.CTRLA.bit.ENABLE = 1;
    while (TC2->COUNT32.SYNCBUSY.bit.ENABLE);
    TC3->COUNT32.CTRLA.bit.ENABLE = 1;
    // enable the IRQ
    NVIC_EnableIRQ(TC2_IRQn);
}