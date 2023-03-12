/* ========================================================================= */
/*                                                                           */
/* ========================================================================= */


/* ------------------------------------------------------------------------- */
/*                       AD 9859                                             */
/* ------------------------------------------------------------------------- */

#include "AD9959.h"

//            GND   GND    1
#define AD9959_P0   17 //  2  I Data pins used for modulation (FSK, PSK, ASK)
#define AD9959_P1   15 //  4  I
#define AD9959_P2   16 //  3  I
#define AD9959_P3    2 //  6  I

#define AD9959_UPD  14 //  5  I  A rising edge transfers data from the serial I/O port buffer to active registers. 
#define AD9959_CSB   3 //  7  I  Active Low Chip Select. Allows multiple devices to share a common I/O bus (SPI). 
#define AD9959_SCK   4 //  8  I  Serial Data Clock for I/O Operations. Data bits are written on the rising edge  and read on the falling edge.  

#define AD9959_IO0   5 //  9 I/O Data Pin SDIO_0 is dedicated to the serial port I/O only. 
#define AD9959_IO1   6 // 10 I/O can be used for the serial I/O port or used to initiate a ramp-up/ramp-down (RU/RD) of the DAC output amplitude.
#define AD9959_IO2   7 // 11 I/O can be used for the serial I/O port or used to initiate a ramp-up/ramp-down (RU/RD) of the DAC output amplitude.
#define AD9959_IO3   8 // 12 I/O In single-bit or 2-bit modes, SDIO_3 is used for SYNC_I/O

#define AD9959_PCD   9 // 13  I  Power down ? ( 1 = power-down mode )
#define AD9959_RST  10 // 14  I  MASTER_RESET  Active High Reset Pin. Asserting the pin forces the internal registers to their default state
//             NC   nc // 15  
//             yclk nc // 16  O  Sync Clk
//             yin  nc // 17  I  Sync In
//             yout nc // 18  O  Sync Out
//            GND   GND   19
//             NC   nc // 20


AD9959 myDDS(AD9959_CSB, AD9959_SCK, AD9959_IO0, AD9959_IO3, AD9959_UPD, AD9959_RST);

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#include <UTFT.h>

extern uint8_t SmallFont[];
extern uint8_t BigFont[];

UTFT tft(ILI9486,38,39,40,41);

// ----- Colors in RGB565 Format ----

#define BLACK   0x0000
#define GRAY    0x632C
#define LGRAY   0xB614
#define WHITE   0xFFFF

#define NAVY    0x000F
#define BLUE    0x001F
#define TEAL    0x03EF
#define CYAN    0x07FF

#define CYAN    0x07FF
#define MAROON  0x7800
#define RED     0xF800

#define DGREEN  0x03E0
#define GREEN   0x07E0

#define YELLOW  0xFFE0
#define ORANGE  0xFC00
#define PURPLE  0x780F
#define LPURPLE 0xC018

#define MAGENTA 0xF81F

/* ------------------------------------------------------------------------- */

void setTextColor(int ForeGround, int BackGround) {
  tft.setColor(ForeGround);
  tft.setBackColor(BackGround);  
}

void Set_Text_Size(int Sz) {
  switch ( Sz) {
    case 0 : tft.setFont(SmallFont); 
             break;
    case 1 : tft.setFont(SmallFont); 
             break;            
    case 2 : tft.setFont(BigFont);
             break;
    case 3 : tft.setFont(BigFont); 
             break;
  }
}

/* ------------------------------------------------------------------------- */
/*                              Encodeur et Boutons                          */
/* ------------------------------------------------------------------------- */

#define encoder0PinA    20 
#define encoder0PinB    21
#define BtnEncoder0     A9

int posencoder0        = 0;
int encoder0mov        = 0;
unsigned long ntime0   = 0;
unsigned long otime0   = 0;

#define encoder1PinA    19 
#define encoder1PinB    18
#define BtnEncoder1    A10

int posencoder1        = 0;
int encoder1mov        = 0;
unsigned long ntime1   = 0;
unsigned long otime1   = 0;

#define BTN_None         0
#define BTN_Ch0          1
#define BTN_Ch0_Long     2
#define BTN_Ch1          3
#define BTN_Ch1_Long     4

#define BTN_LONGDELAY  800

byte keydown  = BTN_None;
byte key      = BTN_None;
unsigned long   BTNTime;

/* ------------------------------------------------------------------------- */

void doEncoder0() {
  noInterrupts();
  ntime0=millis();
  if ( ntime0-otime0 > 60 ) {
    if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB)) {
      encoder0mov=-1;
      posencoder0--;
    } else {
      encoder0mov=+1;
      posencoder0++;
    }
    otime0=ntime0;
  }
  interrupts();
}

/* ------------------------------------------------------------------------- */

void doEncoder1() {
  noInterrupts();
  ntime1=millis();
  if ( ntime1-otime1 > 60 ) {
    if (digitalRead(encoder1PinA) == digitalRead(encoder1PinB)) {
      encoder1mov =-1;
      posencoder1--;
    } else {
      encoder1mov =+1;
      posencoder1++;
    }
    otime1=ntime1;
  }
  interrupts();
}

/* ------------------------------------------------------------------------- */

void readBtnState() {
  unsigned long NTime; 
  NTime=millis();
  
  if ( keydown == BTN_None ) { // no key waiting 
    if ( digitalRead (BtnEncoder0)==LOW ) { BTNTime=NTime;  keydown=BTN_Ch0; }
    if ( digitalRead (BtnEncoder1)==LOW ) { BTNTime=NTime;  keydown=BTN_Ch1; }
 
  } else {                     // key allready down
    if ( NTime - BTNTime > 10 ) { // avoid rebounds
        switch (keydown) {
           case BTN_Ch0:
                 if ( digitalRead (BtnEncoder0)==HIGH ) { // keypress on release ;)   
                   if ( NTime - BTNTime >= BTN_LONGDELAY )  key = BTN_Ch0_Long;                
                   else                                     key = BTN_Ch0;
                   keydown=BTN_None;
                 }
                 break;
           case BTN_Ch1:
                 if ( digitalRead (BtnEncoder1)==HIGH ) { // keypress on release ;)   
                   if ( NTime - BTNTime >= BTN_LONGDELAY )  key = BTN_Ch1_Long;                
                   else                                     key = BTN_Ch1;
                   keydown=BTN_None;
                 }
                 break;
        }       
    }
  }
}

/* ------------------------------------------------------------------------- */

bool keypressed() {
  return ( key != BTN_None );
}

byte readkey() {
  byte tmp = key;
  key=BTN_None;
  return( tmp);
}

/* ------------------------------------------------------------------------- */
/*                            MENU et AFFICHAGE                              */
/* ------------------------------------------------------------------------- */

boolean ChanelOn [4]  = { false, false, false,  false };
long    Frequency[5]  = {  1000,  2000,  3000,   4000, 440 };  //    1 - 200 000 000 Hz
int     Amplitude[4]  = {    50,    50,    50,     50 };       //    0 - 100 %
int     Phase[4]      = {     0,     0,     0,      0 };       //    0 - 359 °
int     Harmonic[4]   = {     1,     1,     1,      1 };       //  -62 -  64  - Divide , + Multiply 
boolean HarmonicOn[4] = { false, false, false,  false };
int     ColorCurve[4] = { YELLOW, CYAN,   RED,   BLUE }; 

/* ------------------------------------------------------------------------- */

#define posHarmY 10

/* ------------------------------------------------------------------------- */

void DisplayBtn(int x0,int y0,int x1,int y1, boolean Select ) {
  if (Select ) tft.setColor(RED);
  else         tft.setColor(BLUE);
  
  tft.drawRoundRect(x0,y0,x1,y1);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void DisplayOneChanel(int no, boolean Select ) {
  int yy=30+no*30;  
  
  Set_Text_Size(3);
  if ( ChanelOn[no] ) {    
    tft.setColor(ORANGE);
    tft.fillRoundRect(1,yy+1,29,yy+28); 

    tft.setColor(BLUE);
    tft.setBackColor(ORANGE); 
    tft.printNumI(no, 7, yy+5 , 1, ' ');
    tft.setBackColor(BLACK);

    if (Select ) tft.setColor(GREEN);
    else         tft.setColor(BLUE);
    tft.drawRoundRect(0,yy,30,yy+29);        
  } else {
    tft.setColor(BLACK);
    tft.fillRoundRect(1,yy+1,29,yy+28); 

    tft.setColor(ORANGE);
    tft.printNumI(no, 7, yy+7 , 1, ' ');
    DisplayBtn(0,yy,30,yy+29, Select);
  } 
}

/* ------------------------------------------------------------------------- */
/*                          F R E Q U E N C Y                                */
/* ------------------------------------------------------------------------- */

void DisplayFrequency(int x,int y, unsigned long Freq ,int dg=0 ) {
  int Cbase;
  char Str1[2] = {'0', '\0'};
  unsigned char  Digit0 = Freq % 10; Freq = Freq / 10;
  unsigned char  Digit1 = Freq % 10; Freq = Freq / 10;
  unsigned char  Digit2 = Freq % 10; Freq = Freq / 10;
  unsigned char  Digit3 = Freq % 10; Freq = Freq / 10;
  unsigned char  Digit4 = Freq % 10; Freq = Freq / 10;
  unsigned char  Digit5 = Freq % 10; Freq = Freq / 10;
  unsigned char  Digit6 = Freq % 10; Freq = Freq / 10;  
  unsigned char  Digit7 = Freq % 10; Freq = Freq / 10;
  unsigned char  Digit8 = Freq % 10; Freq = Freq / 10;
  
  if ( dg==0 )        Cbase=YELLOW;
  else if ( dg==-1 )  Cbase=RED;
       else           Cbase=GREEN;
       
  Set_Text_Size(3);
  if ( dg== 8 ) setTextColor(RED  ,BLACK);
  else          setTextColor(Cbase,BLACK);        
  Str1[0]='0'+Digit8;  tft.print(Str1,x    ,y); 
  Str1[0]='0'+Digit7;  tft.print(Str1,x+ 18,y); 
    
  if ( dg== 7 ) setTextColor(RED  ,BLACK);
  else          setTextColor(Cbase,BLACK); 
  Str1[0]='0'+Digit6;  tft.print(Str1,x+ 36,y); 
//  tft.print("."       ,x+ 46,y); 
          
  if ( dg== 6 ) setTextColor(RED  ,BLACK);
  else          setTextColor(Cbase,BLACK); 
  Str1[0]='0'+Digit5;  tft.print(Str1,x+ 60,y); 
          
  if ( dg== 5 ) setTextColor(RED  ,BLACK);
  else          setTextColor(Cbase,BLACK); 
  Str1[0]='0'+Digit4;  tft.print(Str1,x+ 78,y); 
          
  if ( dg== 4 ) setTextColor(RED  ,BLACK);
  else          setTextColor(Cbase,BLACK); 
  Str1[0]='0'+Digit3;  tft.print(Str1,x+ 96,y); 
//  tft.print("."       ,x+ 110,y); 
          
  if ( dg== 3 ) setTextColor(RED  ,BLACK);
  else          setTextColor(Cbase,BLACK); 
  Str1[0]='0'+Digit2;  tft.print(Str1,x+ 120,y); 
          
  if ( dg== 2 ) setTextColor(RED  ,BLACK);
  else          setTextColor(Cbase,BLACK); 
  Str1[0]='0'+Digit1;  tft.print(Str1,x+ 138,y); 
          
  if ( dg== 1 ) setTextColor(RED  ,BLACK);
  else          setTextColor(Cbase,BLACK); 
  Str1[0]='0'+Digit0;  tft.print(Str1,x+ 156,y); 

  Set_Text_Size(2);
  setTextColor(BLUE  ,BLACK);
  tft.print("Hz" ,x+ 176,y+4); 
}

/* ------------------------------------------------------------------------- */

void DisplayOneFrequency(int no, boolean Select, int digit=0 ) {
  int yy=30+no*30;  

  if ( no<4) {
    DisplayBtn(31,yy  ,252,yy+29, Select);
    if ( digit==0 && HarmonicOn[no] ) DisplayFrequency(38,yy+7, CalHFreq(no) , digit );
    else                              DisplayFrequency(38,yy+7, Frequency[no], digit );  
  } else {   
    DisplayFrequency(38,posHarmY, Frequency[4], digit ) ;
  }
}

/* ------------------------------------------------------------------------- */

void EditFrequency(int line ) {
  boolean editmode = true;
  boolean change=true;
  long pas = 1;

  while ( editmode ) {
    readBtnState();  
    if (encoder1mov != 0 ) {
      noInterrupts();
      switch (pas) {
        case 1: Frequency[line]=Frequency[line]+encoder1mov*       1; break;
        case 2: Frequency[line]=Frequency[line]+encoder1mov*      10; break;
        case 3: Frequency[line]=Frequency[line]+encoder1mov*     100; break;
        case 4: Frequency[line]=Frequency[line]+encoder1mov*    1000; break;
        case 5: Frequency[line]=Frequency[line]+encoder1mov*   10000; break;
        case 6: Frequency[line]=Frequency[line]+encoder1mov*  100000; break;
        case 7: Frequency[line]=Frequency[line]+encoder1mov* 1000000; break;
        case 8: Frequency[line]=Frequency[line]+encoder1mov*10000000; break;                   
      }
      encoder1mov=0;
      change = true;
      interrupts();
    }
    if (encoder0mov != 0 ) {
      noInterrupts();
      pas=pas-encoder0mov;
      encoder0mov=0;
      change = true;
      interrupts();
    }
    if (pas<1) pas=1;
    if (pas>8) pas=8;

    if ( line < 4 ) {
      if (Frequency[line]<        0) Frequency[line]=        0;
      if (Frequency[line]>200000000) Frequency[line]=200000000;   
    } else {
      if (Frequency[line]<       64) Frequency[line]=       64;
      if (Frequency[line]>  3125000) Frequency[line]=  3125000;   
    }
    
    if ( change )  {
      DisplayOneFrequency(line, true ,pas);
      change=false; 
    }
    char tmpk=readkey();
    if ( tmpk == BTN_Ch0 || tmpk == BTN_Ch1 ) editmode=false;   
  }
}

/* ------------------------------------------------------------------------- */
/*                         A M P L I T U D E                                 */
/* ------------------------------------------------------------------------- */

void DisplayOneAmplitude(int no, boolean Select , boolean Edit=false ) {
  int yy=30+no*30; 
  char TempStr[6];
  
  DisplayBtn(253,yy,329,yy+29, Select);
    
  Set_Text_Size(3);
  if ( Edit )  setTextColor(GREEN  ,BLACK);
  else         setTextColor(YELLOW ,BLACK);
  
  sprintf(TempStr, "%3d",Amplitude[no]);
  tft.print(TempStr ,  260,yy+7);
  
  Set_Text_Size(2);
  setTextColor(BLUE  ,BLACK);
  tft.print("%" , 313,yy+10); 
}

/* ------------------------------------------------------------------------- */

void EditAmplitude(int line ) {
  boolean editmode = true;
  boolean change=true;

  while ( editmode ) {
    readBtnState();  
    if (encoder1mov != 0 ) {
      noInterrupts();
      Amplitude[line]=Amplitude[line]+encoder1mov;
      encoder1mov=0;
      change = true;
      interrupts();
    }
     if (encoder0mov != 0 ) {
      noInterrupts();
      Amplitude[line]=Amplitude[line]+encoder0mov*10;
      encoder0mov=0;
      change = true;
      interrupts();
    }   
    if ( Amplitude[line]<  0) Amplitude[line]=  0;
    if ( Amplitude[line]>100) Amplitude[line]=100;    
    
    if ( change )  {
      DisplayOneAmplitude(line, true ,true);
      change=false; 
    }
    char tmpk=readkey();
    if ( tmpk == BTN_Ch0 || tmpk == BTN_Ch1 ) editmode=false;   
  }
}

/* ------------------------------------------------------------------------- */
/*                              P H A S E                                    */
/* ------------------------------------------------------------------------- */

void DisplayOnePhase(int no, boolean Select , boolean Edit=false ) {
  int yy=30+no*30;  
  char TempStr[6];

  DisplayBtn(330,yy,405,yy+29, Select);

  Set_Text_Size(3);
  if ( Edit )  setTextColor(GREEN  ,BLACK);
  else         setTextColor(YELLOW ,BLACK);
  
  sprintf(TempStr, "%3d",Phase[no]);
  tft.print(TempStr,  337,yy+7);       
  Set_Text_Size(2);
  setTextColor(BLUE  ,BLACK);
  tft.print("o" ,  388,yy+1); 
}

/* ------------------------------------------------------------------------- */

void EditPhase(int line ) {
  boolean editmode = true;
  boolean change=true;

  while ( editmode ) {
    readBtnState();  
    if (encoder1mov != 0 ) {
      noInterrupts();
      Phase[line]=Phase[line]+encoder1mov;
      encoder1mov=0;
      change = true;
      interrupts();
    }
     if (encoder0mov != 0 ) {
      noInterrupts();
      Phase[line]=Phase[line]+encoder0mov*10;
      encoder0mov=0;
      change = true;
      interrupts();
    }       
    if ( Phase[line]<  0) Phase[line]=359;
    if ( Phase[line]>359) Phase[line]=  0;    
    
    if ( change )  {
      DisplayOnePhase(line, true ,true);
      change=false; 
    }
    char tmpk=readkey();
    if ( tmpk == BTN_Ch0 || tmpk == BTN_Ch1 ) editmode=false;   
  }
}

/* ------------------------------------------------------------------------- */
/*                            H A R M O N I C                                */
/* ------------------------------------------------------------------------- */

long CalHFreq(int no) {
  long result;
  if ( Harmonic[no] >0 ) result= Frequency[4] * Harmonic[no];
  else                   result= Frequency[4] / ( 2-Harmonic[no] ) +0.5; 
  return result;
}

/* ------------------------------------------------------------------------- */

void DisplayOneHarmonic(int no, boolean Select, boolean Edit=false ) {
  int yy=30+no*30;  
  char TempStr[6];
  
  Set_Text_Size(2);
  if ( HarmonicOn[no] ) {    
    tft.setColor(ORANGE);
    tft.fillRoundRect(407,yy+1,478,yy+28);
   
    if (Harmonic[no]>0) sprintf(TempStr, "%4d",Harmonic[no]);
    else                sprintf(TempStr, "%4d",Harmonic[no]-2);

    if ( Edit ) tft.setColor(RED);
    else        tft.setColor(BLUE);
    tft.setBackColor(ORANGE);
    tft.print(TempStr,  420,yy+9); 
    tft.setBackColor(BLACK);

    if (Select ) tft.setColor(GREEN);
    else         tft.setColor(BLUE);
    tft.drawRoundRect(406,yy,479,yy+29);       
  } else {

    tft.setColor(BLACK);
    tft.fillRoundRect(407,yy+1,478,yy+28); 

    if ( Edit ) tft.setColor(GREEN);
    else        tft.setColor(YELLOW);
    if (Harmonic[no]>0) sprintf(TempStr, "%4d",Harmonic[no]);
    else                sprintf(TempStr, "%4d",Harmonic[no]-2);
    tft.print(TempStr,  410,yy+7);   
    DisplayBtn(406,yy,479,yy+29, Select);
  }
}

/* ------------------------------------------------------------------------- */

void EditHarmonic(int line ) {
  boolean editmode = true;
  boolean change=true;

  while ( editmode ) {
    readBtnState();  
    if (encoder1mov != 0 ) {
      noInterrupts();
      Harmonic[line]=Harmonic[line]+encoder1mov;
      encoder1mov=0;
      change = true;
      interrupts();
    }
    
    if ( Harmonic[line]<-62 ) Harmonic[line]=-62;
    if ( Harmonic[line]> 64 ) Harmonic[line]= 64;    
    
    if ( change )  {
      DisplayOneHarmonic(line, true ,true);
 //     DisplayOneFrequency(line,false,0);
      DisplayFrequency(38,30+line*30+5, CalHFreq(line) , 0 );
      change=false; 
    }
    char tmpk=readkey();
    if ( tmpk == BTN_Ch0 || tmpk == BTN_Ch1 ) editmode=false;   
  }
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void DisplayOneCell(int line, int raw ,boolean Select, boolean Edit = false) {

  switch (raw) { 
    case 0 :  DisplayOneChanel   (line, Select );
              break;
    case 1 :  DisplayOneFrequency(line, Select );
              break;
    case 2 :  DisplayOneAmplitude(line, Select );
              break;
    case 3 :  DisplayOnePhase    (line, Select );
              break;
    case 4 :  DisplayOneHarmonic (line, Select );
              break;
  }            
}

/* ------------------------------------------------------------------------- */

void DisplayLine(int line, int sraw, int scol) {
  for (int i=0; i<5 ; i++ ) {
    DisplayOneCell(line,i, sraw == line && scol==i ,false);
  }
}

/* ------------------------------------------------------------------------- */
/*                            --- S E T U P ---                              */
/* ------------------------------------------------------------------------- */

void setup() {

  //  Boutons 
  // ----------
  pinMode(encoder0PinA, INPUT_PULLUP);
  pinMode(encoder0PinB, INPUT_PULLUP);
  pinMode(BtnEncoder0 , INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoder0, CHANGE); 

  pinMode(encoder1PinA, INPUT_PULLUP);
  pinMode(encoder1PinB, INPUT_PULLUP);
  pinMode(BtnEncoder1 , INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoder1PinA), doEncoder1, CHANGE); 
  
  //    tft
  // -----------
  tft.InitLCD();
  tft.fillScr(BLACK); 
  tft.setFont(SmallFont);
  
//  tft.Set_Rotation(1); 
//  tft.Set_Text_Mode(0);
  
  tft.setColor(RED);
  tft.setBackColor(BLACK);

  //  DDS AD 9958
  // -------------
  pinMode(AD9959_PCD ,OUTPUT);
  pinMode(AD9959_P0  ,OUTPUT);
  pinMode(AD9959_P1  ,OUTPUT);
  pinMode(AD9959_P2  ,OUTPUT);
  pinMode(AD9959_P3  ,OUTPUT);
  pinMode(AD9959_IO1 ,INPUT);
  pinMode(AD9959_IO2 ,INPUT);
  digitalWrite(AD9959_PCD,0);
  digitalWrite(AD9959_P0 ,0);
  digitalWrite(AD9959_P1 ,0);
  digitalWrite(AD9959_P2 ,0);
  digitalWrite(AD9959_P3 ,0);
  delay(50);
  myDDS.initialize();
  delay(10);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void UpdateFrequency() {
  for (int i=0;i<4;i++){
    myDDS.ResetCom();
    myDDS.SetChannel(i);
    if ( ChanelOn[i] ) {
      if ( HarmonicOn[i] ) {
        if (Harmonic[i]>0 ) {
          myDDS.SetFrequence(Frequency[4]*Harmonic[i]);
        } else {
          myDDS.SetFrequence(Frequency[4]/(-1*(Harmonic[i]-2)));
        }
      } else {
        myDDS.SetFrequence(Frequency[i]);
      }
      myDDS.SetAmplitude( (int) (Amplitude[i]*10.23) );
      myDDS.SetPhase(Phase[i]);
    } else {
      myDDS.SetFrequence(0);
      myDDS.SetAmplitude(0);
    }   
    myDDS.Update();
    delayMicroseconds(10);      
  }
}

/* ------------------------------------------------------------------------- */
/*                     ---   M A I N   L O O P   ---                         */
/* ------------------------------------------------------------------------- */

boolean change    = true;
boolean changePos = true;

int Rsel = 0;
int Csel = 0;
int oRsel = 0;
int oCsel = 0;

void RefreshPos() {
  DisplayOneCell(oRsel,oCsel, false, false);
  DisplayOneCell( Rsel, Csel,  true, false);
  oRsel = Rsel;
  oCsel = Csel;
  changePos=false;
}

void RefreshScreen() {
  Set_Text_Size(1);
  setTextColor(LPURPLE  ,BLACK);
  tft.print("Ch"        ,    6, 19); 
  tft.print("Amplitude" ,  258, 19); 
  tft.print("Phase"     ,  350, 19); 
  tft.print("Harmonic"  ,  412, 19); 
  change=false;
  
  for (int i=0;i<4;i++) {
    DisplayLine(i,Rsel,Csel);
  }
  Set_Text_Size(3);
  DisplayFrequency(38,posHarmY, Frequency[4] ,-1 );

  tft.setColor(BLACK);
  tft.fillRect(1,152,478,318);
  
  tft.setColor(LGRAY);
  tft.drawRect(0,151,479,319);
  tft.drawLine(0,235,479,235);

  float step;
  float angle;
  float y0;
  float y1;

  // Reference
  tft.setColor(GRAY);
  step=2*3.141592654/( 480.0/3.0 );    // reference step for one pixel at RefFrequency
  angle= 3.141592654/180.0;            // start angle value
  y0   =sin(angle)*80*1.0;
  for (int i=1 ; i<479 ; i++ ) {
    angle+=step;
    y1=sin(angle)*80*1.0;
    tft.drawLine(i-1,235-y0,i,235-y1);
    y0=y1;
  }

  for (int j=0 ; j<4 ; j++ ) {
    if ( ChanelOn[j] ) {
      tft.setColor(ColorCurve[j] );
      if ( HarmonicOn[j] ) step=2*3.141592654/( 480.0/3.0 )*CalHFreq(j) /Frequency[4];
      else                 step=2*3.141592654/( 480.0/3.0 )*Frequency[j]/Frequency[4];
      angle= 3.141592654/180.0 * Phase[j]; // start angle value
      y0   =sin(angle)*80*Amplitude[j]/100;
      for (int i=1 ; i<479 ; i++ ) {
        angle+=step;
        y1=sin(angle)*80*Amplitude[j]/100;
        tft.drawLine(i-1,235-y0,i,235-y1);
        y0=y1;
      }    
    }
  }
}

/* ------------------------------------------------------------------------- */

boolean frequencychange = false;

void loop() {

  if ( frequencychange ) {
    UpdateFrequency();  
    frequencychange=false;
  }
  
  if ( changePos ) {
    RefreshPos();
  } 
  if ( change ) {
    RefreshScreen();
  } 
    
  readBtnState();
  if (encoder0mov != 0 ) {
    noInterrupts();
    Rsel=Rsel+encoder0mov;
    encoder0mov=0;
    changePos = true;
    interrupts();
  }
    
  if (encoder1mov != 0 ) {
    noInterrupts();
    Csel=Csel+encoder1mov;
    encoder1mov=0;
    changePos = true;
    interrupts();
  }

  if ( Csel>4 ) {
    Csel=0;
    if ( Rsel<3) Rsel++;
    else Rsel=0;
  }
  if ( Csel<0 ) {
    Csel=4;
    if ( Rsel>0) Rsel--;
    else Rsel=4;
  }
  if ( Rsel>3 ) {
    Rsel=0;
    if ( Csel<4 ) Csel++;
    else Csel=0;
  }  
  if ( Rsel<0 ) {
    Rsel=3;
    if ( Csel>0 ) Csel--;
    else Csel=4;
  }

  char tmpk=readkey();
  if ( tmpk == BTN_Ch0 ) {
    switch ( Csel ) {
      case 0 : ChanelOn[Rsel] = not ( ChanelOn[Rsel] );
               frequencychange=true;
               break; 
      case 1 : EditFrequency(Rsel);
               frequencychange=true;
               break;      
      case 2 : EditAmplitude(Rsel);
               frequencychange=true;
               break;      
      case 3 : EditPhase(Rsel);
               frequencychange=true;
               break;         
      case 4 : EditHarmonic(Rsel);
               frequencychange=true;
               break;      
    }
    change = true;
  }
  if ( tmpk == BTN_Ch0_Long ) {
    switch ( Csel ) {
      case 1 : EditFrequency(4);
               frequencychange=true;
               break;          
    }
    change = true;
  }
  if ( tmpk == BTN_Ch1 ) {
    switch ( Csel ) {
      case 0 : ChanelOn[Rsel] = not ( ChanelOn[Rsel] );
               frequencychange=true;
               break;        
      case 4 : HarmonicOn[Rsel] = not ( HarmonicOn[Rsel] );
               frequencychange=true;
               break;      
    }
    change = true;
  } 
}
