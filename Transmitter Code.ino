/*
water tank 6.1
last modified: 2020-1-12
by Aravind
*/

//TRANSMITTER

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define Ground_pin 10 //enable ground to read pin values
#define irq 2 //RF interrupt pin

byte dataa;
RF24 radio(7, 8); // CE, CSN
const byte address[][6] = {"00001", "00002"}; //address

void setup() {
  Serial.begin(9600);

  pinMode(Ground_pin, OUTPUT);
  digitalWrite(Ground_pin, HIGH);
  pinMode(irq, INPUT_PULLUP);

  pinMode(3, INPUT_PULLUP); //L1
  pinMode(4, INPUT_PULLUP); //L2
  pinMode(5, INPUT_PULLUP); //L3
  pinMode(6, INPUT_PULLUP); //L4
  pinMode(9, INPUT_PULLUP); //L5

  radio.begin();
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);
  radio.setAutoAck(1);
  radio.enableAckPayload();
  radio.maskIRQ(1, 1, 0); //(tx_ok,tx_fail,rx_ready); //enable RF interrupt
  radio.setPALevel(RF24_PA_HIGH);
  radio.startListening();
  attachInterrupt(digitalPinToInterrupt(irq), message, FALLING);
  delay(2000);
}
void readd() { //read the pinstates
  digitalWrite(Ground_pin, LOW);
  byte   binary = 0;

  for (uint8_t  i = 3; i <= 9; i++) {
    if (i == 7 || i == 8) continue;
    bool pinstate = digitalRead(i);
    binary = (binary << 1) | pinstate;
  }
  dataa = binary;
  Serial.println(binary, BIN);
  digitalWrite(Ground_pin, HIGH);
}

void message() { //send the data
  detachInterrupt(irq);
  if (radio.available()) {
    byte text = 0;
    radio.read(&text, sizeof(text));
    radio.stopListening();
    radio.write(&dataa, sizeof(dataa));
    radio.startListening();
  }

}
void loop() {
  
  readd();
  digitalWrite(Ground_pin, HIGH);
  //HERE ON THE CHIP IS DEEP SLEEP SUB MICRO AMP FOREVER
  //sleep forever
  //disable adc
  ADCSRA &= ~(1 << 7);
  SMCR |= (1 << 2);
  SMCR |= 1;
  //bod disable
  MCUCR |= (3 << 5);
  MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6);
  __asm__ __volatile__("sleep");
  delay(2000);
  readd();
}