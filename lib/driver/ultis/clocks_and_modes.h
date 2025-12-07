

#ifndef CLOCKS_AND_MODES_H
//#define SCG IP_SCG
//#define PCC IP_PCC
void SOSC_init_8MHz();      /* Initialize system oscillator for 8 MHz xtal */
void SPLL_init_160MHz();    /* Initialize sysclk to 160 MHz with 8 MHz SOSC */
void NormalRUNmode_80MHz(); /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
void AdcClockInit(void);

#endif
