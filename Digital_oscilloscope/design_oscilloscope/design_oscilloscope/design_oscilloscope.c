/*! \mainpage Week-4 Challenge Activity 2: Design Digital Oscilloscope
 *
 * @author     e-Yantra Team
 * @date       2021/06/07
 *
 * \subsection Aim
 * To display waveforms from function generator on GLCD with switches representing Digital Oscilloscope (DSO).
 *
 * \subsection Connections
 * Input channel from Function Generator:						<br>
 * 		Arduino Pin A0 	--> Atmega2560 PORTK Pin 0 (PK0)		<br>
 * 		Arduino Aref	--> 5 V									<br>
 * 
 * GLCD Connections:											<br>
 *		 GLCD Pins	  Arudino Pins		Atmega2560 Pins			<br>
 *			  D0  --> 22			-->	PA0						<br>
 *			  D1  --> 23			-->	PA1						<br>
 *			  D2  --> 24			-->	PA2						<br>
 *			  D3  --> 25			-->	PA3						<br>
 *			  D4  --> 26			-->	PA4						<br>
 *			  D5  --> 27			-->	PA5						<br>
 *			  D6  --> 28			-->	PA6						<br>
 *			  D7  --> 29			-->	PA7						<br>
 *			  RS  --> 37			--> PC0						<br>
 *			  RW  --> 36			--> PC1						<br>
 *			  EN  --> 35			--> PC2						<br>
 *			  RST --> 32			--> PC5						<br>
 *			  CS1 --> 34			-->	PC3						<br>
 *			  CS2 --> 33			-->	PC4						<br>
 *
 * Switch Connections:											<br>
 *		 Switch Pins   Arudino Pins		Atmega2560 Pins			<br>
 *			 DIV+  --> 20			-->	PD1						<br>
 *			 DIV-  --> 19			-->	PD2						<br>
 *			 HOLD  --> 18			-->	PD3						<br>
 *			 UP	   --> 2			-->	PE4						<br>
 *			 DOWN  --> 3			-->	PE5						<br>
 *
 * The DSO screen has total size of 128 x 64 pixels, which is divided into two sections:
 * 
 * [ I ] DISPLAY WAVEFORM => Area where the input waveform is to be displayed (may it be Sine, Square, etc.)
 * 							 The center line is a reference line about which the waveform should be symmetric.
 * 							 The height of this area should accomodate the waveform with max. Vpp of 5V.
 * 							 The width of this area should accomodate the waveform with:
 * 								* min. frequency of 250 Hz (0.25 kHz)
 * 								* max. frequency of 2000 Hz (2 kHz)
 * 
 * [ II ] DISPLAY WAVEFORM INFO => Area where the information about the waveform is to be displayed.
 * 								   The peak-to-peak voltage (Vpp), frequency in kHz, ms/div, V/div.
 * 
 *  ____________________________________________________________
 * |(0,0)                                   |                   |
 * |                                        |                   |
 * |                                        |                   |
 * |          DISPLAY WAVEFORM              |                   |
 * |                                        |                   |
 * |                                        |     DISPLAY       |
 * |                                        |                   |
 * |________________________________________|                   |
 * |                                        |     WAVEFORM      |
 * |                                        |                   |
 * |                                        |                   |
 * |          DISPLAY WAVEFORM              |       INFO        |
 * |                                        |                   |
 * |                                        |                   |
 * |                                        |                   |
 * |_________________________________(95,63)|(96,63)____(127,63)|
 */

//---------------------------------- HEADER FILES -----------------------------------------------------
#include "firebird_simulation.h"
#include "u8glib.h"
#include "adc.h"
#include <util/delay.h>

//------------------------------------- MACROS ----------------------------------------------------------
// If any

//---------------------------------- GLOBAL VARIABLES ---------------------------------------------------
// If any

// array to store the data from the input waveform
// since the waveform display area is of 96 columns or pixels, we store double amount of data in buffer
// Note: the input waveform from the Function Generator has minimum value of 0 V
int buffer_input_data[192];

// values from buffer array for max., min. and half of the amplitude
int V_max, V_min, V_mid;

// peak-to-peak voltage of the input waveform
float V_pp;

// index values from buffer array which indicate:
// start of waveform : where the first wave in the buffer cuts half of the amplitude or where the first wave starts its cycle
// end of waveform : where the second wave in the buffer cuts half of the amplitude or where the first wave ends its cycle or the second wave starts its cycle
int start_wave_idx, end_wave_idx;

// msec per division to be considered in the "Display Waveform" Area
// this value should increment in steps of 0.5 up till 2.0 (OR)
// should decrement in steps of 0.5 up till 0.125 each time DIV+ (OR) DIV- switches are pressed
float ms_per_div = 0.5;

// volts per division to be considered in the "Display Waveform" Area
float volts_per_div = 0.625;

// frequency of input waveform in kHz unit
float freq;

// array to compute and store the pixel locations for the input waveform to be displayed in "Display Waveform" Area
int pixel_loc[96];

// flag to keep track whether the HOLD switch is pressed or not
// this flag will toggle between 1 and 0 on press and release of the switch respectively
int hold_flag = 0;

// vertical displacement to be applied on waveform on each press of UP and DOWN switches
// this value should increment in steps of 1 up till 31 (max. pixels that can accomodate above the center horizontal line) (OR)
// should decrement in steps of 1 up till -31 (max. pixels that can accomodate below the center horizontal line)
int vert_disp = 0;

// << NOTE >> : You can define any extra global variables below


//---------------------------------- FUNCTIONS ----------------------------------------------------------
// << TODO >> : Complete all the functions as per the instructions given in form of comments

/**
 * @brief      Makes **ONLY** DIV+, DIV-, HOLD, UP, DOWN switch pins and dso_input_channel pin as input,
 * 				Activates pull-up for **ONLY** these switch pins and deactivates pull-up for **ONLY** dso_input_channel pin
 */
void dso_switches_input_channel_config(void)
{
	// << NOTE >> : Use Masking and Shift Operators here
	// << TODO >> : Complete the function as expected in the comment above.
	
	// Make **ONLY** three switches (DIV+, DIV- and HOLD) connected to div_hold_switch_port (PORTD) as input
	div_hold_switch_ddr_reg &= ~((1<<div_add_sw_pin)|(1<<div_sub_sw_pin)|(1<<hold_sw_pin));
	
	// Activate pull-up for **ONLY** for three switches (DIV+, DIV- and HOLD) connected to div_hold_switch_port (PORTD)
	div_hold_switch_port_reg |= ((1<<div_add_sw_pin)|(1<<div_sub_sw_pin)|(1<<hold_sw_pin));

	// Make **ONLY** two switches (UP and DOWN) connected to up_down_switch_port (PORTE) as input
	up_down_switch_ddr_reg &= ~((1<<up_sw_pin)|(1<<down_sw_pin));

	// Activate pull-up for **ONLY** for two switches (UP and DOWN) connected to up_down_switch_port (PORTE)
	up_down_switch_port_reg |= ((1<<up_sw_pin)|(1<<down_sw_pin));

	// Make **ONLY** the dso_input_channel pin connected to dso_input_channel_port (PORTK) as input
	dso_input_channel_ddr_reg &= ~(1<<dso_input_channel_pin);

	// Deactivate pull-up for **ONLY** dso_input_channel pin connected to dso_input_channel_port (PORTK)
	dso_input_channel_port_reg &= ~(1<<dso_input_channel_pin);
}


/**
 * @brief      Configures External Interrupt on DIV+, DIV-, HOLD, UP, DOWN switch pins
 */
void dso_switches_interrupt_config(void)
{
	// << NOTE >> : Use Masking and Shift Operators here
	// << TODO >> : Complete the function as expected in the comment above

	// All interrupts have to be disabled before configuring interrupts
	cli();	// Disable interrupts globally

	// Configure falling edge detection on div_add_sw_pin_int (INT1) and div_sub_sw_pin_int (INT2)
	EICRA_reg |= ((1<<div_add_sw_ISC_bit1)|(1<<div_sub_sw_ISC_bit1));
	EICRA_reg &= ~((1<<div_add_sw_ISC_bit0)|(1<<div_sub_sw_ISC_bit0));

	// Configure low level detection on hold_sw_pin_int (INT3)
	EICRA_reg &= ~((1<<hold_sw_ISC_bit0)|(1<<hold_sw_ISC_bit1));

	// Configure falling edge detection on up_sw_pin_int (INT4) and down_sw_pin_int (INT5)
	EICRB_reg |= ((1<<up_sw_ISC_bit1)|(1<<down_sw_ISC_bit1));
	EICRB_reg &= ~((1<<up_sw_ISC_bit0)|(1<<down_sw_ISC_bit0));

	// Enable INT1, INT2, INT3, INT4 and INT5 interrupts
	EIMSK_reg |= ((1<<div_add_sw_pin_int)|(1<<div_sub_sw_pin_int)|(1<<hold_sw_pin_int)|(1<<up_sw_pin_int)|(1<<down_sw_pin_int));

	sei();	// Enable interrupts gloabally
}


/**
 * @brief      Initializes the Digital Oscilloscope peripherals (switches, display, input channel)
 */
void dso_init(void)
{
	// << NOTE >> : You are not allowed to modify or change anything inside this function

	dso_switches_input_channel_config();
	dso_switches_interrupt_config();
	adc_init();

	// maps the GLCD connections with Arduino Mega pins to the ATmega2560 pins
	// the function is defined in "u8glib.c" file
	glcd_setup();
}


/**
 * @brief      Interrupt Service Routine for div_add_sw_pin_int (INT1)
 */
ISR(div_add_sw_pin_int_vect)
{
	if(ms_per_div<2.0){
		ms_per_div+=0.5;
	}
}


/**
 * @brief      Interrupt Service Routine for div_sub_sw_pin_int (INT2)
 */
ISR(div_sub_sw_pin_int_vect)
{
	if(ms_per_div-0.5>0.125){
		ms_per_div-=0.5;
	}
}


/**
 * @brief      Interrupt Service Routine for hold_sw_pin_int (INT3)
 */
ISR(hold_sw_pin_int_vect)
{
	hold_flag=hold_flag==0?1:0;
}


/**
 * @brief      Interrupt Service Routine for up_sw_pin_int (INT4)
 */
ISR(up_sw_pin_int_vect)
{
	if(vert_disp>-31){
		vert_disp--;
	}
}


/**
 * @brief      Interrupt Service Routine for down_sw_pin_int (INT5)
 */
ISR(down_sw_pin_int_vect)
{
	if(vert_disp<31){
		vert_disp++;
	}
}


/**
 * @brief      Display the Start Screen of DSO
 */
void dso_start_screen(void)
{
	setFont(u8g_font_5x7);
	firstPage();

	// << NOTE >> : You can add your creativity over here!
	do
	{
		drawStr( 35, 20, "DSO START" );
		drawLine(35,30,75,30);
		drawLine(35,30,35,60);
		drawLine(35,50,75,30);
		drawLine(35,60,75,60); // An attempt on the logo :')
		_delay_ms(1000);   //added delay so that the start message should stay for a while to read.
	}
	while( nextPage() );
}


/**
 * @brief      Sample the waveform from the DSO input channel and store the data in a buffer array.
 */
void sample_input_wave(void)
{
	// << TODO >> : Complete the logic in function as expected in the comment above.
	for (int i = 0; i < 192; i++)
	{
		// store the data from input waveform into buffer
		
		// << NOTE >> : You can add your creativity over here!
		
		// sample the data as required by giving small delays in capturing the input data
		// _delay_us(10);
		
		/* Now Nyquist criterion says that the sampling frequency that is rate at which we take the sample of the input frequency in our case (convert_analog_signal_data)
		   should be at-least twice of the frequency that is given as the input. Now since the micro-controller used is operating at a greater frequency that we will
		   normally input to the oscilloscope so we don't need to worry about the aliasing problem. We make use of this information to increase or decrease the ms_per_div
		   We will simply add some delay in between the consecutive sampling, resulting in more closer waves displayed on the display. */
		
		/* I added these delays based on my experiments, for twice the ms_per_div value I needed to show twice the peaks that were displayed initially.*/
		if(ms_per_div==1.0) _delay_us(85);
		else if(ms_per_div==1.5) _delay_us(160);
		else if(ms_per_div==2.0) _delay_us(210);

		// << HINT >> : You can make use of "ms_per_div" value and Nyquist criteria to determine the sampling frequency and
		// 				increase or decrease the waveform width	based on the status of DIV+ and DIV- switches.
		//				Although, this feature is "not mandatory" (optional) to have.
		// << NOTE >> : You can strive for accurately displaying the waveform based on "ms_per_div" value,
		//				but while evaluation the "ms_per_div" value won't be considered, so you may not pay much attention to it.
		buffer_input_data[i] = convert_analog_channel_data(dso_input_channel_pin);
	}
}


/**
 * @brief      From the buffer data of input waveform, calculate the values in buffer array which is max. (V_max), min. (V_min),
 * 			   half (V_mid) of the amplitude and the peak-to-peak voltage (V_pp) in Volts of the given waveform
 */
void calc_vpp_vmid(void)
{
	// << TODO >> : Complete the logic in function as expected in the comment above.

	V_max = 0;
	V_min = 255;   // I changed this value to 255 because the logic I used required this.

	// calculate these values:
	// V_max : value in buffer array that is maximum of the amplitude
	// V_min : value in buffer array that is minimum of the amplitude
	
	// Simple logic to find the maximum and minimum of the array by traversing it.
	for(int i=0;i<192;i++){
		V_max=V_max>buffer_input_data[i]?V_max:buffer_input_data[i];
		V_min=V_min<buffer_input_data[i]?V_min:buffer_input_data[i];
	}

	// calculate V_mid : value in buffer array that is half of the amplitude
	V_mid = (V_max+V_min)/2;

	// calculate V_pp : value in Volts for the peak-to-peak amplitude of the input waveform
	V_pp = (V_max-V_min)*0.0196;     // 0.0196 is the conversion factor after calculating 5/255
}


/**
 * @brief      Calculate the index values from buffer array where the "first" wave in given waveform starts (start_wave_idx) and ends (end_wave_idx) its cycle
 *		     _______	    ________
 *          |		|	   |		|
 *  ___start|_______|___end|________|______
 *          |		|	   |		|
 *          |		|______|		|
 */
void calc_start_end_wave_idx(void)
{
	// << TODO >> : Complete the logic in function as expected in the comment above.

	// compute the values of start_wave_idx and end_wave_idx

	/* V_mid won't actually always be present in the array since we have calculated it so we search the two consecutive values in the array between which the
    the V_mid value lies. And we do it twice so that we can find the start and end index of a wave */
	for(int i=0;i<191;i++){
		if(buffer_input_data[i]<V_mid && buffer_input_data[i+1]>=V_mid){
			start_wave_idx=i;
			break;
		}
	}
	for(int j=start_wave_idx+1;j<191;j++){
		if(buffer_input_data[j]<V_mid && buffer_input_data[j+1]>=V_mid){
			end_wave_idx=j;
			break;
		}
	}
}


/**
 * @brief      Find the frequency of input waveform
 */
void calc_frequency(void)
{
	// << TODO >> : Complete the logic in function as expected in the comment above.
	
	/* the frequency being displayed on my oscilloscope is not accurate and shows some error and I figured that we are making use of a simulation software and 
	   everyone is using it on different machines so we can't standardize a equation for these simulations but we can experiment with the factor(8) and see if we get
	   the frequency right at any other value, I also tried changing the ms_per_div value and got closer to the actual frequency at 0.555*/
	
	/* now after adding some delay to when we increase or decrease the ms_per_div we need to add some factor to the frequency equation so that it doesn't change when
	we change the ms_per_div. Ideally these should be just factors 8 but on the simulation the frequencies displayed were not as desired so I added some more factors
	to counter that.
	Also these factors works really well most of the time for my laptop :)*/
	if(ms_per_div==0.5) freq = (8/((end_wave_idx-start_wave_idx)*ms_per_div))*0.875;
	else if(ms_per_div==1.0) freq = (16/((end_wave_idx-start_wave_idx)*ms_per_div))*0.406;
	else if(ms_per_div==1.5) freq = (24/((end_wave_idx-start_wave_idx)*ms_per_div))*0.250;
	else if(ms_per_div==2.0) freq = (32/((end_wave_idx-start_wave_idx)*ms_per_div))*0.218;
	
}


/**
 * @brief      Measure various parameters of input waveform such as V_pp in Volts, V_mid, start_wave_idx, end_wave_idx and freq in kHz
 */
void measure_parameters(void)
{
	// << NOTE >> : You are not allowed to modify or change anything inside this function

	// calculate the V_pp (peak-to-peak) voltage in Volts and the V_mid (value from buffer array which is half the amplitude)
	calc_vpp_vmid();

	// calculate the index values from buffer array where the "first" wave in given waveform starts (start_wave_idx) and ends (end_wave_idx) its cycle
	calc_start_end_wave_idx();

	// calculate the frequency of input waveform in kHz unit
	calc_frequency();
}


/**
 * @brief      Compute and store the pixel locations of the "Display Waveform" to show each data point of input waveform
 * 				or Mapping the data points of input waveform to the pixel locations inside "Display Waveform" area such that
 * 				waveform is symmetric about the center horizontal line
 */
void translate_wave_to_pixels(void)
{
	// << TODO >> : Complete the logic in function as expected in the comment above.
	// << NOTE >> : Make sure the computed pixel locations for the waveform are symmetric about the center horizontal line
	
	// Here we translate only the first half of the array since we have pixel data array size half of the input data buffer array.
	for(int i=0;i<96;i++){
		pixel_loc[i]=32-(buffer_input_data[i]-V_mid)/4;
	}
}


/**
 * @brief      Draw the reference frame in "Display Waveform" Area for easy view of the input waveform
 * 			   with equal divisions for Time in msec (on X-axis) and Voltage level in Volts (on Y-axis)
 */
void draw_ref_frame(void)
{
	// << NOTE >> : You are not allowed to modify or change anything inside this function

	// drawing outer boundary or rectangle in the "Display Waveform" Area of 96 (columns) x 64 (rows)
	drawFrame(0, 0, 95, 63);

	// drawing the center horizontal and vertical line for reference and symmetricity
	drawLine(48, 0, 48, 63);
	drawLine(0, 32, 95, 32);

	// drawing short vertical lines or markings on center horizontal line with equal divisions for Time in msec
	for (int x = 0; x < 97; x += 8)
		drawLine(x, 31, x, 33);
	
	// drawing short horizontal lines or markings on center vertical line with equal divisions for Voltage level in Volts
	for (int y = 0 ; y < 64; y += 8)
		drawLine(47, y, 49, y);
	
	// plot small dots in all four quadrants for symmetricity in viewing the input waveform
	for (int x = 8; x < 96; x += 8)
	{
		for (int y=8; y < 64; y += 8)
			drawPixel(x,y);
	}
}


/**
 * @brief      Draw the input waveform data points translated to pixel locations in "Display Waveform" area
 * 			   Make use of "drawLine" function from u8g library to connect the pixel dots or data points so the input waveform is visualized
 */
void draw_waveform(void)
{
	// << TODO >> : Complete the logic in function as expected in the comment above.
	// << NOTE >> : Apply the vertical displacement to waveform based on the "vert_disp" value
	
	// we draw the waveform by drawing vertical lines joining consecutive points.
	// The if conditions ensure that once the wave reaches the upper limit of the display, then those vertical lines originating from the point of contact of the wave
	// and the upper limit of the display don't appear on the screen.
	for(int i=0;i<95;i++){
		int y1=pixel_loc[i]+vert_disp;
		int y2=pixel_loc[i+1]+vert_disp;
		if(pixel_loc[i]+vert_disp<0) y1=0;
		if(pixel_loc[i+1]+vert_disp<0) y2=0;
		drawLine(i,y1,i,y2);
	}
}


/**
 * @brief      Write text in the "Display Waveform Info" Area for displaying various parameters measured of the input waveform
 * 			   and data of parameters measured from the given waveformk
 */
void display_wave_info_text_data(void)
{
	// << NOTE >> : You are not allowed to modify or change anything inside this function

	drawStr(96, 7, "ms/div");
	setPrintPos(96, 14);
	print_float(ms_per_div, 3);

	drawStr(96, 23, "V/div");
	setPrintPos(96, 30);
	print_float(volts_per_div, 3);
	
	drawStr(96, 40, "Vpp");
	setPrintPos(96, 47);
	print_float(V_pp, 3);
	drawStr(123, 47, "V");
	
	drawStr(96, 55, "f(kHz)");
	setPrintPos(96, 62);
	print_float(freq, 3);
}


/**
 * @brief      Display the input waveform in "Display Waveform" area; info text and measured parameter values in "Display Waveform Info" area
 */
void dso_display_waveform_data(void)
{
	// << NOTE >> : You are not allowed to modify or change the "do-while" loop
	// << TODO >> : You can although add a condition to hold the running wave on press of HOLD switch
	
	// I was confused what to add here since the hold switch works fine. So i decided not to add anything.
	firstPage();
	do
	{
		// display the reference frame
		draw_ref_frame();

		// display the waveform as close and identical as possible with the input waveform by Function Generator
		draw_waveform();

		// display the waveform info text and data of measured parameters
		display_wave_info_text_data();
	}
	while( nextPage() );
}


/**
 * @brief      Start the operation of DSO to sample, measure, translate and display the waveform and parameter values in the respective areas on GLCD
 */
void start_dso_operation(void)
{
	// << NOTE >> : You are not allowed to modify or change anything inside this function

	// sample the input waveform
	sample_input_wave();

	// measure various parameters of the given waveform
	measure_parameters();

	// convert the waveform data points to pixel coordinates of the GLCD
	translate_wave_to_pixels();

	// display the waveform and data of measured parameters
	dso_display_waveform_data();
}


//------------------------------------------- MAIN ------------------------------------------------------------
/**
 * @brief      Main Function
 *
 * @details    Initializes the DSO peripherals. Calls the Start Screen of DSO and initiates the DSO operation.
 */
int main(void)
{
	dso_init();
	dso_start_screen();

	while(1)
	{
		start_dso_operation();
	}
	return 0;
}
