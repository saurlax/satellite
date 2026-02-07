/***************************************************************************//**
 *   @file   ADF4360.c
 *   @brief  Implementation of ADF4360 Driver.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ADF4360.h"		// ADF4360 definitions.
#include "ADF4360_cfg.h"    // ADF4360_cfg definitions.
#include "Communication.h"	// Communication definitions.
#include "TIME.h"           // TIME definitions.

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
unsigned char ver         = 0;
unsigned char prescaleVal = 8;
unsigned long regR        = 0;
unsigned long regCtrl     = 0;
unsigned long regN        = 0;

/***************************************************************************//**
 * @brief Initialize the device.
 *
 * @param ver - AD4360 version.
 *                      Example: 0 - ADF4360-0
 *                               1 - ADF4360-1
 *                               ...
 *                               8 - ADF4360-8
 *                               9 - ADF4360-9
 *
 * @return status - Result of the initialization procedure.
 *					Example: 0x0 - SPI peripheral was not initialized.
 *				  			 0x1 - SPI peripheral is initialized.
*******************************************************************************/
unsigned char ADF4360_Init(unsigned char adf4360Version)
{
	unsigned char status = 0x0;
    
	/* Initialize SPI communication. */
    status = SPI_Init(0, 1000000, 0, 1);
    /* Initialize timer. */
    TIME_Init();
    /* Store the version of the device in use. */
    ver = adf4360Version;
    /* Initialize ADF4360 registers. */
    regR = ADF4360_R_CNT_LD_PRECISION * ADF4360_st.lockDetectPrecision | 
           ADF4360_R_CNT_ANTIBACKLASH(ADF4360_st.antiBacklash);   
    ADF4360_Write(ADF4360_REG_R_COUNTER | regR);
    regCtrl = ADF4360_CTRL_PWR_DWN(ADF4360_st.powerDownMode)|
              ADF4360_CTRL_CURRENT1(ADF4360_st.currentSetting1) |
              ADF4360_CTRL_CURRENT2(ADF4360_st.currentSetting2) |
              ADF4360_CTRL_OUT_PWR_LVL(ADF4360_st.outPowerLevel) | 
              ADF4360_CTRL_MTLD * ADF4360_st.muteTillLd | 
              ADF4360_CTRL_CP_GAIN * ADF4360_st.cpGain | 
              ADF4360_CTRL_CP_THREE_STATE * ADF4360_st.cpThreeState | 
              ADF4360_CTRL_PHASE_DETECT_POL * ADF4360_st.phaseDetectPol | 
              ADF4360_CTRL_MUXOUT(ADF4360_st.muxControl) | 
              ADF4360_CTRL_CORE_POWER(ADF4360_st.corePowerLevel);
    ADF4360_Write(ADF4360_REG_CONTROL | regCtrl);
    /* Recommended Interval Between Control Latch and N Counter Latch writes. */
    TIME_DelayMs(10);
    regN =   ADF4360_N_CNT_DIVIDE_2_SELECT * ADF4360_st.divideBy2Select |
             ADF4360_N_CNT_DIVIDE_2 * ADF4360_st.divideBy2;
    ADF4360_Write(ADF4360_REG_N_COUNTER | regN);
    
	return(status);
}

/***************************************************************************//**
 * @brief Write data into a register.
 *
 * @param data - Data value to write.
 *
 * @return None.
*******************************************************************************/
void ADF4360_Write(unsigned long data)
{
	unsigned char slaveDeviceId = 1;
    unsigned char spiWord[3]    = {0, 0, 0};
  
	spiWord[0] = ((data & 0xFF0000) >> 16);
	spiWord[1] = ((data & 0x00FF00) >> 8);
	spiWord[2] = ((data & 0x0000FF) >> 0);
	SPI_Write(slaveDeviceId, spiWord, 3);
}

/***************************************************************************//**
 * @brief Powers down or powers up the device.
 *
 * @param powerMode - Power option.
 *                    Example: 0 - powers down the device;
 *                             1 - power up the device.
 *
 * @return None.
*******************************************************************************/
void ADF4360_Power(unsigned char powerMode)
{
    if(powerMode)
    {
        ADF4360_Write(ADF4360_REG_CONTROL |
                      regCtrl | 
                      ADF4360_CTRL_PWR_DWN(ADF4360_PWR_NORMAL_OPERATION));
    }
    else
    {
        ADF4360_Write(ADF4360_REG_CONTROL |
                      regCtrl |
                      ADF4360_CTRL_PWR_DWN(ADF4360_PWR_SYNCH_POWER_DOWN));
    }
}

/***************************************************************************//**
 * @brief Increases the R counter value until the maximum frequency of PFD is
 *        greater than PFD frequency.
 *
 * @param rCounter - R counter value.
 *
 * @return rCounter - modified R counter value.
*******************************************************************************/
unsigned short ADF4360_TuneRcounter(unsigned short rCounter)
{
	unsigned long frequencyPfd = 0;	// PFD frequency
	
	do
	{
		rCounter++;
		frequencyPfd = ADF4360_st.refIn / rCounter;
	}
	while(frequencyPfd > ADF4360_MAX_FREQ_PFD);
    
    return rCounter;
}

/***************************************************************************//**
 * @brief Selects a value for Band Select Clock Divider that is used to divide 
 *        the output of the R counter until a frequency below 1 MHz is obtained.
 *
 * @param frequencyPfd - Frequency value of Phase Frequency Detector.
 *
 * @return bsc - Band Select Clock value.
*******************************************************************************/
unsigned short ADF4360_GetBandDivider(unsigned long frequencyPfd)
{
	unsigned long dividedRfreq = 0;
	unsigned char bsc          = 1;
    
    /* The R counter output is used as the clock for the band select logic and 
       should not exceed 1 MHz. */
	
    dividedRfreq = frequencyPfd;
    while((dividedRfreq > 1000000) && (bsc < 8))
	{
        bsc *= 2;
		dividedRfreq = frequencyPfd / bsc;
	}
    
    return bsc;
}

/***************************************************************************//**
 * @brief Sets the ADF4360 frequency.
 *
 * @param frequency - The desired frequency value.
 *
 * @return calculatedFrequency - The actual frequency value that was set.
*******************************************************************************/
unsigned long long ADF4360_SetFrequency(unsigned long long frequency)
{
    unsigned long long vcoFrequency        = 0; // VCO frequency
    unsigned long      frequencyPfd        = 0;	// PFD frequency
    unsigned long      freqRatio           = 0; // VCOfreq / PFDfreq
    unsigned long long calculatedFrequency = 0; // Actual VCO frequency
    unsigned short	   rCounterValue 	   = 0; // Value for R counter
    unsigned short     a                   = 0; // Value for A counter
    unsigned short     b                   = 0; // Value for B counter
    unsigned char      band                = 0; // Band Select Clock Value
    unsigned char      bandBits            = 0; // Band Select Clock Bits
    
    /* Force "frequency" parameter to fit in the Output frequency range. */
    if(frequency <= ADF4360_part[ver].vcoMaxFreq)
    {
        if(frequency >= ADF4360_part[ver].vcoMinFreq)
        {
            vcoFrequency = frequency;
        }
        else
        {
            vcoFrequency = ADF4360_part[ver].vcoMinFreq;
        }
    }
    else
    {
        vcoFrequency = ADF4360_part[ver].vcoMaxFreq;
    }
    /* If ADF4360-8 or ADF4360-9 are used. */
    if(ver > ADF4360_7)
    {
        /* Dual-modulus prescaler does not exist. */
        prescaleVal = 1;
        /* A counter does not exist or has a different purpose. */
        a = 0;
        /* Get the actual PFD frequency. */
        rCounterValue = ADF4360_TuneRcounter(rCounterValue);
        frequencyPfd = ADF4360_st.refIn / rCounterValue;
        /* Find Counter B value using VCO frequency and PFD frequency. */
        b = (unsigned short)((float)vcoFrequency / frequencyPfd + 0.5f);
    }
    else // If ADF4360-0, ADF4360-1, ... , ADF4360-7 are used. 
    {
        /* Adjust the dual-modulus prescaler value so that counters A and B will 
        be supplied by a clock that has a frequency below "countersMaxFreq". */
        while(((vcoFrequency / prescaleVal) > 
              ADF4360_part[ver].countersMaxFreq) && 
              (prescaleVal < ADF4360_part[ver].maxPrescalerVal))
        {
            prescaleVal *= 2;
        }
        do
        {
            /* Get the actual PFD frequency. */
            rCounterValue = ADF4360_TuneRcounter(rCounterValue);
            frequencyPfd = ADF4360_st.refIn / rCounterValue;
            /* Find the values for Counter A and Counter B using VCO frequency 
            and PFD frequency. */
            freqRatio = (unsigned short)((float)vcoFrequency / frequencyPfd 
                        + 0.5f);
            b = freqRatio / prescaleVal;
            a = freqRatio % prescaleVal;
        }while((a > b) && (b < 3)); // B must be greater or equal to A
    }
    /* Find the actual VCO frequency. */
    calculatedFrequency = ((b * prescaleVal) + a) * frequencyPfd;
    /* Select the value of the bits for the Band Select Clock Divider. */
    band = ADF4360_GetBandDivider(frequencyPfd);
    /* Relationshio between band value and band bits. */
    bandBits = (unsigned char)(((band - 1) * 0.42) + 0.8);
    /* Load the saved values into ADF4118 registers using Counter Reset
    Method. */
    ADF4360_Write(ADF4360_REG_R_COUNTER |          // Select R Counter Register
                  regR |                           // Write the fixed settings
                  ADF4360_R_CNT_BAND_CLK(bandBits) |
                  ADF4360_R_CNT_REF_COUNTER(rCounterValue));
    ADF4360_Write(ADF4360_REG_CONTROL |             // Select Control Register
                  regCtrl |                         // Write the fixed settings
                  ADF4360_CTRL_PRESCALE(prescaleVal / 16));
    /* Recommended Interval Between Control Latch and N Counter Latch writes. */
    TIME_DelayMs(10);
    ADF4360_Write(ADF4360_REG_N_COUNTER |           // Select N Counter Register
                  regN |                            // Write the fixed settings
                  ADF4360_N_CNT_B_COUNTER(b) | 
                  ADF4360_N_CNT_A_COUNTER(a));
    
    return calculatedFrequency;
}
