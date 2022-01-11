/*
 * 
 * Author(s):   Kim Holmberg Christensen 
 *              Jørgen Drelicharz Greve
 * 
 * Filename:    anemometer.h
 * Version:     1.0 
 * Date:        16.05.2021
 * 
 */

#ifndef ANEMOMETER_H
#define	ANEMOMETER_H

#ifdef	__cplusplus
extern "C" {
#endif

///////////////// NEW CODE NOT IMPLEMENTED YET /////////////////////////////////
/*
void wind();
void getADC();
void turnTiming();
void windspeedCal();
void transmit();
*/
////////////////////////////////////////////////////////////////////////////////


void getWindspeed(void);    // Calculate windspeed from ADC data
void transmitMps(void);     // Transmit windspeed

#ifdef	__cplusplus
}
#endif

#endif	/* ANEMOMETER_H */

