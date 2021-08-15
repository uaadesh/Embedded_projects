#include<mega16.h>
#include<delay.h>

void main()
{
int z[]={0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};
int currflr=0;
int targflr=0;
int flrdiff=0;
//Target floor display
PORTA=0b00000000;
DDRA=0b01111111;

//Floor buttons and power display
PORTB=0b00011111;
DDRB=0b01100000;

//floor and door motors
PORTC=0b00000000;
DDRC=0b00001111;

//Current floor display
PORTD=0b00000000;
DDRD=0b01111111;

//Motor clk==up, anticlk==down.
PORTA=z[0];
PORTD=z[0];
PORTB.5=1;
while(1)
{
if(PINB.0==0)   //Ground floor
{
   delay_ms(300);
   targflr=0;
   PORTA=z[targflr];
}
if(PINB.1==0)   //first floor
{
   delay_ms(300);
   targflr=1;
   PORTA=z[targflr];
}
if(PINB.2==0)   //second floor
{
   delay_ms(300);
   targflr=2;
   PORTA=z[targflr];
}
if(PINB.3==0)   //third floor
{
   delay_ms(300);
   targflr=3;
   PORTA=z[targflr];
}
if(PINB.4==0)   //start button
{
   delay_ms(300);
   flrdiff=targflr-currflr;
   if(flrdiff>0)
   {
      while(flrdiff!=0)
      {
         PORTC.0=1;
         PORTC.1=0;
         delay_ms(4000);
         flrdiff=flrdiff-1;
         currflr=currflr+1;
         PORTD=z[currflr];
         //led blink at each floor
         PORTB.6=1;
         delay_ms(200);
         PORTB.6=0;
      }
      PORTC.0=0;
      PORTC.1=0;
      //Door operation=>
      delay_ms(1000);
      //open
      PORTC.2=0;
      PORTC.3=1;
      delay_ms(1000);
      //hold
      PORTC.2=0;
      PORTC.3=0;
      delay_ms(2000);
      //close
      PORTC.2=1;
      PORTC.3=0;
      delay_ms(1000);
      PORTC.2=0;
      PORTC.3=0;
   }
   else
   {
      flrdiff=flrdiff*(-1);
      while(flrdiff!=0)
      {
         PORTC.0=0;
         PORTC.1=1;
         delay_ms(4000);
         flrdiff=flrdiff-1;
         currflr=currflr-1;
         PORTD=z[currflr];
         //led blink at each floor
         PORTB.6=1;
         delay_ms(200);
         PORTB.6=0;
      }
      PORTC.0=0;
      PORTC.1=0;
      //Door operation=>
      delay_ms(1000);
      //open
      PORTC.2=0;
      PORTC.3=1;
      delay_ms(1000);
      //hold
      PORTC.2=0;
      PORTC.3=0;
      delay_ms(2000);
      //close
      PORTC.2=1;
      PORTC.3=0;
      delay_ms(1000);
      PORTC.2=0;
      PORTC.3=0;
   }
}
}
}