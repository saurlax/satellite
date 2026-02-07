/***************************************************************************//**
 *   @file   ADF4360_cfg.h
 *   @brief  Header file of ADF4360 Driver Configuration.
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
#ifndef __ADF4360_CFG_H__
#define __ADF4360_CFG_H__


struct ADF4360_InitialSettings ADF4360_st = 
{
    25000000,   // refIn (Hz)
    
    /* Control Latch */
    0,   // powerDownMode;
    7,   // currentSetting2;
    7,   // currentSetting1;
    3,   // outPowerLevel;
    0,   // muteTillLd;
    0,   // cpGain;
    0,   // cpThreeState;
    1,   // phaseDetectorPolarity;
    5,   // muxControl;
    0,   // corePowerLevel;
    
    /* N Counter Latch */
    0,  // divideBy2Select. Not available for ADF4360-8 and ADF4360-9
    0,  // divideBy2. Not available for ADF4360-8 and ADF4360-9
    
    /* R Counter Latch */
    0,  // lockDetectPrecision;
    0,  // antiBacklash;
};

struct ADF4360_Specifications ADF4360_part[10]=
{
    /* ADF4360-0 */
    {
        2400000000,   // vcoMinFreq (Hz)
        2725000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-1 */
    {
        2050000000,   // vcoMinFreq (Hz)
        2450000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-2 */
    {
        1850000000,   // vcoMinFreq (Hz)
        2150000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-3 */
    {
        1600000000,   // vcoMinFreq (Hz)
        1950000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-4 */
    {
        1450000000,   // vcoMinFreq (Hz)
        1750000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-5 */
    {
        1200000000,   // vcoMinFreq (Hz)
        1400000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-6 */
    {
        1050000000,   // vcoMinFreq (Hz)
        1250000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-7 */
    {
        350000000,    // vcoMinFreq (Hz)
        1800000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        16              // maxPrescalerVal (8/9 and 16/17 are available)
    },
    
    /* ADF4360-8 */
    {
        65000000,     // vcoMinFreq (Hz)
        400000000,    // vcoMaxFreq (Hz)
        400000000,    // countersMaxFreq (Hz)
        1               // maxPrescalerVal (not available)
    },
    
    /* ADF4360-9 */
    {
        65000000,     // vcoMinFreq (Hz)
        400000000,    // vcoMaxFreq (Hz)
        400000000,    // countersMaxFreq (Hz)
        1               // maxPrescalerVal (not available)
    }
};

#endif // __ADF4360_CFG_H__
