#pragma language=extended
#define  ENABLE_BIT_DEFINITIONS 
#include <ioavr.h>
#include <intrinsics.h>

#include "lasercont.h"


extern char rxbuf[]; // fixed at 0x64 in main code to allow use more more RAM by bootloader than allocated for main app

void  dospm(char spmval);

#define brxbufsize 0x30

__no_init char brxptr@0x60; // fixed below buffer to avoid buffer piss-overage
__no_init char t1val@0x61;

// ensure boot() is first routine in bootseg segment so address is fixed for calls from main code

#pragma location="BOOTSEG" // defined in linker options as 0x700-7ff
void boot(void) 
{
  
   __disable_interrupt();    
  OCR1A=UCSRA=TCCR1A=brxptr=0; // ensure laser off
 
  UCSRB= (1<<RXEN) | (1<<TXEN);
  UCSRC=6;
  UBRRL=25; // 19200 
   
  TCCR1B=0x05; // /1024 prescale 128us clk, 8s roll, 32.768ms MSB count

    do {
         brxptr=0;
      do { // wait for command
         // format: <length> <command> <parameters>.  <length> includes <length> byte
      do { // wait for byte
          if(TCNT1L); // latch timer low byte
          if (brxptr) if(TCNT1H) {brxptr=0;} // timeout
          led=0;if(TCNT1H & 2) led=1;        // flashy
         } while((UCSRA & 0x80) ==0);
 
      
      if(brxptr<brxbufsize) rxbuf[brxptr++]=UDR;  
      TCNT1=0; // reset framing timeout
      
      } while ((brxptr==0) || (brxptr!=rxbuf[0]));
      
       __watchdog_reset(); // only reset WD on good command, so will still time out if it sees stream of 250k data or other junk
       
       // write eeprom command 
   if(brxptr==0x08) // additonal qualifier, not strictly neccessary, can go if space needed
      if(rxbuf[1]=='E')  if(rxbuf[2]=='P'){ // EE write
  
   EECR = (0<<EEPM1)|(0<<EEPM0); // don't use IAR intrinsic eeprom function as it won't be in boot segment 
   EEAR=rxbuf[3];
   EEDR=rxbuf[4];
   EECR|=(1<<EEMPE);
   EECR|=(1<<EEPE);  
    }
   
   // write flash command
   
   if(brxptr==0x25)  // additonal qualifier, not strictly neccessary, can go if space needed
     if(rxbuf[1]==0xef) if (rxbuf[4]<0x07) // addr ms
      {
    
      asm("st -y,r31");// save regs on stack to keep C world happy
      asm("st -y,r30");
      asm("st -y,r27");
      asm("st -y,r26");
     asm("st -y,r1");
     asm("st -y,r0");
      asm("LDS R30,rxbuf+3");   
      asm("LDS R31,rxbuf+4");
      asm("ldi r26,low(rxbuf+5)");
      asm("ldi r27,high(rxbuf+5)");
      asm("spmloop:ld r0,X+");
      asm("ld r1,X+");
      SPMCSR=1;
      asm("SPM");
      asm("adiw r30,2");
      asm("cpi R26,low(rxbuf+5+32)");
      asm("brne spmloop");
      dospm(3);//erase
     dospm(5);//write
      asm("ld r0,y+"); // restore regs
     asm("ld r1,y+");
      asm("ld r26,y+");
      asm("ld r27,y+");
      asm("ld r30,y+");
      asm("ld r31,y+");  
                     
         } // if prog-flash cmd

    
    }

 while(1); // exit is via WDT timout
}


#pragma location="BOOTSEG"
void  dospm(char spmval)
{  
   asm("LDS R30,rxbuf+3");   
   asm("LDS R31,rxbuf+4");
   SPMCSR=spmval;
   asm("SPM");   
   
}