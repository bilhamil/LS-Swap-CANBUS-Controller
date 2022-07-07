#include <SPI.h>
#include <mcp2515.h>

/*
  Blink onboard LED at 0.1 second interval
*/
struct can_frame canMsg1;
struct can_frame canMsg2;
MCP2515 mcp2515(PB13);

int pinVal = 0;
bool pin = true;
bool onOff = true;
bool sendOne = false;

bool error = true;
void setup() {
  canMsg1.can_id  = 0x1F5;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 0x0F;
  canMsg1.data[1] = 0x0F;
  canMsg1.data[2] = 0x00;
  canMsg1.data[3] = 0x01;
  canMsg1.data[4] = 0x00;
  canMsg1.data[5] = 0x00;
  canMsg1.data[6] = 0x03;
  canMsg1.data[7] = 0x00;

  canMsg2.can_id  = 0x1F5;
  canMsg2.can_dlc = 8;
  canMsg2.data[0] = 0x0F;
  canMsg2.data[1] = 0x0F;
  canMsg2.data[2] = 0x00;
  canMsg2.data[3] = 0x02;
  canMsg2.data[4] = 0x00;
  canMsg2.data[5] = 0x00;
  canMsg2.data[6] = 0x03;
  canMsg2.data[7] = 0x00;
  
  // initialize digital pin PB2 as an output.
  pinMode(PC13, OUTPUT); // LED connect to pin PB2
  pinMode(PA1, INPUT);
  pinMode(PB0, OUTPUT);

  //  setup serial
  //Serial.begin(4000);
  pinMode(PB12, INPUT_PULLUP);
  SPI.setMOSI(PB5);
  SPI.setMISO(PB4);
  SPI.setSCLK(PB3);
  //SPI.begin();
  
  delay(2000);
  error = false;

  mcp2515.reset();
  mcp2515.setPin(0b00001000);//turnRXbuf1 pin to GND
  //mcp2515.setPin(0b00101000);//turnRXbuf1 pin to VCC
  
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  
  mcp2515.setNormalMode();
  //mcp2515.setLoopbackMode();

  
}
void loop() {
  /*pinVal = analogRead(PA1);
  if(pinVal > 1000)
  {
    digitalWrite(PC13, HIGH);
  }
  else
  {
    digitalWrite(PC13, LOW);
  }

  pin = !pin;
  
  digitalWrite(PB0, pin?HIGH:LOW);

  
  delay(2000);*/
  
  
  MCP2515::ERROR out;
  if(sendOne)
  {
    out = mcp2515.sendMessage(MCP2515::TXB1, &canMsg1);
  }
  else
  {
    out = mcp2515.sendMessage(MCP2515::TXB1, &canMsg2);
  }
  sendOne = !sendOne;
  
  if(out == MCP2515::ERROR_OK)
  {
      if(onOff)
        digitalWrite(PC13, HIGH);
      else  
        digitalWrite(PC13, LOW);
      
      onOff = !onOff;
  }

  delay(100);
}
