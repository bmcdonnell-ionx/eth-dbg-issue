/****************************************************************//**
* @file main.cpp
* @brief    Test hardware interfaces used by SD Card on Embedded Artists'
*           LPC4088 QuickStart Board
*
* @note
* Copyright (c): IONX LLC as an unpublished work
*
* @par
* The information contained herein is confidential property of IONX LLC
* The use, copying, transfer or disclosure of such information is prohibited
* except by express written agreement.
*
*******************************************************************/

#include "mbed.h"


/*******************************************************************************
** defines, macros, typedefs, enums, etc. (non-dependent) :
********************************************************************************/

#define PHY_REG_BMCR        0x00        // Basic Mode Control Register
#define PHY_BMCR_PWR_DOWN   (1 << 11)   // 1 = General power down mode
#define DP83848C_DEF_ADR    0x0100      // Default PHY device address
#define MII_WR_TOUT         0x00050000  // MII Write timeout count
#define MIND_BUSY           0x00000001  // MII is Busy


/*******************************************************************************
** external variables :
********************************************************************************/


/*******************************************************************************
* file scope variable declarations (static) :
*******************************************************************************/


/*******************************************************************************
* file scope function prototypes (static) :
*******************************************************************************/


/*******************************************************************************
* static object initializations :
*******************************************************************************/


/*******************************************************************************
* function definitions :
*******************************************************************************/

static int phy_write(unsigned int PhyReg, unsigned short Data)
{
    unsigned int timeOut;

    LPC_EMAC->MADR = DP83848C_DEF_ADR | PhyReg;
    LPC_EMAC->MWTD = Data;

    // Wait until operation completed
    for(timeOut = 0; timeOut < MII_WR_TOUT; timeOut++)
    {
        if((LPC_EMAC->MIND & MIND_BUSY) == 0)
        {
            return 0;
        }
    }

    return -1;
}



// main() runs in its own thread in the OS
int main()
{
   int ret;

   {
      // Initialize Ethernet by instantiating the Ethernet class.
      // The Ethernet interface doesn't contain a power-down
      // method so we reimplement phy_write here (above) and
      // put the PHY in power-down here.
      Ethernet eth;

      // Per section 4.2.1 in the LAN8720 datasheet, the Auto-Negotiation Enable
      //  bit must be disabled before sending the shutdown command.
      eth.set_link(Ethernet::HalfDuplex10); // disable auto negotiation

      // (Attempt to) power down the PHY chip.
      // BUG:
      // For as-yet unknown reasons, calling this function breaks debugging
      //  *after* reset, or even after power cycle. (It doesn't break
      //  immediately, but once the micro gets past the first for loop in
      //  ethernet_init(), the debugger can't halt the micro, so single-
      //  stepping and the like are no longer possible.
      // This broken debugging issue even persists after a power cycle.
      // The only way I've found to fix it is to (1) power cycle, (2) flash
      //  erase, (3) power cycle again.
      ret = phy_write(PHY_REG_BMCR, PHY_BMCR_PWR_DOWN);
   }

   DigitalOut led1(LED1, 1); // off
   Serial serial(USBTX, USBRX, 115200);

   printf("\r\n---\r\n"
         "phy_write() returned %i.\r\n", ret);

   while (true)
   {
      led1 = !led1;
      wait_ms(500);
   }
}
