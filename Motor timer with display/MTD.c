#include<mega16.h>
#include<delay.h>

void main()
{
int ctr=0;
int z[]={0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};
PORTD=0b00000000;
DDRD=0b11111111;
PORTC=0b00000000;
DDRC=0b01000001;
PORTA=0b00000011;
DDRA=0b00000000;
PORTD=z[0];
while(1)
{
if((PINA.1==0) && (ctr<10))
{
   while(PINA.1==0)
   {}
   ctr=ctr+1;
   PORTD=z[ctr];
}
if(PINA.0==0)
{
   while(ctr!=0)
   {
      PORTC.0=1;
      PORTC.6=0;
      PORTD=z[ctr];
      delay_ms(1000);
      ctr=ctr-1;
   }
   PORTC.0=0;
   PORTC.6=0;
   PORTD=z[0];
}
}
}