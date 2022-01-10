/*
 * 
 * Author(s):   Kim Holmberg Christensen 
 *              Jørgen Drelicharz Greve
 * 
 * Filename:    main.c
 * Version:     1.0 
 * Date:        16.05.2021
 * 
 */

#include "mcc_generated_files/system.h"
#include "mcc_generated_files/adc1.h"
#include "NRF.h"
#include "anemometer.h"
#include "tempsensor.h"


int main(void)
{
    SYSTEM_Initialize();        // MCC initialize
    startup_NRF_delay();        // SPI startup delay
    pwr_up_NRF();               // Start the NRF module
    config_NRF();               // Configure the NRF module
    ADC1_Initialize();          // Initialize ADC1
    
    while (1)
    {
        getWindspeed();         // Calculate windspeed from ADC data
        transmitMps();          // Transmit windspeed
    }
    
    
    
    return 1;
}
