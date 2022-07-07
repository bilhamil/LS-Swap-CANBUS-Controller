#include <SPI.h>
#include <mcp2515.h>

/*
  Blink onboard LED at 0.1 second interval
*/
struct can_frame canMsg;
struct can_frame AC_ON_MSG;
struct can_frame AC_OFF_MSG;
struct can_frame AC_ON_MSG;
struct can_frame AC_OFF_MSG;
MCP2515 mcp2515(PB13);

int pinVal = 0;
bool pin = true;
bool onOff = true;

void setup() {
  //AC Serial ON 1F1 AE 16 00 00 08 00 00 7A 
  //src: https://pcmhacking.net/forums/viewtopic.php?f=38&t=6962&start=10
  AC_ON_MSG.can_id  = 0x1F1;
  AC_ON_MSG.can_dlc = 8;
  AC_ON_MSG.data[0] = 0xAE;
  AC_ON_MSG.data[1] = 0x16;
  AC_ON_MSG.data[2] = 0x00;
  AC_ON_MSG.data[3] = 0x00;
  AC_ON_MSG.data[4] = 0x08;
  AC_ON_MSG.data[5] = 0x00;
  AC_ON_MSG.data[6] = 0x00;
  AC_ON_MSG.data[7] = 0x7A;

  //AC Serial OFF 1F1 AE 0E 00 00 08 00 00 7A 
  //src: https://pcmhacking.net/forums/viewtopic.php?f=38&t=6962&start=10
  AC_ON_MSG.can_id  = 0x1F1;
  AC_ON_MSG.can_dlc = 8;
  AC_ON_MSG.data[0] = 0xAE;
  AC_ON_MSG.data[1] = 0x0E;
  AC_ON_MSG.data[2] = 0x00;
  AC_ON_MSG.data[3] = 0x00;
  AC_ON_MSG.data[4] = 0x08;
  AC_ON_MSG.data[5] = 0x00;
  AC_ON_MSG.data[6] = 0x00;
  AC_ON_MSG.data[7] = 0x7A;
  
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
  
  //delay(2000);
  mcp2515.reset();
  mcp2515.setPin(0b00001000);//turnRXbuf1 pin to GND
  
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  // set filter for only PRND Messages
  //mcp2515.setFilterMask(MCP2515::MASK0, false, 0x07FFFFFF);
  //mcp2515.setFilter(MCP2515::RXF0, false,      0x01F50F0F);
  mcp2515.setFilterMask(MCP2515::MASK0, false, 0x7FF);
  mcp2515.setFilter(MCP2515::RXF0, false,      0x1F5);
  mcp2515.setNormalMode();
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

  if (mcp2515.readMessage(MCP2515::RXB0, &canMsg) == MCP2515::ERROR_OK) {
    
    //digitalWrite(PC13, LOW);
    if(canMsg.can_id == 0x1F5 && canMsg.data[3] == 0x01)
    {
      digitalWrite(PC13, HIGH);
    }
    if(canMsg.can_id == 0x1F5 && canMsg.data[3] == 0x02)
    {
        digitalWrite(PC13, LOW);
    } 
      onOff = !onOff;
    
  }

  //delay(50);
}
