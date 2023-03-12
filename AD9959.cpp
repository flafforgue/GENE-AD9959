/* ========================================================================= */
/*                                                                           */
/* ========================================================================= */

#include "AD9959.h"

//
//  SINGLE-TONE MODE
//
// Single-tone mode is the default mode of operation after a master reset signal
// 1) Power up the DUT and issue a master reset
// 2) Enable only one channel enable bit (Register 0x00) and disable the other channel enable bits
// 3) Using the serial I/O port, program the desired frequency tuning word (Register 0x04) 
//    and/or the phase offset word (Register 0x05) for the enabled channel
// 4) Repeat Step 2 and Step 3 for each channel
// 5) Send an I/O update signal

// Function that handles the creation and setup of instances

AD9959::AD9959(int csb, int clk, int data, int io3, int upd, int rst ) {
  CSB     = csb;
  SCLK    = clk;
  SDATA   = data;
  IO3     = io3;
  UPD     = upd;
  RESET   = rst;
}

void AD9959::initialize() {
  //define pins as outputs
  pinMode(CSB   ,OUTPUT);
  pinMode(SCLK  ,OUTPUT);
  pinMode(SDATA ,OUTPUT);

  pinMode(IO3   ,OUTPUT);
  pinMode(UPD   ,OUTPUT);
  pinMode(RESET ,OUTPUT);

  digitalWrite(IO3  ,0); 
  digitalWrite(RESET,0);  // Active High Reset Pin so set to  0
  digitalWrite(UPD  ,0);  // A rising edge transfers data from the serial I/O port buffer

  digitalWrite(SCLK ,0);  // Data bits are written on the rising edge so set to  0
  digitalWrite(CSB  ,1);  // Unselect Chip  ( Active Low Chip Select )
  digitalWrite(SDATA,0);  // 
  
  DDSReset();
  DDSInit();
}

void AD9959::DDSReset() { 
  digitalWrite(RESET,1);  // Active High Reset Pin
  delayMicroseconds(1);   // 1 us pulse
  digitalWrite(RESET,0);
  delayMicroseconds(1);   // wait reset done
}

void AD9959::DDSInit() { 
  ResetCom();
  write8 (Reg9959_CSR  ,     0xF0 );    // all channels
  write24(Reg9959_FR1  , 0xD30020 );    // VCO Gain High
                                        // Pll divider 20
                                        // pump charge 150ua
                                        // Sync disabled
                                        
  write16(Reg9959_FR2  ,   0xA000 );    // 1 = the sweep accumulator memory elements for all four channels are asynchronously cleared
                                        // 1 = the phase accumulator memory elements for all four channels are asynchronously cleared
                                        
  write24(Reg9959_CFR  , 0x000314 );    // 11 = the DAC is at the largest LSB value
                                        //  1 = the sweep accumulator memory elements are asynchronously cleared
                                        //  1 = the phase accumulator memory elements are asynchronously cleared
  Update();
}

void AD9959::Update() {
  digitalWrite(UPD ,1);   // A rising edge transfers data from the serial I/O port buffer to active registers
  delayMicroseconds(1);   // 1 us pulse
  digitalWrite(UPD ,0);
}

void AD9959::ResetCom() {
  digitalWrite(CSB ,1);   // unselect chip
  digitalWrite(IO3 ,1);   // pulse of IO3 to Reset comunication
  delayMicroseconds(1);   // 1 us pulse
  digitalWrite(IO3 ,0);
  digitalWrite(CSB ,0);   // activate chip
}

/* ------------------------------------------------------------------------- */
/*                        AD9959 Programing                                  */ 
/* ------------------------------------------------------------------------- */

void AD9959::SetChannel(int chanel) {
  byte chans[] = { B00010000, B00100000, B01000000, B10000000, B11110000 };
  write8(Reg9959_CSR, chans[chanel] );
}

void AD9959::SetFrequence(long freq) {
  double ftw;
  ftw = ( 4294967296.0 * freq ) / ( 25000000.0 * 20.0 ) ; // to be multiplied by 20
  SetFtw( (long) ( ftw + 0.5 ));
}

void AD9959::SetFtw(long ftw) {
  write32(Reg9959_CFTW0, ftw );
}

void AD9959::SetAmplitude(long amp) {
  long Reg = ACR_AME_ENA;
  write24(Reg9959_ACR,(long)( Reg | ( amp & 1023 )) ); 
}

void AD9959::SetPhase(int  phase){
  double pw = 16384.0 * phase / 360.0;
  write16(Reg9959_CPOW0, (int) ( pw + 0.5) );   
}
    
void AD9959::SetPow(long pw) {
  write16(Reg9959_CPOW0, pw );  
}

/* ------------------------------------------------------------------------- */
/*                                                                           */ 
/* ------------------------------------------------------------------------- */

void  AD9959::SweepFrequency(long Start ,long End ,int RRate ,int FRate) {
  double ftw;

  write24(Reg9959_CFR  , 0x000314 );
  
  ftw = ( 4294967296.0 * Start ) / ( 25000000.0 * 20.0 ) ; // to be multiplied by 20
  write32(Reg9959_CFTW0, ftw );

  ftw = ( 4294967296.0 * End ) / ( 25000000.0 * 20.0 ) ; // to be multiplied by 20
  write32(Reg9959_CW1, ftw ); 
}

/* ------------------------------------------------------------------------- */
/*                    Write Number of Bytes To AD9959                        */ 
/* ------------------------------------------------------------------------- */

void AD9959::write8 (byte reg, byte data) {
  SPI_begin(); 
  SPI_transfer(reg);   // The channel enable bits do not require an I/O update
  delayMicroseconds(1);
  
  SPI_transfer(data);
  SPI_end();
}

void AD9959::write16(byte reg, unsigned int  data) {
  write16(reg, (byte)((data >>  8) & 0xff) ,(byte)(data & 0xff));           
}

void AD9959::write16(byte reg, byte data0815 , byte data0007) {
  SPI_begin(); 
  SPI_transfer(reg);   // The channel enable bits do not require an I/O update
  delayMicroseconds(1);
   
  SPI_transfer(data0815);
  SPI_transfer(data0007);
  SPI_end();
}

void AD9959::write24(byte reg, unsigned long data) {
  write24(reg, (byte)((data >> 16) & 0xff) ,(byte)((data >>  8) & 0xff) ,(byte)(data & 0xff));           
}

void AD9959::write24(byte reg, byte data1623 , byte data0815 , byte data0007) {
  SPI_begin(); 
  SPI_transfer(reg);   // The channel enable bits do not require an I/O update
  delayMicroseconds(1);
    
  SPI_transfer(data1623);
  SPI_transfer(data0815);
  SPI_transfer(data0007);
  SPI_end();
}

void AD9959::write32(byte reg, unsigned long data) {
  write32(reg, (byte)((data >> 24) & 0xff) ,(byte)((data >> 16) & 0xff) ,(byte)((data >>  8) & 0xff) ,(byte)(data & 0xff));           
}

void AD9959::write32(byte reg, byte data2431 , byte data1623 , byte data0815 , byte data0007) {
  SPI_begin(); 
  SPI_transfer(reg);   // The channel enable bits do not require an I/O update
  delayMicroseconds(1);
  
  SPI_transfer(data2431);
  SPI_transfer(data1623);
  SPI_transfer(data0815);
  SPI_transfer(data0007);
  SPI_end();
}

/* ------------------------------------------------------------------------- */
/*                                 SPI Functions                             */ 
/* ------------------------------------------------------------------------- */

void AD9959::SPI_begin(){
//  digitalWrite(SCLK,  LOW);         // for safety
  digitalWrite(CSB,   LOW);           // Start New Word Loading 
//  delayMicroseconds(1);
}

void AD9959::SPI_end(){
  digitalWrite(CSB  , HIGH);          // End of Datas
  delayMicroseconds(1);
}

void AD9959::SPI_transfer(byte Data) {
  // MSB first
  for (byte i = 0; i < 8; i++) {
    if (Data & 0x80) digitalWrite(SDATA, HIGH);
    else             digitalWrite(SDATA, LOW);
    digitalWrite(SCLK, HIGH);         // Data bits are written on the rising edge of SCLK 
                                      // and read on the falling edge of SCLK.
    Data = Data << 1;                 // next bit      
    digitalWrite(SCLK, LOW);          // Clock to Low level
  }
}

void AD9959::SPI_transferw(unsigned int Data){
  for (byte i = 0; i < 16; i++) {
    if (Data & 0x8000) digitalWrite(SDATA, HIGH);
    else               digitalWrite(SDATA, LOW);
    digitalWrite(SCLK, HIGH);         // Clock on rising edge, so Set to High
    Data = Data << 1;                 // next bit      
    digitalWrite(SCLK, LOW);          // Clock to Low level
  }
}
