#include <18F26K22.h>
#device ADC=10
#device *=16

#include <stdlib.h>
#FUSES INTRC_IO
#FUSES NOPROTECT
#FUSES PUT
#FUSES NOLVP
#FUSES BROWNOUT
#FUSES BORV29
#FUSES NOMCLR
#FUSES NOWDT
//#FUSES WDT // does not appear to be working
//#FUSES WDT_SW
#FUSES WDT512 // 512 = 10 seconds
//#FUSES WDT1024 // 1024 = 18 seconds
//#FUSES WDT2048 // 2048 = 32 seconds
#FUSES NOPLLEN
#FUSES NOFCMEN
#FUSES NOIESO
#FUSES NOXINST
#FUSES NODEBUG
#use delay(clock=4000000, restart_wdt)


/* UART1 - connection to two TriStar charge controllers */
#use rs232(UART1,stream=STREAM_TRISTAR,baud=9600,xmit=PIN_C6,rcv=PIN_C7,errors)	


#byte TXSTA=GETENV("SFR:txsta1")
#bit  TRMT=TXSTA.1

#byte TXSTA2=GETENV("SFR:txsta2")
#bit  TRMT2=TXSTA2.1

#byte PORTB=GETENV("SFR:portb")
#byte INTCON2=GETENV("SFR:intcon2")
#bit RBPU=INTCON2.7

#use standard_io(A)
#use standard_io(B)
#use standard_io(C)
#use standard_io(E)

#define RS232_EN                 PIN_A7
#define LED_RED                  PIN_C0
#define LED_GREEN                PIN_C1
#define SER_TO_TS                PIN_C6
#define SER_FROM_TS              PIN_C7

#define ROTARY_SW_1              PIN_B2
#define ROTARY_SW_2              PIN_B3
#define ROTARY_SW_4              PIN_B4
#define ROTARY_SW_8              PIN_B5

#define SER_TO_NET               PIN_B6
#define SER_FROM_NET             PIN_B7


/* analog inputs ADC channels*/
#define AN_CH_T0	0	// RA0 / AN0 (on board NTC)


typedef union {
	int16 l[2];
    int8 b[4];
    int32 word;
} u_lblock;

#byte port_b=GETENV("SFR:portb")
#byte port_c=GETENV("SFR:portc")
#BIT ADFM=GETENV("BIT:ADFM") 
