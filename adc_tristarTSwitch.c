int16 read_adc_average16(int8 ch) {
	int16 sum;
	int8 i;


	// Calculate the mean.  This is done by summing up the
	// values and dividing by the number of elements.
	sum = 0;

	set_adc_channel(ch);
	delay_us(10);

	for( i = 0; i < 16 ; i++ ) {
		sum += read_adc();
	}

	/* divide sum by our 16 samples and round by adding 8 */
	return ( (sum+8) >> 4 );
}

