#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
// For using the temperature sensor DHT-11
#include "DHT.h" // dht temp humidity sensor library
#define DHTPIN 4 // set digital port 4
#define DHTTYPE DHT11
DHT dht(DHTPIN,DHTTYPE);

//for nrf24 debug
int serial_putc( char c, FILE * ) 
{
  Serial.write( c );
  return c;
} 

//for nrf24 debug
void printf_begin(void)
{
  fdevopen( &serial_putc, 0 );
}

//nRF24 set the pin 9 to CE and 10 to CSN/SS
// Cables are:
//     SS       -> 10
//     MOSI     -> 11
//     MISO     -> 12
//     SCK      -> 13

RF24 radio(9,10);

//we only need a write pipe, but am planning to use it later
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL,0xF0F0F0F0D2LL };
//max payload size
char SendPayload[31] = "payloaddummy";
char tempstr[10] = "";

void setup(void) {
  Serial.begin(57600); //Debug 
  printf_begin();
  // start DH11
  dht.begin();
  //nRF24 configurations
  radio.begin();
  radio.setChannel(0x4c);
  radio.setAutoAck(1);
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(32);
  radio.openReadingPipe(1,pipes[0]);
  radio.openWritingPipe(pipes[1]);
  radio.startListening();
  radio.printDetails(); //for Debugging
}

void loop() {

  // Getting temperature from dht11

  int t = dht.readTemperature(); // deg C
  int f = dht.readTemperature(true); // deg F
  int h = dht.readHumidity();

  if (isnan(t)){
    Serial.println("Sensor read failed");
    return;
  } 
 //add a temperature in between tag
  strcat(SendPayload, "T");
  dtostrf(t,2,2,tempstr); 
  strcat(SendPayload,tempstr); 
  strcat(SendPayload, "T");   // add first string
  
  //Humidity
  strcat(SendPayload, "H");
  dtostrf(f,2,2,tempstr); 
  strcat(SendPayload,tempstr); 
  strcat(SendPayload, "H");   // add first string
  
  //Pressure
  strcat(SendPayload, "P");
  dtostrf(h,2,2,tempstr); 
  strcat(SendPayload,tempstr); 
  strcat(SendPayload, "P");   // add first string

  radio.stopListening();
  bool ok = radio.write(&SendPayload,sizeof(SendPayload));
  radio.startListening(); 
  //Serial.println(SendPayload);  
  // slow down a bit
  delay(10000);  

}
