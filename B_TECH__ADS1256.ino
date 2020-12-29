#include "ads1256.h"
#include <SPI.h>

#define DtRdy 6 //dataready pin
#define CS 10  //ChipSelect pin
#define R 8   //reset pin
#define Mhz 1920000


int32_t data;

String inByte;
double multi = 0.000000596;
int counter = 0;
double voltage;
double voltA;
int lastC;

//========================================================================= SETUP
void setup() {
 Serial.begin(250000);        //connect
 SPI.begin();               //Port ready
 
 pinMode(DtRdy,INPUT);      //DataReady wait
 pinMode(CS,OUTPUT);        //ChipSelect RDY
 pinMode(R,OUTPUT);         //init start ADS

 digitalWrite(CS,HIGH);     //Bus is free
 digitalWrite(R,HIGH);      //ADS Ready to run
 Serial.println("Setup OK!");
 Reset();
 wrSTATUS();
 delay(100);
 checkSTATUS();
}
//========================================================================= LOOP
void loop() {
 
  if (Serial.available() > 0) {
      Decision(Serial.readString());
      
 }
 delay(100);
}
//========================================================================= RESET
void Reset(){
SPI.beginTransaction(SPISettings(1700000, MSBFIRST, SPI_MODE1)); // initialize SPI with  clock, MSB first, SPI Mode1

  digitalWrite(CS, LOW);
  delayMicroseconds(10);
  SPI.transfer(0xFE); //Reset
  delay(2); //Minimum 0.6ms required for Reset to finish.
  SPI.transfer(0x0F); //Issue SDATAC
  delayMicroseconds(10);
  digitalWrite(CS, HIGH);

  SPI.endTransaction();
Serial.println("Reset OK");

}
//========================================================================= RDATAC
int32_t rdatac (){
  int32_t data = NULL;
  
  
  SPI.beginTransaction(SPISettings(Mhz, MSBFIRST, SPI_MODE1)); // initialize SPI with  clock, MSB first, SPI Mode1

  digitalWrite(CS, LOW);
  delayMicroseconds(10);
  SPI.transfer(0x03); //Rdatac
  delayMicroseconds(10);
  
do{
  while (digitalRead(DtRdy)!=0){}
  data |= SPI.transfer(0xFF); //Rdatac recieve MSB
  data<<=8;
  data |= SPI.transfer(0xFF); //Rdatac recieve MidSB
  data<<=8;
  data |= SPI.transfer(0xFF); //Rdatac recieve LSB

  if (long minus = data >> 23 == 1) 
  {
    data = data - 16777216;  //for the negative sign
  }
  voltage = multi*data;
  Serial.print("[ "); Serial.print(voltage*1000000,3); Serial.print(" mV"); Serial.print(" ]"); Serial.println(" ");//return data
  if(lastC==0){data = 0;}

if(Serial.available() > 0) {  
       inByte = Serial.readString(); }
       
      if(lastC==0){delay(200);}  //measurement delay
       delay(20);
//COUNTER HERE+++++++++++++++++++++++++++++++++++++
counter++;
if(lastC!=0){
  voltA=voltA+voltage*1000000;
  data = 0;
 // Serial.println(voltA);
  
  if(lastC == counter){counter = 0;
                         inByte = "STOP";}
  }

}while (inByte!="STOP");
  
   while (digitalRead(DtRdy)!=0){} //WAIT TILL DRDY FALSE
   SPI.transfer(0x0F); //correct SDATAC issuing
  
  delayMicroseconds(10);
  digitalWrite(CS, HIGH);

  SPI.endTransaction();

inByte = " ";
counter = 0;
Serial.println("RDATAC issued");

  }
//========================================================================= selfcheck
void checkSTATUS(){
  SPI.beginTransaction(SPISettings(Mhz, MSBFIRST, SPI_MODE1)); // initialize SPI with  clock, MSB first, SPI Mode1

  digitalWrite(CS, LOW);
  delayMicroseconds(10);
  SPI.transfer(B00010011);//READ STATUS
  SPI.transfer(B00000000);//BYTE
  while (digitalRead(DtRdy)!=0){}
  
  int stat = SPI.transfer(0xFF); 
  Serial.println(stat,BIN);
  delayMicroseconds(10);
  digitalWrite(CS, HIGH);

  SPI.endTransaction();

  }

void wrSTATUS(){
  SPI.beginTransaction(SPISettings(Mhz, MSBFIRST, SPI_MODE1)); // initialize SPI with  clock, MSB first, SPI Mode1

  digitalWrite(CS, LOW);
  delayMicroseconds(10);
  SPI.transfer(B01010011); //WREG DRATE
  SPI.transfer(B00000000); //BYTE
  SPI.transfer(B00110011); // 50 SPS
  delayMicroseconds(10);
  digitalWrite(CS, HIGH);

  SPI.endTransaction();
  }

double Average(){
                 Serial.print("Input quantity of measurements to average it: ");
                 while (lastC==0)
                 {
                 if(Serial.available() > 0) {  
                 lastC = Serial.parseInt();
                 Serial.println(lastC);}
                 }
                 rdatac();
                 
                 Serial.println(voltA/lastC);
                 voltA=0;
                 lastC = 0;
}

  
void Decision(String str)
{
  Serial.println(str);
  if(str == "HELP")
  {           Serial.println("<Command>,         <Description>");
              Serial.println("  HELP,             Issues this list");
              Serial.println("  START,            Take Voltage between AIN0 & AIN1 continiously");
              Serial.println("  STOP,             Stops START command, if nothing to stop -> restart ADS");
              Serial.println("  GET,              Voltage - correct = result, will run 1 time");
              Serial.println("  ZERO,             Gets the average value out of N measures");
    return;
  }
  if(str == "START")
  {
    rdatac();
    return;
  }
  if(str == "STOP")
  {
    Serial.println("A: Run START command first. There is nothing to stop.");
    Serial.println("A: ADS reset issued.");
    Reset();
    return;
  }
  if(str == "GET")
  {
    
    return;
  }
  if(str == "ZERO")
  {
    Average();
    return;
  }
  Serial.println("A: That command is not supported. Type HELP command to get full list of available commands.");
  return;
}
