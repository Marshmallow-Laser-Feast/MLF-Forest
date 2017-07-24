#pragma language=extended
#define  ENABLE_BIT_DEFINITIONS 
#include <ioavr.h>
#include <intrinsics.h>
#include"lasercont.h"

// MLF laser controller
// Compile with IAR EWB (kickstart) AVR V6.11
// target ATTiny2313A

// EEPROM locations

__no_init __eeprom char ee_devid @0x00;   // device ID
__no_init __eeprom char ee_timeslot@0x01; // timeslot


extern void boot(void);

#define rxbufsize 0x24

__no_init char rxbuf[rxbufsize] @0x64; // place at start of RAM as bootloader uses larger buffer than defined for main app code

#define txbuf rxbuf // share buffer between tx & rx

// laser control/tipover
char tipfilt,tipval,tipthresh,lasholdtime;
char lasholdoff=0; // holdoff timer
unsigned int tippage;

// accel data (corrected for normal orientation, i.e. z=up)
char accx,accy,accz;

char my_id, my_timeslot; // RAM copy for speed & avoid int task ee reading messing with any ee writes
volatile char statusflags;  
#define status_tip 1
#define status_reset 2
#define status_badacc 4

//_________________________________________________________ I2C
//I2C

#define iicdelay() __delay_cycles(20)

// sda polarity reversed as it's a ddr register
char sendiic(char d)
{
 char i;
 scl=s0;
 for(i=0;i!=8;i++) {
    sda=d&0x80?d1:d0;
   iicdelay();
   scl=s1;
      while(scl_in==0) ;
   iicdelay();
   scl=s0;
   d<<=1;
 }
 iicdelay();
 scl=s1;
 sda=d1;
 iicdelay();
 i=sda_in?1:0; 
 scl=s0;
 return(i);
}

void iistop()
{
scl=s0;
sda=d0;
iicdelay();
scl=s1;
iicdelay();
sda=d1;
iicdelay();

}
  
char iistart(char addr)
{

  
sda_dat=0;  
scl_dat=0;
sda=d1; 
iicdelay();
scl=s1;
iicdelay();
sda=d0;
iicdelay();
scl=s0;
iicdelay();
return(sendiic(addr));
    
}


char getbyte(char ack)
{
 char i,d;

 for(i=0;i!=8;i++){  
   scl=s1;
   while(scl_in==0);
   iicdelay();
   d<<=1;
   if(sda_in) d|=1;
   scl=s0;
   iicdelay();
   }
 
 sda=ack?d0:d1; // ack
 iicdelay();
 scl=s1;
 iicdelay();
 scl=s0;
 iicdelay();
 sda=d1;
 iicdelay();
return(d);

}


char setaccreg(char reg,char data)
{
  if(iistart(0x98)) {iistop();return(1);}
     sendiic(reg);sendiic(data);iistop();
 return(0);
}
     
     
char readacc()
{
char x,y,z;
  
  x=0x40; // fault marker
  if(iistart(0x98)) iistop(); // no ack
 
  else {
  
  sendiic(0); // reg address
  iistart(0x99);
  x=getbyte(1);
  z=getbyte(1); // swap Y/Z for laser orientation. 
  y=getbyte(0); 
   iistop();
 
  // reset to restart conversion cycle (there is no way to poll INT via I2c - WTF?)
   // might as well do all initialisation so it copes with disconnects
  setaccreg(7,0x40);
  setaccreg(8,0); // max sample rate
  //setaccreg(6,0x10);  // INT pin for timing debug
  setaccreg(7,0x41); // exit standby
  
}
  __disable_interrupt(); // ensure atomic update so tx int always grabs coherent set of data 
  accx=x;  accy=y;  accz=z;
  __enable_interrupt();

  return(0);
}


//____________________________________________________________________ comms stuff

volatile char txptr=0;
volatile char txlen=0; // transmit length pending
volatile char breakcount=0; // counter for entry to bootloader via 256 breaks
volatile char flags=0; // signals to FG task

#define bootreq 0x80 // sequence of breaks received - flag FG task to enter bootloader
#define eeslotupdate 0x40 // set to request update timeslot info to eeprom

volatile char rxptr=0; // rx buffer pointer, =0 when waiting for FF

#pragma vector= TIMER1_COMPA_vect // T1 compare - start transmit in our timeslot
__interrupt void t1int(void) 
{
  txen=1; // enable 485 TX
  txptr=1;
   TIMSK&=~(1<<OCIE1A); // disable further Timer1 interrupts
  UDR=my_id; // first byte is always ID
  UCSRB=   (1<<UDRIE) | (1<<TXEN);// enable UDRE, rx ints
}

 
#pragma vector=USART0_UDRE_vect 
__interrupt void txint(void) // UART TX buffer empty int - send next byte
{
  if(txptr<txlen) 
      UDR=txbuf[txptr++];  // send next byte 
    else    
      UCSRB=  (1<<RXCIE) | (1<<TXCIE) | (1<<TXEN) | (1<<RXEN);// enable txc int to release 485 after last byte sent 
  }
 
  
#pragma vector=USART0_TX_vect // TX complete interrupt - release 485 driver after last byte finished sending
__interrupt void txcint(void)
{   txen=0; //release RS485
    txlen=0; // signal to rx that tx is finished (shared tx/rx buffer)
}


volatile char lasertimer=0;
volatile char acctimer;

#pragma vector= TIMER0_OVF0_vect // T0 compare - laser timer (laser drive from PWM0A, also timer for acc read
__interrupt void t0int(void) 
{

 acctimer--; // FG triggers when b7 set, adds interval to reset
 if(--lasertimer==0) OCR0A=0; // laser off when timer expires
}


//----------------------------------------------------------------- UART Rx interrupt

#pragma vector=USART0_RX_vect

#define rxtimeout (40*50) // usecs from FF to end oc command

__interrupt void rxint(void)
{ char itemp;
     
       
  if(UCSRA & 1<<FE) { // break received
   itemp=UDR; // get break byte
   if(++breakcount==0) flags|=bootreq; // 256 breaks - req to enter bootloader
   return; }
 
      breakcount=0;
      itemp=UDR; // received char
                   
        if(rxptr==0) { //waiting for ff start byte
         if (itemp==0xff) { 
            rxptr=1;
            TCNT1=0;OCR1A=rxtimeout; TIFR=1<<OCF1A; // setup rx timeout to ensure framing
  
         } return; }    
     
    // rxlen!=0 :  command in progress

      if(txlen) return;         // if transmit pending, ignore rx, as buffer is shared and Timer1 shared between rx timeout and tx slot timing

     if(TIFR&(1<<OCF1A)) {rxptr=0;return;} // comms timeout since last received byte

       if(rxptr<rxbufsize) rxbuf[rxptr++]=itemp; else rxptr=0; // store, or reset if too long - probably mis-framed
        if(rxptr<=rxbuf[1]) return; // got complete command?
   
       // complete command received 
       txlen=0; // assume no tx due
       
#define targetid rxbuf[0] // reuse unused buffer location to save stack space.
       
       targetid=rxbuf[3]; // copy target ID here as tx buffer fill overwrites rxbuf
  
       //..................................................... command 1
       
        if (rxptr==7) // right length
            if (rxbuf[2]==0x01) // command 01
                 if ((rxbuf[3]==my_id)||((rxbuf[3]==0)&&(rxbuf[4]==0))) // my ID or broadcast & not writing slot 
           { // ID/set timeslot  command
             
         if(rxbuf[4]) {my_timeslot=rxbuf[4]; flags|=eeslotupdate;statusflags&=~status_reset;} // write timeslot

        txbuf[1]=fwversion;
        txbuf[2]=hwversion;
        txbuf[3]=my_timeslot;
        txbuf[4]=0;  // unused parameters for now
        txbuf[5]=0;
        txlen=6;
            } // command 01
        
        //............................................................ command 2
        if(rxbuf[2]==0x02) if(rxptr==0x24) if(rxbuf[3]==0) {
          
#define ltemp rxbuf[1] // reuse          
          
          ltemp=rxbuf[0x0c+((my_id-1)>>3)]; // get byte in laser bitmap
          ltemp>>=((my_id-1)&7);// get control bit in byte to bit 0
          if(ltemp&1) {lasertimer=rxbuf[10];OCR0A=128;} else OCR0A=0; // set laser state
          
         tipfilt=rxbuf[8];
         tipthresh=rxbuf[9];
           lasholdtime=rxbuf[11];
          if(rxbuf[4]==1) { // raw acc data
          
           txbuf[1]=accx;
           txbuf[2]=accy;
           txbuf[3]=accz;
                   
          } 
           else if(rxbuf[4]==2) { // motion data ( just tip info for now)
          
           txbuf[1]=0;
           txbuf[2]=0;
           txbuf[3]=tipval;
                      
          }
          
        txbuf[4]=statusflags; 
         txlen=5;
     
        }// command 02
   
      //..........................................................  
        
        if(txlen) { // initiate reply
        TCNT1=0;
        if (targetid)  OCR1A=timeslot_instant; // sent to my ID - no timeslot-specific delay
        else OCR1A=(unsigned int)my_timeslot*(bytetime*txlen+slotgap); //broadcast - delay til my timeslot
     TIFR=(1<<OCF1A); // clear int flag
     TIMSK|=(1<<OCIE1A); // enable Timer 1 compare 1 interrupt           
          
        }
          rxptr=0; // reset rx state
  
}



//------------------------------------------------------------------ inithardware
void inithardware(void) 
{
  //UART 
  UCSRA=0; // bit 1 = U2X, set 1 to double baudrate
  UBRRH=0;
  UBRRL=0x01; // 1=250k @8MHz (U2X = 0) 
  UCSRB=  (1<<RXCIE) | (1<<UDRIE) | (1<<RXEN) | (1<<TXEN);// enable txc, rx ints
  UCSRC=6;
  
  // timer 0 - tick cycle and PWM for laser enable charge pump
  
 TCCR0A=0x83; // fastpwm
 TCCR0B=0x03; //clk/64 clock 8us roll8 2.048ms
 OCR0A=0; // laser off
  
 //timer 1 - tx timeslot delay based on node ID
 TIMSK=0; // no ints yet
 TCCR1A=0x00;
 TCCR1B=0x02; // clk/8 clock 1uS, roll16 65.536mS

}


//======================================================================================
//** warning - take care with assignments to portd (led in particular)
// non-atomic read-mod-writes can mess up interrupt code txen handling
// assign led to constant only, i.e. if (x) led=1; else led = 0;, NOT led=x?1:0;


void main( void )
{
char flashtimer;
    DDRD=0xfe;
    DDRB=0x4f;
    PORTB=PORTD=0;  
    WDTCR=0x1e; //set WDCT
     WDTCR=0x1e; // ~1S for bootloader timeout
     if((MCUSR & 1<<WDRF)==0) boot();

 inithardware();
 __enable_interrupt();


   my_id=ee_devid; // avoid silly ID on new chip ; 
   if((my_id==0xff) || (my_id==0)) ee_devid=my_id=254; // avoid silly ID on new chip
   
   my_timeslot=ee_timeslot; 
   if((my_timeslot<timeslot_min) || (my_timeslot>timeslot_max))  my_timeslot=ee_timeslot=timeslot_default;

statusflags=status_reset;

TIMSK|=1<<TOV0; // enable laser /acc timer interupt

 do {
          __watchdog_reset();  
          if(acctimer&0x80) { // 16ms tick
            acctimer+=acccycle; // reload timer    
            flashtimer++;
            if(flags & eeslotupdate) 
               { flags&=~eeslotupdate; if(ee_timeslot!=my_timeslot) ee_timeslot=my_timeslot; } // only write ee if changed to save wear
            if(flags & bootreq) boot();
            
         led=1; // dim if acc OK - on for duration of acc read, very short if no ack from acc
        readacc(); 
                 
        if(accx& 0x40) { // bad accdata
        tipval=0xfe; // maximum tip to turn laser off, dont use ff due to comms framing
        statusflags|=status_badacc;
        }
        else {
        statusflags&=~status_badacc;
        if(tipfilt>31) tipfilt=31; // clip filter value
        tippage=(tippage*tipfilt)/32+(unsigned int)((accz+0x20) & 0x3f)*(32-tipfilt); // lowpass filter tip value
        tipval=tippage>>4; 
      
        } // good acc data
 
        if(tipval>tipthresh) lasholdoff=lasholdtime; // restart holdoff as long as tipped, 16ms ticks
        if(lasholdoff) {lasholdoff--;statusflags|=status_tip;laseren=0;} // kill laser til holdtime expired 
              else {statusflags&=~status_tip;laseren=1;} // laser enable
        
        led=0;      
        if(OCR0A) { // if laser commanded on
                  if(laseren) led=1;  //not disabled by tip-over
                   else if(flashtimer&2) led=0;else led=1;} // led flash if inhibitd by tip
        
            }

  
  } while(1);
}
