#include <SPI.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;//defines Real Time Clock parameters
File myFile;//Creates a file in the SD card to write data
SoftwareSerial Sense(5,6);//Sensor serial ports initialize
SoftwareSerial BT (8,9);//Bluetooth serial ports initialize
int co2 = 0, humidity = 0, temperature = 0;
byte setPolling[5] = {75, 32, 50, 13, 10};
byte getCO2[3] = {90, 13, 10};//creates parameters for Cozir reader to get CO2 readings
byte getHumidity[3] = {72, 13, 10};//creates parameters for Cozir reader to get Relative Humidity readings
byte getTemperature[3] = {84, 13, 10};//creates parameters for Cozir reader to get Temperature readings
const int chipSelect = 10;//Sets pin for data to be written to SD card

void setup()
{
Serial.begin(9600); //Set up Arduino (always comes first).
Serial.println("CO2 Meter COZIR sampling");
while(!Serial){// wait for serial port to connect.
}
Sense.begin(9600);//Initializes Sensor
Sense.write(setPolling, sizeof(setPolling)); // Set COZIR to polling mode. 
Serial.println("COZIR sendor set to polling mode.");
BT.begin(9600);//Initialize Bluetooth module
BT.println("Bluetooth Connected");
Serial.print("Initializing SD card...");
if (!SD.begin(chipSelect)) //Initializes SD card
  {
  Serial.println("initialization failed!");
  return;
  }
Serial.println("SD Initialized");
Wire.begin();
 RTC.begin();//initiates Real Time Clock
if (! RTC.isrunning()) {
  //Serial.println("RTC is NOT running!");
  RTC.adjust(DateTime(__DATE__, __TIME__));//adjusts date and time to computer 
  }
 Serial.println(' ');
}
int getReading(String str, byte command[], int n)//Communicates with Cozir sensor to attain reading
{
  byte rbuffer;
  float reading = 0, multiplier = 10;
  boolean goodReading = false;

 // Serial.println("getReading...");
  Sense.write(command, n);
  delay(100);
    rbuffer = Sense.read();
    // Look for measurement flag: H: Humidity, T: Temperature, Z: CO2.
    while (rbuffer != command[0]) {
      rbuffer = Sense.read();
  //  Serial.println(rbuffer);
    }
    // Look for first digit.
    while ((rbuffer < 48) || (rbuffer > 57)) {
      rbuffer = Sense.read();
  //    Serial.println(rbuffer);
    }
    // Read up to CR.
    while (rbuffer != 13) {
      // Read digits and convert to number value.
      while (rbuffer > 47 && rbuffer < 58) {
        reading = multiplier * reading + (float)(rbuffer - 48);
        rbuffer = Sense.read();
      }
    }
    rbuffer = Sense.read();
    if (rbuffer == 10) {
      myFile.print(reading);
    //  Bluetooth.print(reading);
      Serial.print(reading);
      goodReading = true;
    } else {
      myFile.println("Bad reading.  Try again.");
   //   Bluetooth.println(reading);
      Serial.println(reading);
    }
} // End of get Reading.


int SerTimeStamp () {
//Prints a time stamp (mm/dd/yyy hh:mm:ss) to computer serial port
    DateTime now = RTC.now();
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print('/');    
    Serial.print(now.year(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print(",");
}
void MyTimeStamp () {
//Prints a time stamp (mm/dd/yyy hh:mm:ss) to SD Card
    DateTime now = RTC.now();
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print('/');    
    myFile.print(now.year(), DEC);
    myFile.print(' ');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
    myFile.print(':');
    myFile.print(now.second(), DEC);
    myFile.print(",");
}
int MyHeader(){
//Prints header to (Date, CO2, Relative Humidity, Temperature) to SD card
    myFile.print(String("Date,"));
    myFile.print(String("CO2,"));
    myFile.print(String("Relative Humidity,"));
    myFile.println(String("Temperature"));
}
int SerHeader(){
//Prints header to (Date, CO2, Relative Humidity, Temperature) to computer serial port
    Serial.print(String("Date,"));
    Serial.print(String("CO2,"));
    Serial.print(String("Relative Humidity,"));
    Serial.println(String("Temperature"));
}
void GetData(){
//Runs getReading definition to attain different sensor readings and returning a string (CO2, Relative Humidity, Temperature)
  getReading(" ", getCO2, sizeof(getCO2));
  myFile.print(",");
  Serial.print(",");
  getReading(" ", getHumidity, sizeof(getHumidity));
  myFile.print(",");
  Serial.print(",");
  getReading(" ", getTemperature, sizeof(getTemperature));
  myFile.println(" ");
  Serial.println(" ");
}

void SenseData(){
//Print sensor data to .txt file and computer Serial with time stamp
myFile = SD.open("test.txt", FILE_WRITE);//print to SD card
if(SD.exists("test.txt")){
  Sense.listen();//tells Serial to listen to Sensor port
  MyTimeStamp();
  SerTimeStamp();
  GetData();
  Serial.println(" ");
  myFile.close();
  BT.listen(); //Tells serial port to listen to Bluetooth Module
  }
}
//If experimenting with bluetooth this will poll SD card and write it to BT Serial Port
/*void BlueWrite(){
//polls SD card and writes the data in the Bluetooth Tterminal
      File myFile=SD.open("test.txt");
      if(myFile)
        {
        while(myFile.available())
          {
          BT.write(myFile.read());
          }
        myFile.close();
        }
       else{
        BT.println("SD Card Removed or Not Available");
       }
}

char a;//creates a variable to be used in the switch*/
void loop(){
 // Bluetooth must listen for the majority of the time to hear switch from computer
 /* BT.listen();//tells serial port to read Bluetooth module
  delay(3000);//waits for set time before next reading or command
  a=(BT.read());//reads the Bluetooth command and assigns it to character "a".
  switch (a){
    case'b'://if "b" is typed in to bluetooth module the 
    {
    BlueWrite(); 
    }
  }*/
  SenseData();
  delay(3000);
}   
  
