/********************************************************************************
 * FileName:       
 * Author:         jimugy  Version: QXW-Summer-V1.x  Date: 2010-7-21
 * Description:    
 * Version:        
 * Function List:  
 *                 
 * History:        
 *     <author>   <time>    <version >   <desc>
 *      jimugy       2010-7-21            
*******************************************************************************/
#include "System.h"

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;

    RCC_GetClocksFreq(&rcc_clocks);

    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}

INT32U  OS_CPU_SysTickClkFreq (void)
{
    INT32U  freq;


    freq = BSP_CPU_ClkFreq();
    return (freq);
}


