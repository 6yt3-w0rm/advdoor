#include <IRremote.h>
const int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

#include <Servo.h>

Servo myservo;

//-----------------------pins-------------------------------------

const int redLed = 8;
const int greenLed = 6;
const int reedSw = A2;
const int autoTogglePin = A1;
const int touch = A0;
const int motorVss = 7;
const int servoPort = 9;

//----------------------------------------------------------------

//-----------------------variables--------------------------------

int count = 0;

bool lock = 0;
bool sBreak = 0;
bool trigEvent = 1;
bool autoLock = 1;
bool masterToggle = 1;
bool remoteToggle = 1;

bool reedStatus = 1;
bool reset = 0;

//----------------------------------------------------------------

//----------------------------RFID--------------------------------

#include <EEPROM.h>
#include <SPI.h>    
#include <MFRC522.h> 

bool programMode = false;

uint8_t successRead; 

byte storedCard[4];   
byte readCard[4];  
byte masterCard[4];

#define SS_PIN 10
#define RST_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);

//----------------------------------------------------------------

void serial(){
  Serial.println("---------------------");
  Serial.print("trigEvent : ");
  Serial.println(trigEvent);
  Serial.print("reedStatus : ");
  Serial.println(reedStatus);
  Serial.print("reset : ");
  Serial.println(reset);
  Serial.print("lock : ");
  Serial.println(lock);
  Serial.print("auto : ");
  Serial.println(reset);
}

void lSt(int g, int r){
  digitalWrite(redLed, r); 
  digitalWrite(greenLed, g);}

void aLock(){

  reedStatus = !digitalRead(reedSw);
  if(!reedStatus){reset=1;}
  masterToggle = digitalRead(autoTogglePin);
  /*
  if(masterToggle){
    if(remoteToggle||digitalRead(autoTogglePin)){autoLock=1;}
  }
  if(reset){if(autoLock && reedStatus){lock=1; trigEvent=1; reset=0;}}
*/

if(reset && masterToggle){ if(reedStatus){lock=0; trigEvent=1;reset=0; delay(100);}}
  
}

void manualTouch() {
  if(digitalRead(A0)){delay(100);lock=!lock; trigEvent=1;}
}

void setup() {  
  Serial.begin(9600);
  myservo.attach(servoPort);
  irrecv.enableIRIn();
  pinMode(motorVss, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(reedSw, INPUT);
  pinMode(touch, INPUT);
  pinMode(autoTogglePin, INPUT);
  for(int i=0;i<10;i++){
  lSt(1, 1);delay(50);lSt(0, 0);delay(50);
  }


  //------------------------RFID------------------------------------
//  pinMode(wipeB, INPUT_PULLUP);
 
  
  SPI.begin();
  mfrc522.PCD_Init();
  

}

void motor(){
  if(lock){myservo.write(108);}else{myservo.write(65);} 
  digitalWrite(motorVss, 0);
  //trigEvent = 0;
  }

void loop() {
  
//  serial();
 count = trigEvent + count;
 if(count > 3){trigEvent = 0; count = 0;}
 if (irrecv.decode(&results)){
         switch (results.value){
          case 0xFF38C7:
           lock = !lock;
           trigEvent=1;
           delay(15);
              break;
           case 0xFF6897:
           lock = 1;
           trigEvent = 1;
           delay(15);
              break;
         }
         delay(100);
        irrecv.resume();
       }

    manualTouch();

    aLock(); 

//----------------------------------RFID--------------------------

//successRead = getID();

if ( findID(readCard) ) { 
       lock=1;
       trigEvent=1;
      }
      else {      
       
      }

//----------------------------------------------------------------
       
//if(sBreak){lSt(0, 0);}
  if(trigEvent){digitalWrite(motorVss, 1);delay(100);}
      if(lock){           
        lSt(0, 1);
       motor();}
       else{
        lSt(1, 0);
       motor();}

       }

  
///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  if (mfrc522.PICC_IsNewCardPresent()) {
  for ( uint8_t i = 0; i < 4; i++) { readCard[i] = mfrc522.uid.uidByte[i]; }
  mfrc522.PICC_HaltA(); // Stop reading
  //return 1;
  }
/*  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
   // return 0;
  }*/

}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
       return false;
    }
  }
  return true;  
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
    else {    // If not, return false
    }
  }
  return false;
}
