#include <msp430.h>
#include "msp_config.h"

/*
 * Configure the pins to be used for SPI communication over USCI
 */
int msp_config_spi_pins() {
  // set SCLK on P2.7
  P2SEL |= SCLK_PIN;
  P2DIR |= SCLK_PIN;
  // set SOMI on P3.4
  P3SEL |= SOMI_PIN;
  // set SIMO on P3.3
  P3SEL |= SIMO_PIN;
  P3DIR |= SIMO_PIN;
  // set wiznet reset pin on P1.5
  P1DIR |= RST_WIZ_PIN;
  // deselect wiznet 2.0
  P2DIR |= CS_WIZ_PIN;
  P2DIR |= CS_MEM_PIN;

  return 0;
}


/*
 * Configure the USCI peripheral
 */
int msp_config_usci()
{
  // software reset enabled... USCI logic held in reset state
  UCA0CTL1 |= UCSWRST;
  // data captured on first clk edge, changed on next clk edge; msb sent and receive first; master mode; synchronous mode
  UCA0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;
  // use SMCLK as clock
  UCA0CTL1 |= UCSSEL_2;
  // divide SMCLK by 2
  UCA0BR0 |= 0x02;
  // no change to UCA0BR1 register
  UCA0BR1 = 0;
  // no change in modulation register
  UCA0MCTL = 0;
  // turn off software reset mode... release the USCI for operation
  UCA0CTL1 &= ~UCSWRST;

  // Don't enable interrupts yet: just get it working with polling

  return 0;
}


/*
 * Configure the mps430's clock
 */
int msp_config_clock() {
  // bring the VCore up to operating voltage for clock init
  if (set_v_core_level(0x01) != 0) {
      return -1;
  }
  if (set_v_core_level(0x02) != 0) {
      return -1;
  }
  if (set_v_core_level(0x03) != 0) {
      return -1;
  }
  // set DCO FLL reference to REF0
  UCSCTL3 |= SELREF_2;
  // set ACLK to REF0
  UCSCTL4 |= SELA_2;
  // disable the FLL control loop
  __bis_SR_register(SCG0);
  // set the lowest possible DCOx, MODx
  UCSCTL0 |= 0x0000;
  // select 16Mhz DCO operation
  UCSCTL1 |= DCORSEL_5;
  // set DCO multiplier for 16Mhz
  UCSCTL2 |= FLLD_0 + 487;
  // enable the FLL control loop
  __bic_SR_register(SCG0);
  // wait for DCO to settle
  __delay_cycles(500000);
  // loop until XT1, XT2, and DCO fault flags are cleared
  do {
      // clear XT2, XT1, DCO fault flags
      UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
      // clear oscillator fault flags
      SFRIFG1 &= ~OFIFG;
  } while (SFRIFG1 & OFIFG); // loop while there are oscillator fault flags

  return 0;
}


/*
 * Set the VCore level of the msp430 so that the clock can be configured properly
 * Found this code in the user guide for the msp430f5 series
 */
int set_v_core_level(unsigned int level) {

    PMMCTL0_H = PMMPW_H;                        // Open PMM registers for write
    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;// Set SVS/SVM high side new level
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;// Set SVM low side to new level
    while ((PMMIFG & SVSMLDLYIFG) == 0)
        ;                       // Wait till SVM is settled
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);              // Clear already set flags
    PMMCTL0_L = PMMCOREV0 * level;                  // Set VCore to new level
    if ((PMMIFG & SVMLIFG))
        while ((PMMIFG & SVMLVLRIFG) == 0)
            ;                       // Wait till new level reached
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;// Set SVS/SVM low side to new level
    PMMCTL0_H = 0x00;                   // Lock PMM registers for write access
    return 0;
}
