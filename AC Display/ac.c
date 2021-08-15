#include<mega16.h>
#include<mylcd.h>

void main()
{
int status=0;
int temp=27;
PORTD=0b00000111;
DDRD=0b00000000;

while(1)
{
if(PIND.0==0)
{
   delay_ms(100);
   status=1;
   lcd_init();
   lcd_s("*LG ELECTRONICS*");
   lcd_xy(1,0);
   lcd_s("- Saving Power -");
   delay_ms(5000);
   lcd_cmd(0x01);
   lcd_s("TEMP:");
   lcd_3d(temp);
   lcd_s("  C|+|-|");
   lcd_xy(0,9);
   lcd_char(0b11011111);
   lcd_xy(1,0);
   lcd_s("STATUS:NORMAL");
}
if((PIND.1==0) && (status==1)&& (temp<36))
{
   delay_ms(100);
   //the icon dissapears when the button is pressed
   lcd_xy(0,12);
   lcd_s(" ");
   delay_ms(100);
   lcd_xy(0,12);
   lcd_s("+");
   
   lcd_xy(0,5);
   temp=temp+1;
   lcd_3d(temp);
   if((temp<=36) && (temp>=31))
   {
   lcd_xy(1,7);
   lcd_s("HOT     ");
   }
   else if((temp<=30) && (temp>=25))
   {
   lcd_xy(1,7);
   lcd_s("NORMAL  ");
   }
   else if((temp<=24) && (temp>=18))
   {
   lcd_xy(1,7);
   lcd_s("COOL    ");
   }
}
if((PIND.2==0) && (status==1)&& (temp>18))
{
   delay_ms(100);
   //the icon dissapears when the button is pressed
   lcd_xy(0,14);
   lcd_s(" ");
   delay_ms(100);
   lcd_xy(0,14);
   lcd_s("-");
   
   lcd_xy(0,5);
   temp=temp-1;
   lcd_3d(temp);
   if((temp<=36) && (temp>=31))
   {
   lcd_xy(1,7);
   lcd_s("HOT     ");
   }
   else if((temp<=30) && (temp>=25))
   {
   lcd_xy(1,7);
   lcd_s("NORMAL  ");
   }
   else if((temp<=24) && (temp>=18))
   {
   lcd_xy(1,7);
   lcd_s("COOL    ");
   }
}
}
}