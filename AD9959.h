/* ========================================================================= */
/*                                                                           */
/* ========================================================================= */

#ifndef AD9959_h
#define AD9959_h

#include <Arduino.h>

/* ------------------------------------------------------------------------- */
/*                             R E G I S T E R S                             */
/* ------------------------------------------------------------------------- */

#define Reg9959_CSR    0x00 // 1b The channel enable bits do not require 
                            //    an I/O update to enable or disable a channel
                            //    The channel enable bits are enabled or disabled 
                            //    immediately after the CSR data byte is written
#define Reg9959_FR1    0x01 // 3b
#define Reg9959_FR2    0x02 // 2b 
#define Reg9959_CFR    0x03 // 3b

#define Reg9959_CFTW0  0x04 // 4b Channel Frequency Tuning Word 32 bits
#define Reg9959_CPOW0  0x05 // 3b Channel Frequency Tuning Word 14 bits
#define Reg9959_ACR    0x06 // 3b Amplitude Control Register
#define Reg9959_LSRR   0x07 // 2b Linear Sweep Ramp Rate (LSRR) Falling 8bits / Rising 8 bits
#define Reg9959_RDW    0x08 // 4b LSR Rising Delta Word (RDW) 32 bits
#define Reg9959_FDW    0x09 // 4b LSR Falling Delta Word (FDW) 32 bits

#define Reg9959_CW1    0x0A // 4b Frequency tuning word[31:0] 
                            //    or phase word[31:18] 
                            //    or amplitude word[31:22]

// Bit Descriptions for CSR ( 1 bytes )
// --------------------------
//                                    76543210

#define CSR_MSB         0x00      // B.......0
#define CSR_LSB         0x01      // B.......1

#define CSR_Serial_2W   0x00      // B.....00.
#define CSR_Serial_3W   0x02      // B.....01.
#define CSR_Serial_2bit 0x04      // B.....10.
#define CSR_Serial_4bit 0x06      // B.....11.

#define CSR_Channel_0    0x10     // B0001....
#define CSR_Channel_1    0x20     // B0010....
#define CSR_Channel_2    0x40     // B0100....
#define CSR_Channel_3    0x80     // B1000....
#define CSR_Channel_All  0xF0     // B1111....


// Bit Descriptions for FR1 ( 3 bytes )
// --------------------------
//                                       2         1
//                                    321098765432109876543210

#define FR1_CPL_75ua    0x000000  // B......00................
#define FR1_CPL_100ua   0x010000  // B......10................
#define FR1_CPL_125ua   0x020000  // B......10................
#define FR1_CPL_150ua   0x030000  // B......11................

#define FR1_VCO_low     0x000000  // B0.......................
#define FR1_VCO_high    0x800000  // B1.......................


// Bit Descriptions for FR2 ( 2 bytes )
// --------------------------
//                                         1
//                                    5432109876543210

// Bit Descriptions for CFR ( 3 bytes )
// --------------------------
// AFP Select  (CFR[23:22])
//    CFR[14]
// 00 X Modulation disabled
// 01 0 Amplitude modulation 
// 10 0 Frequency modulation 
// 11 0 Phase modulation

// Bit Descriptions for ACR ( 3 bytes )
// --------------------------
//                                       2         1
//                                    321098765432109876543210
#define ACR_AME_DIS     0x000000  // B...........0............   
#define ACR_AME_ENA     0x001000  // B...........1............

#define ACR_RAMP_MANU   0x000000  // B............0...........
#define ACR_RAMP_AUTO   0x000800  // B............1...........

// Modulation Level (FR1[9:8]) Description 
// 00 Two-level modulation 
// 01 Four-level modulation 
// 10 Eight-level modulation 
// 11 16-level modulation

// Bit Descriptions for CFTW0
// --------------------------

// Bit Descriptions for CPOW0
// --------------------------

/* ------------------------------------------------------------------------- */

class AD9959 {
  
  public:
    AD9959(int csb, int clk, int data, int io3, int upd, int rst );

    void initialize();
    void DDSInit();

    void SetChannel  (int chanel);
    void SetFrequence(long freq);
    void SetFtw      (long ftw);    // 32 bits F = ftw . 25 MHz . prescaler / 2^32
    void SetAmplitude(long amp);    // 0..1023
    void SetPhase    (int  phase);  // 0..359
    void SetPow      (long pw);     // 0..2^14

    void SweepFrequency(long Start ,long End ,int RRate ,int FRate);
    void DDSReset();
    void Update();
    void ResetCom();
        
    void write8 (byte reg, byte data);
    void write16(byte reg, unsigned int  data); 
    void write16(byte reg, byte data0815 , byte data0007); 
    void write24(byte reg, unsigned long data);
    void write24(byte reg, byte data1623 , byte data0815 , byte data0007); 
    void write32(byte reg, unsigned long data);
    void write32(byte reg, byte data2431 , byte data1623 , byte data0815 , byte data0007); 

    void SPI_begin(); 
    void SPI_transfer(byte Data);
    void SPI_transferw(unsigned int Data);
    void SPI_end();
    
  private:
    int CSB;
    int SCLK;
    int SDATA;

    int IO3;
    int UPD;
    int RESET;
    
};

/* ------------------------------------------------------------------------- */

#endif
