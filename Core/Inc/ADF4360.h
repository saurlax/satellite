/***************************************************************************//**
 *   @file   ADF4360.h
 *   @brief  Header file of ADF4360 Driver.
 *   @author Dan Nechita
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: 768
*******************************************************************************/
#ifndef __ADF4360_H__
#define __ADF4360_H__

/* ADF4360 part versions */
#define ADF4360_0       0
#define ADF4360_1       1
#define ADF4360_2       2
#define ADF4360_3       3
#define ADF4360_4       4
#define ADF4360_5       5
#define ADF4360_6       6
#define ADF4360_7       7
#define ADF4360_8       8
#define ADF4360_9       9

/* ADF4360 latch control bits  */
#define ADF4360_REG_CONTROL     	    0
#define ADF4360_REG_R_COUNTER		    1
#define ADF4360_REG_N_COUNTER		    2

/* Control Latch bits */
#define ADF4360_CTRL_PRESCALE(x)	    ((0x3 & (x)) << 22)
#define ADF4360_CTRL_PWR_DWN(x)	        ((0x3 & (x)) << 20)
#define ADF4360_CTRL_CURRENT1(x)        ((0x7 & (x)) << 17)
#define ADF4360_CTRL_CURRENT2(x)        ((0x7 & (x)) << 14)
#define ADF4360_CTRL_OUT_PWR_LVL(x)     ((0x3 & (x)) << 12)
#define ADF4360_CTRL_MTLD   			(1 << 11)
#define ADF4360_CTRL_CP_GAIN   	   	    (1 << 10)
#define ADF4360_CTRL_CP_THREE_STATE 	(1 << 9)
// #define ADF4360_CTRL_PHASE_DETECT_POL	(1 << 8)
#define ADF4360_CTRL_PHASE_DETECT_POL(x) ((!!(x)) << 8)
#define ADF4360_CTRL_MUXOUT(x)     		((0x7 & (x)) << 5)
#define ADF4360_CTRL_COUNTER_RESET		(1 << 4)
#define ADF4360_CTRL_CORE_POWER(x)  	((0x3 & (x)) << 2)

/* ADF4360_CTRL_PRESCALE(x) options. */
#define ADF4360_PRESCALE_8_9		    0
#define ADF4360_PRESCALE_16_17		    1
#define ADF4360_PRESCALE_32_33		    2

/* ADF4360_CTRL_PWR_DWN(x) options. */  
#define ADF4360_PWR_NORMAL_OPERATION        0
#define ADF4360_PWR_ASYNCH_POWER_DOWN       1
#define ADF4360_PWR_SYNCH_POWER_DOWN        3

/* ADF4360_CTRL_OUT_PWR_LVL(x) options. */ 
#define ADF4360_OUT_POWER_3_5   		0
#define ADF4360_OUT_POWER_5_0           1
#define ADF4360_OUT_POWER_7_5           2
#define ADF4360_OUT_POWER_11_0		    3

/* #define ADF4360_CTRL_MUXOUT(x) options. */
#define ADF4360_MUX_THREE_STATE         0
#define ADF4350_MUX_DIGITAL_LD          1
#define ADF4350_MUX_N_DIVIDER           2
#define ADF4350_MUX_DVDD                3
#define ADF4350_MUX_R_DIVIDER           4
#define ADF4350_MUX_N_LD                5
#define ADF4350_MUX_SERIAL_DATA         6
#define ADF4350_MUX_DGND                7

/* ADF4360_CTRL_CORE_POWER(x) options. */ 
#define ADF4360_CORE_POWER_5     		0
#define ADF4360_CORE_POWER_10    		1
#define ADF4360_CORE_POWER_15    		2
#define ADF4360_CORE_POWER_20    		3

/* N Counter Latch bits */
#define ADF4360_N_CNT_DIVIDE_2_SELECT	    (1 << 23)
#define ADF4360_N_CNT_DIVIDE_2              (1 << 22)
#define ADF4360_N_CNT_CP_GAIN               (1 << 21)
#define ADF4360_N_CNT_B_COUNTER(x)		    ((0x1FFF & (x)) << 8)
#define ADF4360_N_CNT_A_COUNTER(x)		    ((0x1F & (x)) << 2) 

/* R Counter Latch bits */
#define ADF4360_R_CNT_BAND_CLK(x)       	((0x3 & (x)) << 20)
#define ADF4360_R_CNT_TEST		    		(1 << 19)
#define ADF4360_R_CNT_LD_PRECISION	    	(1 << 18)
#define ADF4360_R_CNT_ANTIBACKLASH(x)     	((0x3 & (x)) << 16)
#define ADF4360_R_CNT_REF_COUNTER(x)        ((0x3FFF & (x)) << 2) 

/* ADF4360_R_CNT_BAND_CLK(x) options. */ 
#define ADF4360_BAND_DIVIDER_1			0
#define ADF4360_BAND_DIVIDER_2			1
#define ADF4360_BAND_DIVIDER_4			2
#define ADF4360_BAND_DIVIDER_8			3

/* ADF4360 Specifications */
#define ADF4360_MAX_FREQ_PFD            8000000 // Hz

/*****************************************************************************/
/************************** Types Declarations *******************************/
/*****************************************************************************/
/**
 * struct ADF4360_Specifications - Stores the minimum or maximum values that 
 *                                 reflect the performance of a device version.
 *
 * @ vcoMinFreq: Minimum frequency that the VCO can output.
 * @ vcoMaxFreq: Maximum frequency that the VCO can output.
 * @ countersMaxFreq: The maximum frequency that can be applied to A and B 
 *                    counters.
 * @ maxPrescalerVal: The maximum value of the dual-modulus prescaler. Some 
 *                    versions of the device do not have a prescaler, therefore 
 *                    the maximum value of the prescaler is set to 1.
 */
struct ADF4360_Specifications
{
    unsigned long long vcoMinFreq;
    unsigned long long vcoMaxFreq;
    unsigned long      countersMaxFreq;
    unsigned char      maxPrescalerVal;
};

/**
 * struct ADF4360_InitialSettings - Stores the settings that will be written to
 *        the device when the "ADF4360_Init" function is called.
 *
 * @ refIn: Input Reference Frequency. Maximum value accepted 250000000 Hz.
 * @ powerDownMode: Provides programmable power-down modes. Range 0..3
 * @ currentSetting2: Charge Pump Currents. Range 0..7
 * @ currentSetting1: Charge Pump Currents. Range 0..7
 * @ outPowerLevel: Set the output power level of the VCO. Range 0..3
 * @ muteTillLd: Mute-till-lock detect bit:
 *                  0 - functions disabled;
 *                  1 - RF outputs are not switched until PLL is locked.
 * @ cpGain: Charge pump gain bit:
 *              0 - Current Settings 1 is used;
 *              1 - Current Settings 2 is used.
 * @ cpThreeState: Charge Pump Three-State:
 *                    0 - normal operation;
 *                    1 - Puts the charge pump into three-state mode.
 * @ muxControl: Allows the user to access various internal points on the chip.
 *               Range 0..7
 * @ corePowerLevel: Sets the power level in the VCO core. Range 0..3
 * @ divideBy2Select: Divide-by-2 select bit:
 *                     0 - the fundamental is used as the prescaler input;
 *                     1 - divide-by-2 output is selected as the prescaler input
 * @ divideBy2: Divide-by-2 bit: 
 *                 0 - normal operation occurs;
 *                 1 - the output divide-by-2 functions is chosen.
 * @ lockDetectPrecision: Lock detect precision bit. Sets the number of 
 *                        reference cycles with less than 15 ns phase error for 
 *                        entering the locked state:
 *                           0 -  three cycles are taken;
 *                           1 -  five cycles are taken.
 * @ antiBacklash: Sets the antibacklash pulse width. Range 0..3.
 */
struct ADF4360_InitialSettings
{
    unsigned long  refIn;
    
    /* Control Latch */
    unsigned char powerDownMode;
    unsigned char currentSetting2;
    unsigned char currentSetting1;
    unsigned char outPowerLevel;
    unsigned char muteTillLd;
    unsigned char cpGain;
    unsigned char cpThreeState;
    unsigned char phaseDetectPol;
    unsigned char muxControl;
    unsigned char corePowerLevel;
    
    /* N Counter Latch */
    unsigned char divideBy2Select; // Not available for ADF4360-8 and ADF4360-9
    unsigned char divideBy2;       // Not available for ADF4360-8 and ADF4360-9
    
    /* R Counter Latch */
    unsigned char lockDetectPrecision;
    unsigned char antiBacklash;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/*! Initialize the device. */
unsigned char ADF4360_Init(unsigned char adf4360Version);

/*! Write data into a register. */
void ADF4360_Write(unsigned long data);

/*! Powers down or powers up the device. */
void ADF4360_Power(unsigned char powerMode);

/*! Sets the ADF4360 frequency. */
unsigned long long ADF4360_SetFrequency(unsigned long long frequency);

#endif //__ADF4360_H__
