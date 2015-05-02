// A simple sketch to pass multiple values to raspberry pi nrf hub
// For more details see http://blog.riyas.org
// credits to maniacbug,pynrf, and other libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
// For using the temperature sensor DS18S20
#include <OneWire.h> 
int DS18S20_Pin = 8; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2

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

// here we can send up to 30 chars
char SendPayload[31] = "NODE1";
char tempstr[10] = "NODE1";//to store dtostr

void setup(void) {
  Serial.begin(57600); //Debug 
  printf_begin();
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
  
  //Get temperature from sensor
    float temperature = getTemp();
    float humidity =23; // add a sensor here
    float pressure=24;  // add another sensor here
  // Assign temperature to payload, here am sending it as string
  
  //add a temperature in between tag
  strcat(SendPayload, "T");
  dtostrf(temperature,2,2,tempstr); 
  strcat(SendPayload,tempstr); 
  strcat(SendPayload, "T");   // add first string
  
  //Humidity
  strcat(SendPayload, "H");
  dtostrf(humidity,2,2,tempstr); 
  strcat(SendPayload,tempstr); 
  strcat(SendPayload, "H");   // add first string
  
  //Pressure
  strcat(SendPayload, "P");
  dtostrf(pressure,2,2,tempstr); 
  strcat(SendPayload,tempstr); 
  strcat(SendPayload, "P");   // add first string

  //send a heartbeat
  radio.stopListening();
  bool ok = radio.write(&SendPayload,strlen(SendPayload));
  radio.startListening(); 
  //Serial.println(SendPayload);  

  // slow down a bit
  delay(1000);  
}


// Getting temperature from DS18S20

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}
