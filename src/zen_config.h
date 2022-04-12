#ifndef ZEN_CONFIG_H_
#define ZEN_CONFIG_H_


#define WIFISSID "Loowatt88"  //Enter wifi ssid between inverted commas
#define PASSWORD "Loowatt88"  //Enter wifi password between inverted commas
#define TOKEN "BBFF-RqTIKigU6wwypXiHpuLULbbMrcVVmG" //Enter API Key between inverted commas

 /*
 * Momentary Flush Duration in ms
 */
int FlushDuration = 1700; 

 /*
 * Refill length in ppr
 * MAX_FILM_COUNT = mm_ppr * actual_film_length_mm 
 * 257600 for 16m
 */
unsigned int MAX_FILM_COUNT = 257600 ; 

 /*
 * Actual length of Refill in mm
 * 16100 mm for a 16.0m refill
 */
int actual_film_length_mm = 16100; 

 /*
 * Flushable Refill Length
 * Need to be in float-> always use decimal place
 * 16.0 out of 16.1 for a 16.0m Refill
 */
float mm_ppr = 16.0;

 /*
 * Jam Detection RPM
 */
int jam_rpm = 36; 

#endif /* ZEN_CONFIG_H_ */