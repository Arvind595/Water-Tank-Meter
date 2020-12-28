/*
water tank 6.1
last modified: 2020-1-12
by Aravind
*/
//RECIVER 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define clockpin 10
#define datapin 9
#define latchpin 6
#define bz 5
#define irq 2

byte dataa = 31;  //31 states null for SR
RF24 radio(7, 8); // CE, CSN

const byte address[][6] = {"00001", "00002"};

void updateSR(byte v) { //Shift register Setting Bits function
  digitalWrite(latchpin, LOW);
  shiftOut(datapin, clockpin, MSBFIRST, v);
  digitalWrite(latchpin, HIGH);
}

void buzz() {
  tone(bz, 3000);
  delay(60);
  noTone(bz);
  delay(100);
}

void setup() {
  pinMode(irq, INPUT_PULLUP);
  pinMode(clockpin, OUTPUT); //SR defaults
  pinMode(datapin, OUTPUT);
  pinMode(latchpin, OUTPUT);

  digitalWrite(latchpin, LOW);
  digitalWrite(clockpin, LOW);

  Serial.begin(115200);

  radio.begin();
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.setAutoAck(1);
  radio.enableAckPayload();
  radio.maskIRQ(1, 1, 0); //(tx_ok,tx_fail,rx_ready);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();

  attachInterrupt(digitalPinToInterrupt(irq), message, FALLING);
  delay(1000);
  updateSR(0); //clear SR in setup
}

void loop() {

  for (int i = 0; i < 4; i++) {
    radio.stopListening();
    const byte text = 255;
    radio.write(&text, sizeof(text));
    radio.startListening();
    delay(10);
  }
  //The Tank is divided into 5 Levels hence 5bits
  //reciving sequence_ 5BITS MSB 1  1  1  1  1 LSB
  //____________________________L1 L2 L3  L4 L5___

Serial.println(dataa);
 if (dataa==31 || (~dataa)==0){
  updateSR(16);
  delay(50);
  updateSR(0);
  delay(50);
}
	//update LEDS based on L(levels)
  else if (dataa == 0 || (((~ dataa) & 1 ) == 1)) { //L5 CHECK
   Serial.println("L5");
    updateSR(31);
    buzz();
  }
   if ((((~dataa) & 16)>> 4 == 1)) {updateSR(16); Serial.println("L1");} //L1 CHECK
   if ((((~dataa) & 8) >> 3 == 1)) {updateSR(24); Serial.println("L2");} //L2 CHECK
   if ((((~dataa) & 4) >> 2 == 1)) {updateSR(28); Serial.println("L3");} //L3 CHECK
   if ((((~dataa) & 2) >> 1 == 1)) {updateSR(30); Serial.println("L4");} //L4 CHECK
  
//   else {
//      switch (dataa) {
//        case 1:  updateSR(30); //L4
//          break;
//        case 3:  updateSR(28); //L3
//          break;
//        case 7:  updateSR(24); //L2
//          break;
//        case 15: updateSR(16);//L1
//          break;
//      }
//    }

}
void message() { //interrupt function triggerd by RF 
  detachInterrupt(irq);
  if (radio.available()) {
    byte text = 1;
    radio.read(&text, sizeof(text));
    dataa = text;
    Serial.println(text);
  }
}