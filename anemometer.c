/*
 * 
 * Author(s):   Kim Holmberg Christensen 
 *              Jørgen Drelicharz Greve
 * 
 * Filename:    anemometer.c
 * Version:     1.0 
 * Date:        16.05.2021
 * 
 */


#include <stdio.h>
#include "anemometer.h"
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/adc1.h"
#include "mcc_generated_files/spi1.h"
#include "globalVariables.h"
#include "NRF.h"


/********** DEFINITIONS FOR getWindspeed() ************************************/
#define channel 0               // ADC channel used
#define ADCresHIGH 300          // 450mV = 3351
#define ADCresLOW 200           // 15mV  = 112
//////////////// NEW CODE NOT IMPLEMENTED YET //////////////////////////////////
//#define ADCresHiLoLim 250       // NEW DEFINE NOT IMPLEMENTED YET
////////////////////////////////////////////////////////////////////////////////
#define intrrTime 0.000010      // Timer 1 interrupt time
#define trnDist 0.73            // One scoup turn length in meters
#define calibFact 4,5           // Calibration constant. Changelog:
                                // 6 -> 3,5 -> 4,5

/********** DEFINITIONS FOR getWindspeed() ************************************/
#define testMode 3              // 1 = UART and LED, 2 = UART, 3 = LED
                                // The system will transmit more frequent when
                                // testMode is 1


/********** VARIABLES FOR getWindspeed() **************************************/
int i = 0;                      // for loop counting variable
int switchVar1 = 0;             // The switchVar 1, 2 and 3 variables are used - 
int switchVar2 = 0;             // to control the anemometer half turn timing -
int switchVar3 = 0;             // calculation.
int intrrCnt1ready = 0;         // First half turn interrupt count ready
int intrrCnt2ready = 0;         // Second half turn interrupt count ready
int halfTrn1cnt = 0;            // Counts number of half turns
int halfTrn2cnt = 0;            // Counts number of half turns
int trnTimeReady = 0;           // One turn time is calculated and ready
int conversion = 0;             // ADC conversion result
int ADCresult = 0;              // ADC result (1 or 0)
long intrrCnt1 = 0;             // Holds the latest number of half turn -
long intrrCnt2 = 0;             // interrupts.
long tmr1intrrCnt1 = 0;         // Extern variable counted one up in the timer -
long tmr1intrrCnt2 = 0;         // 1 interrupt callback function.
double trnTime = 0;             // Time of one turn in seconds
double tps = 0;                 // Number of turns per second
double wsUnc = 0;               // Uncalibrated windspeed
volatile int mpsReady = 0;      // 1 if m/s result is ready, 0 if not
volatile double mps = 0;        // Windspeed in meters per second


/********** VARIABLES FOR transmitMps() ***************************************/
char dataCheck[6] = {NULL};     // Used to sort out any extreme values
char dataBuff[6] = {NULL};      // Used to store the five latest mps values
char dataSum = 0;               // The sum of the latest five mps values
char mpsArr[6] = {NULL};        // Holds double to char array conv. mps result
char mpsSend[6] = {NULL};       // Used for inserting a W into the result
int j = 0;                      // for loop counting variable
int k = 0;                      // for loop counting variable
int sendSpeed = 0;              // How often to transmit data
float dataAvg = 0;              // Windspeed avg. for extreme value sorting
long cnt1 = 0;                  // Used to control the transmission speed




/////////// BEGINNING OF !!!NEW!!! CODE ////////////////////////////////////////
/////////// NEW CODE IS NOT IMPLEMENTED YET ////////////////////////////////////
/*
struct adcGet{
    int conversion;
    int normADC;
}adcGet;

struct trnTim{
    int trn;
    long oneTrnIntrrCnt;
    double trnTime;
}trnTim;

struct windSpeed{
    double tps;
    double wsUnc;
    double mps;
}windSpeed;

struct tx{
    char mpsArr[6];
    int k;
    char mpsSend[6];
}tx;


void wind(){
    getADC();
    
    if(tmr1intrrCnt2 >= 1000){
        transmit();
    }
}


void getADC(){
    struct adcGet conversion, adc;
    adc.conversion = 0;
    adc.normADC = 0;
    
    /////////// GET ADC CONVERSION RESULT //////////////////////////////////////
    // MCC generated ADC initialization routine
    
    ADC1_Enable();
    ADC1_ChannelSelect(channel);
    ADC1_SoftwareTriggerEnable();
    for(i=0;i <1000;i++){}          // Provide a small delay
    ADC1_SoftwareTriggerDisable();
    while(!ADC1_IsConversionComplete(channel));
    adc.conversion = ADC1_ConversionResultGet(channel);
    ADC1_Disable(); 
    
    /////////// ADC RESULT = HIGH/LOW //////////////////////////////////////////
    // Decide if the ADC conversion should be interpreted as 1 or 0
    if(adc.conversion > ADCresHiLoLim){
            adc.normADC = 1;
            turnTiming();
        }
    else if(adc.conversion <= ADCresHiLoLim){
            adc.normADC = 0;
            turnTiming();
        }
    else{
        
    }
}


void turnTiming(){
    struct trnTim trn;
    struct adcGet adc;
    
    if(adc.normADC != adc.normADC){
        if(trnTim.trn == 0){
            tmr1intrrCnt1 = 0;      //Start timer
        }
        trnTim.trn++;
    }
    
    if(trnTim.trn == 2){
       trnTim.trn = 0; 
       trnTim.oneTrnIntrrCnt = tmr1intrrCnt1; //Stop timer
       trnTim.trnTime = trnTim.oneTrnIntrrCnt * intrrTime;
       
       windspeedCal();
    }
    
}


void windspeedCal(){
    struct trnTim trn;
    struct windSpeed tps, wsUnc, mps;
    windSpeed.tps = 0;
    windSpeed.wsUnc = 0;   
    windSpeed.mps = 0;
    
    if(trnTim.trnTime != 0){  // Avoid division by zero
        windSpeed.tps = 1/trnTim.trnTime;  // Turns per second
        }
    else{
        windSpeed.tps = 0.0001;
        }
    windSpeed.wsUnc = trnDist * windSpeed.tps;    // Uncalibrated Windspeed
    windSpeed.mps = windSpeed.wsUnc * calibFact;  // Calibrated windspeed in m/s
}


void transmit(){
    struct windSpeed mps;
    struct tx mpsArr, k, mpsSend;
    tx.mpsArr = {NULL};
    tx.k = 0;
    
    sprintf(tx.mpsArr,"%lf",windSpeed.mps); // Convert double to char array (string)
    
    /////////// PREPARE AND TRANSMIT DATA //////////////////////////////////
    // Inserting a W in front of the windspeed data to sort good data from
    // bad and to be able to send and process other types of data later on
    for(tx.k=1;tx.k<7;tx.k++){
        tx.mpsSend[0] = 'W';
        tx.mpsSend[k] = tx.mpsArr[k-1];
        }

    Send_Data_NRF(tx.mpsSend);         // SPI to NRF module
    
    tmr1intrrCnt2 = 0;      //Restart transmit delay after tx
    
}
*/
////////////////////////////////////////////////////////////////////////////////
/////////// END OF !!!NEW!!! CODE //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////






/////////// GET CURRENT WINDSPEED FROM ANEMOMETER //////////////////////////////
void getWindspeed(void)
    {
    
    /////////// GET ADC CONVERSION RESULT //////////////////////////////////////
    // MCC generated ADC initialization routine
    
    ADC1_Enable();
    ADC1_ChannelSelect(channel);
    ADC1_SoftwareTriggerEnable();
    for(i=0;i <1000;i++)            // Provide a small delay
    {
    }
    ADC1_SoftwareTriggerDisable();
    while(!ADC1_IsConversionComplete(channel));
    conversion = ADC1_ConversionResultGet(channel);
    ADC1_Disable(); 
    
    /////////// ADC RESULT = HIGH/LOW //////////////////////////////////////////
    // Decide if the ADC conversion should be interpreted as 1 or 0
    if(conversion > ADCresHIGH)
        {
        ADCresult = 1;
        }
    
    if(conversion <= ADCresLOW)
        {
        ADCresult = 0;
        }
    
    
    /////////// HALF TURN TIMING ///////////////////////////////////////////////
    // Using timer interrupt to time a anemometer half turn
    if(ADCresult == 1 && switchVar1 == 0)   // Hall sensor = HIGH
        {
        intrrCnt2 = tmr1intrrCnt2;  // 2nd half turn interrupt count update
        tmr1intrrCnt1 = 0;          // Begin 1st half turn interrupt count
        switchVar1 = 1;
        switchVar2 = 1;

        if(switchVar3 == 1 && halfTrn2cnt < 2) // Make sure only ONE half turn 
            {                                  // has passed
            intrrCnt2ready = 1;     // 2nd half turn interrupt count ready
            switchVar3 = 0;
            halfTrn2cnt = 0;
            }
        if(halfTrn2cnt >= 2)
            {
            intrrCnt2ready = 0;     // 2nd half turn interrupt count NOT ready
            halfTrn2cnt = 0;        // Restart half turn count
            }
        halfTrn1cnt++;
        }
     
    if(ADCresult == 0 && switchVar1 == 1 && switchVar2 == 1)// Hall sensor = LOW
        {
        intrrCnt1 = tmr1intrrCnt1;  // 1st half turn interrupt count update
        tmr1intrrCnt2 = 0;          // Begin 2nd half turn interrupt count
        switchVar1 = 0;
        switchVar2 = 0;
        switchVar3 = 1;
        
        if(halfTrn1cnt < 2)         // Make sure only ONE half turn has passed
            {
            intrrCnt1ready = 1;     // 1st half turn interrupt count ready
            halfTrn1cnt = 0;
            }
        if(halfTrn1cnt >= 2)
            {
            intrrCnt1ready = 0;     // 1st half turn interrupt count NOT ready
            halfTrn1cnt = 0;        // Restart half turn count
            }
        halfTrn2cnt++;
        }
    
     
    /////////// WINDSPEED CALC /////////////////////////////////////////////////
    if(intrrCnt1ready == 1)             // Calculate turn time if count is ready
        {
        intrrCnt1ready = 0;
        trnTime = 2*(intrrCnt1 * intrrTime);    // 2x because of half turn
        intrrCnt1 = 0;
        trnTimeReady = 1;               // Turn time is ready
        }

    if(intrrCnt2ready == 1)             // Calculate turn time if count is ready
        {
        intrrCnt2ready = 0;
        trnTime = 2*(intrrCnt2 * intrrTime);    // 2x because of half turn
        intrrCnt2 = 0;
        trnTimeReady = 1;               // Turn time is ready
        }

    if(trnTimeReady == 1)
        {
        if(trnTime != 0)                // Avoid division by zero
            {
            tps = 1/trnTime;            // Turns per second
            }
        else
            {
            tps = 0.0001;
            }
        wsUnc = trnDist * tps;          // Uncalibrated Windspeed
        mps = wsUnc * calibFact;        // Calibrated windspeed in m/s
        trnTimeReady = 0;
        mpsReady = 1;                   // Windspeed is ready for transmission
        }
    }



/////////// TRANSMIT CURRENT WINDSPEED /////////////////////////////////////////
void transmitMps(void)
    {
    cnt1++;
    
    /////////// DETERMINE HOW OFTEN TO TRANSMIT DATA ///////////////////////////
    // cnt1 is used as a simple timer that counts one up every time to avoid
    // using timer interrupt resources on something that does not need it.
    sendSpeed = -383 * mps + 25000;    // Determine transmission interval from
                                       // current windspeed (mps))
    if(sendSpeed > 25000)              // Limit thelowest transmission speed
        {                              // to about every 15 seconds if windspeed
        sendSpeed = 25000;             // is 0 m/s
        }
    if(sendSpeed < 2000)               // Limit the highest transmission speed
        {                              // to about every 1 second if windspeed
        sendSpeed = 2000;              // is 60 m/s
        }
    
    
    if(testMode == 1)                  // If the system is in test mode 1 it 
        {                              // will transmit more frequently
        sendSpeed = 1000;
        }
    
    
    /////////// TRANSMIT DATA //////////////////////////////////////////////////
    // Transmit about every 10 seconds at low windspeeds and about every second
    // at high windspeeds with a fluent transition between low and high
    // windspeeds.
    if(mpsReady == 1 && cnt1 > sendSpeed)
        {
        cnt1 = 0;
        sprintf(mpsArr,"%lf",mps);      // Convert double to char array (string)
        mps = 0;
        
        /////////// PREPARE AND TRANSMIT DATA //////////////////////////////////
        // Inserting a W in front of the windspeed data to sort good data from
        // bad and to be able to send and process other types of data later on
        for(k=1;k<7;k++)
            {
            mpsSend[0] = 'W';
            mpsSend[k] = mpsArr[k-1];
            }
        
        Send_Data_NRF(mpsSend);         // SPI to NRF module
        
        for(k=0; k<=sizeof(mpsSend); k++)
            {
            mpsSend[i] = '0';           // Reset the array
            }
        
        if(testMode == 1)               // Used for testing the system
            {
            printf("%s\n",mpsSend);             // UART for test
            LATBbits.LATB5 = !LATBbits.LATB5;   // Control LED
            }
        if(testMode == 2)               // Used for testing the system
            {
            printf("%s\n",mpsSend);             // UART for test
            }
        if(testMode == 3)               // Used for testing the system
            {
            LATBbits.LATB5 = !LATBbits.LATB5;   // Control LED
            }
        }
    }