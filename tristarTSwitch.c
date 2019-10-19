#include "tristarTSwitch.h"

typedef struct {
	int16 t_setpoints[16];
} struct_config;



typedef struct {
	/* most recent valid */
	/* circular buffer for ADC readings */
	int8 restart_cause;
	int8 rotary_switch_value;
} struct_current;


/* global structures */
struct_config config;
struct_current current;

#include "adc_tristarTSwitch.c"


 // #define RS232_DEBUG 1


int8 read_rotary_switch(void) {
	int8 value=0;

	/* turn on port b pull up resistors on rotary switch pins */
	port_b_pullups(0b00111100);
	delay_ms(1);


	if ( 0 == input(ROTARY_SW_1) )
		value+=1;
	if ( 0 == input(ROTARY_SW_2) )
		value+=2;
	if ( 0 == input(ROTARY_SW_4) )
		value+=4;
	if ( 0 == input(ROTARY_SW_8) )
		value+=8;

	/* shut off all pullups to save power */
//	port_b_pullups(0b00000000);

	return value;
}


void modbus_tristar_disable() {
	output_high(RS232_EN);
	delay_ms(10);

	/* disable output
	01,05,00,01,FF,00,DD,FA (slave address=1)

	*/
	fputc(0x01,STREAM_TRISTAR);
	fputc(0x05,STREAM_TRISTAR);
	fputc(0x00,STREAM_TRISTAR);
	fputc(0x01,STREAM_TRISTAR);
	fputc(0xFF,STREAM_TRISTAR);
	fputc(0x00,STREAM_TRISTAR);

	/* CRC */
	fputc(0xDD,STREAM_TRISTAR);
	fputc(0xFA,STREAM_TRISTAR);

	delay_ms(10);
	/* turn off RS-232 port */
	output_low(RS232_EN);

	/* red to signfiy Tristar is disabled */
	output_low(LED_GREEN);
	output_high(LED_RED);
}

void modbus_tristar_enable() {
	output_high(RS232_EN);
	delay_ms(10);

	/* enable output
	01,05,00,01,00,00,9C,0A (slave address=1)
	*/
	fputc(0x01,STREAM_TRISTAR);
	fputc(0x05,STREAM_TRISTAR);
	fputc(0x00,STREAM_TRISTAR);
	fputc(0x01,STREAM_TRISTAR);
	fputc(0x00,STREAM_TRISTAR);
	fputc(0x00,STREAM_TRISTAR);
	
	/* CRC */
	fputc(0x9C,STREAM_TRISTAR);
	fputc(0x0A,STREAM_TRISTAR);

	delay_ms(10);
	/* turn off RS-232 port */
	output_low(RS232_EN);

	/* green to signfiy Tristar is disabled */
	output_high(LED_GREEN);
	output_low(LED_RED);
}



void init() {
	setup_oscillator(OSC_4MHZ);	

	setup_adc(ADC_CLOCK_DIV_8); 
	setup_adc_ports(sAN0, VSS_VDD);
	ADFM=1; /* right justify ADC results */

	setup_dac(DAC_OFF);
	setup_vref(VREF_OFF);
	setup_spi(SPI_DISABLED);

	setup_wdt(WDT_ON);
}

void print_restart_cause(int8 val) {
	switch ( val ) {
		case WDT_TIMEOUT: fprintf(STREAM_TRISTAR,"WDT_TIMEOUT"); break;
		case MCLR_FROM_SLEEP: fprintf(STREAM_TRISTAR,"MCLR_FROM_SLEEP"); break;
		case MCLR_FROM_RUN: fprintf(STREAM_TRISTAR,"MCLR_FROM_RUN"); break;
		case NORMAL_POWER_UP: fprintf(STREAM_TRISTAR,"NORMAL_POWER_UP"); break;
		case BROWNOUT_RESTART: fprintf(STREAM_TRISTAR,"BROWNOUT_RESTART"); break;
		case WDT_FROM_SLEEP: fprintf(STREAM_TRISTAR,"WDT_FROM_SLEEP"); break;
		case RESET_INSTRUCTION: fprintf(STREAM_TRISTAR,"RESET_INSTRUCTION"); break;
		default: fprintf(STREAM_TRISTAR,"unknown!");
	}
	fprintf(STREAM_TRISTAR,"\r\n");
}

void set_config(void) {
	config.t_setpoints[0]=905;  // -15C
	config.t_setpoints[1]=871;  // -10C
	config.t_setpoints[2]=832;  // -5C
	config.t_setpoints[3]=786;  // 0C
	config.t_setpoints[4]=736;  // 5C
	config.t_setpoints[5]=683;  // 10C
	config.t_setpoints[6]=626;  // 15C
	config.t_setpoints[7]=569;  // 20C
	config.t_setpoints[8]=512;  // 25C
	config.t_setpoints[9]=457;  // 30C
	config.t_setpoints[10]=405; // 35C
	config.t_setpoints[11]=357; // 40C
	config.t_setpoints[12]=313; // 45C
	config.t_setpoints[13]=273; // 50C
	config.t_setpoints[14]=238; // 55C
	config.t_setpoints[15]=512; // user defined - default to 25C
}


void main(void) {
	int16 tSet;
	int16 adc;
	int8 bootRestartCause;
	int8 i;

	/* record restart cause before it gets lost */
	bootRestartCause=restart_cause();

	restart_wdt();

	/* setup hardware */
	init();

#ifdef RS232_DEBUG	
	/* turn on RS-232 port */
	output_high(RS232_EN);
	delay_ms(10);
	fprintf(STREAM_TRISTAR,"\r\n\r\n# tristarTSwitch.c cold boot %s\r\n",__DATE__);

	fprintf(STREAM_TRISTAR,"# boot restart_cause()=%u ",bootRestartCause);
	print_restart_cause(bootRestartCause);

	delay_ms(10);
	output_low(RS232_EN);
#endif

	/* flash the number of times + 1 of our switch value */
	current.rotary_switch_value=read_rotary_switch();
	for ( i=0 ; i<=current.rotary_switch_value ; i++ ) {
		/* orange means Tristar not yet set */
		output_high(LED_GREEN);
		output_high(LED_RED);
		delay_ms(250);
		output_low(LED_GREEN);
		output_low(LED_RED);
		delay_ms(250);
	}


	/* load  hard coded settings */
	set_config();

	for ( ; ; ) {
		/* WDT should wake up from sleep and get here within a few instructions */
		current.restart_cause = restart_cause();

		/* orange means Tristar not yet set */
		output_high(LED_GREEN);
		output_high(LED_RED);

		delay_ms(500);


		/* setup ADC and read input voltage */
		setup_adc(ADC_CLOCK_DIV_8); 
		setup_adc_ports(sAN0, VSS_VDD);
		ADFM=1; /* right justify ADC results */

		/* read rotary switch and lookup temperature set point */
		current.rotary_switch_value=read_rotary_switch();
		tSet=config.t_setpoints[current.rotary_switch_value];

		/* read temperatures */
		adc=read_adc_average16(0);

		if ( adc<tSet ) {
			/* temperature above set point */
			modbus_tristar_disable(); 
		} else {
			/* temperature below set point */
			modbus_tristar_enable(); 
		}

		/* shut off ADC before going to sleep to save power */
		setup_adc(ADC_OFF);

		/* sleep restarts the watchdog so we should get a relative consistent wakeup */
		sleep();
		/* instruction is pre-fetched prior to sleep ... so run a NOP as first new instruction */
		delay_cycles(1);
	}
}


#if 0

#ifdef RS232_DEBUG	
		/* turn on RS-232 port */
		output_high(RS232_EN);
		delay_ms(10);
		fprintf(STREAM_TRISTAR,"\r\n\r\n# tristarTSwitch.c warm boot %s\r\n",__DATE__);

		fprintf(STREAM_TRISTAR,"# restart_cause()=%u ",current.restart_cause);
		print_restart_cause(current.restart_cause);

		fprintf(STREAM_TRISTAR,"# current.sequence_number=%lu\r\n",current.sequence_number);

		fprintf(STREAM_TRISTAR,"# config.v_contactor_on_above=%lu\r\n",config.v_contactor_on_above);
		fprintf(STREAM_TRISTAR,"# config.v_contactor_off_below=%lu\r\n",config.v_contactor_off_below);
#endif

#ifdef RS232_DEBUG	
			fprintf(STREAM_TRISTAR,"# rotarySwitchValue=%u tSet=%lu\r\n",current.rotary_switch_value,tSet);
#endif



#ifdef RS232_DEBUG	
			fprintf(STREAM_TRISTAR,"# [0]=%lu\r\n",adc);
#endif

#ifdef RS232_DEBUG	
		delay_ms(10);
		/* turn off RS-232 port */
		output_low(RS232_EN);
#endif


#endif