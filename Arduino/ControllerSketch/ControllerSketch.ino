#include <SPI.h>
#include <mcp2515.h>

/*
  Blink onboard LED at 0.1 second interval
*/
struct can_frame canMsg;
struct can_frame AC_ON_MSG;
struct can_frame AC_OFF_MSG;
struct can_frame AC_PRESENT;

struct can_frame DASH_PRESENT;
struct can_frame DASH_MESSAGE_1;
struct can_frame DASH_MESSAGE_2;

#define PREAMBLE_SIZE 25
struct can_frame DASH_PREAMBLE[PREAMBLE_SIZE] = {{ 0x90004060, 1, {0x02}},
{ 0x8FFFE060, 0, {}},
{ 0x9002E060, 1, {0x01}},
{ 0x90046060, 1, {0x00}},
{ 0x9004E060, 5, {0x01, 0x79, 0xB4, 0x28, 0x00}},
{ 0x90138060, 1, {0x00}},
{ 0x90148060, 1, {0x00}},
{ 0x8FFFE060, 0, {}},
{ 0x90004060, 1, {0x02}},
{ 0x9000C060, 2, {0x02, 0x60}},
{ 0x90138060, 1, {0x01}},
{ 0x90004060, 1, {0x02}},
{ 0x90300060, 8, {0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04}},
{ 0x90302060, 8, {0x31, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
{ 0x8FFFE060, 0, {}},
{ 0x90306060, 4, {0x00, 0xC0, 0x00, 0x03}},
{ 0x9000C060, 2, {0x02, 0x60}},
{ 0x9001E060, 5, {0x86, 0x28, 0x04, 0xFF, 0x88}},
{ 0x9000C060, 2, {0x02, 0x60}},
{ 0x8FFFE060, 0, {}},
{ 0x900A4060, 3, {0x00, 0x00, 0x00}},
{ 0x8FFFE060, 0, {}},
{ 0x8FFFE060, 0, {}},
{ 0x9004E060, 5, {0x01, 0x79, 0xB4, 0x28, 0x10}},
{ 0x9001E060, 5, {0x86, 0x28, 0x04, 0xFF, 0x88}}};

HardwareSerial SerialHWD(USART1);   // PA3  (RX)  PA2  (TX)

MCP2515 mcp2515(PB13);

int pinVal = 0;
int messageCount = 0;
bool pin = true;
bool onOff = true;
bool lastAC = false;

long timeSinceLastRead = millis();
long timeSinceLastACMessage = millis();
long timeSinceLastDashMessage = millis();
long timeSinceLastOtherDashMessage = millis();

#define PARK_IND PB0
#define NEUTRAL_IND PA7
#define DRIVE_IND PA6
#define SECOND_IND PA5
#define LOW_IND PA8

#define CRUISE_IND PB10
#define AC_IN PA4
#define PWR_IND PB1


//send preamble so we pretend to be the dash (hopefully!)
void sendPreamble() {
  int i = 0;
  for(i = 0; i < PREAMBLE_SIZE; i++)
  {
     MCP2515::ERROR err;
     do{
      delay(30);
      err = mcp2515.sendMessage(&(DASH_PREAMBLE[i]));
      if(err == MCP2515::ERROR_OK)
      {
        printCanWithARB(DASH_PREAMBLE[i]);
        SerialHWD.println("");
      }
     } while(err != MCP2515::ERROR_OK);
  }

  SerialHWD.println("Preamble sent!");
}

void setup() {
  //0X60 low speed climate message
  AC_ON_MSG.can_id  = 0x800C0099;
  AC_ON_MSG.can_dlc = 7;
  AC_ON_MSG.data[0] = 0x41;
  AC_ON_MSG.data[1] = 0x00;
  AC_ON_MSG.data[2] = 0x00;
  AC_ON_MSG.data[3] = 0x24;
  AC_ON_MSG.data[4] = 0x00;
  AC_ON_MSG.data[5] = 0x06;
  AC_ON_MSG.data[6] = 0x00;
  //AC_ON_MSG.data[7] = 0x00;

  //0X60 low speed climate message
  AC_OFF_MSG.can_id  = 0x800C0099;
  AC_OFF_MSG.can_dlc = 7;
  AC_OFF_MSG.data[0] = 0x41;
  AC_OFF_MSG.data[1] = 0x00;
  AC_OFF_MSG.data[2] = 0x00;
  AC_OFF_MSG.data[3] = 0x24;
  AC_OFF_MSG.data[4] = 0x00;
  AC_OFF_MSG.data[5] = 0x05;
  AC_OFF_MSG.data[6] = 0x00;
  
  AC_PRESENT.can_id = 0x8FFFE099;
  AC_PRESENT.can_dlc = 0;

  DASH_PRESENT.can_id = 0x8FFFE060;
  DASH_PRESENT.can_dlc = 0;

  DASH_MESSAGE_1.can_id = 0x90004060;
  DASH_MESSAGE_1.can_dlc = 1;
  DASH_MESSAGE_1.data[0] = 0x02;
  
  DASH_MESSAGE_2.can_id = 0x9000C060;
  DASH_MESSAGE_2.can_dlc = 2;
  DASH_MESSAGE_2.data[0] = 0x02;
  DASH_MESSAGE_2.data[1] = 0x60;
  
  // initialize digital pin PB2 as an output.
  pinMode(PC13, OUTPUT); // LED connect to pin PB2
  pinMode(PA1, INPUT);
  pinMode(PB0, OUTPUT);
  pinMode(PA7, OUTPUT);
  pinMode(PA6, OUTPUT);
  pinMode(PA5, OUTPUT);
  pinMode(PA8, OUTPUT);
  pinMode(AC_IN, INPUT);
  pinMode(CRUISE_IND, OUTPUT);
  pinMode(PWR_IND, OUTPUT);

  //  setup serial
  SerialHWD.begin(115200);
  pinMode(PB12, INPUT_PULLUP);
  SPI.setMOSI(PB5);
  SPI.setMISO(PB4);
  SPI.setSCLK(PB3);
  
  mcp2515.reset();
  mcp2515.setPin(0b00001000);//turnRXbuf1 pin to GND
  mcp2515.setBitrate(CAN_33KBPS, MCP_8MHZ);
  
  // set filter for only PRND Messages and cruise control status messages
  mcp2515.setFilterMask(MCP2515::MASK0, true, 0x03FFE000);
  mcp2515.setFilterMask(MCP2515::MASK1, true, 0x03FFE000);
  mcp2515.setFilter(MCP2515::RXF0, true,      0x25<<13);
  mcp2515.setFilter(MCP2515::RXF1, true,      0x2f<<13);
  mcp2515.setFilter(MCP2515::RXF2, true,      0x0);
  mcp2515.setFilter(MCP2515::RXF3, true,      0x0);
  mcp2515.setFilter(MCP2515::RXF4, true,      0x0);
  mcp2515.setFilter(MCP2515::RXF5, true,      0x0);
  mcp2515.setNormalMode();

  SerialHWD.println("Setup Done!");

  // send preamble to look like the instrument cluster
  delay(100);
  sendPreamble();
}


__u32 getArbID(can_frame &frame)
{
  return (frame.can_id >> 13) & 0x1fff;
}

__u32 getECUID(can_frame &frame)
{
  return frame.can_id & 0x1fff;
}

void printCanWithARB(can_frame &canMSG)
{
  SerialHWD.printf("ID: %X", canMSG.can_id);
  SerialHWD.printf("\tArbID: %X", getArbID(canMSG));
  SerialHWD.print(" Data: ");
    
  for (int i = 0; i < canMSG.can_dlc; i++)  { // print the data
    SerialHWD.printf("%02X ", canMSG.data[i]);
  }
}

void printCan(can_frame &canMSG)
{
  SerialHWD.printf("ID: %X", canMSG.can_id);
  SerialHWD.print(" Data: ");
    
  for (int i = 0; i < canMSG.can_dlc; i++)  { // print the data
    SerialHWD.printf("%02X ", canMSG.data[i]);
  }
}

void printCanStruct(can_frame &canMSG)
{
  SerialHWD.printf("{ 0x%X, %d, {", canMSG.can_id, canMSG.can_dlc);
  for (int i = 0; i < canMSG.can_dlc; i++)  { // print the data
    SerialHWD.printf("0x%02X", canMSG.data[i]);
    if(i != canMSG.can_dlc -1)
    {
      SerialHWD.printf(", ");
    }
  }
  SerialHWD.printf("}},");
}

void handleCruiseStatus(can_frame &frame)
{
  if(frame.data[3] & 0x40)
  {
    digitalWrite(CRUISE_IND, HIGH);
  }
  else
  {
    digitalWrite(CRUISE_IND, LOW);
  }
}

void handleCruiseStatusLS(can_frame &frame)
{
  if((frame.data[1] & 0x0F) == 0x0D)
  {
    digitalWrite(CRUISE_IND, HIGH);
  }
  else
  {
    digitalWrite(CRUISE_IND, LOW);
  }
}

void handlePRND2LHS(can_frame &frame)
{
  switch(frame.data[3])
  {
    case 0x01: // park;
      digitalWrite(PARK_IND, HIGH);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x02: // reverse (BCM controls reverse lights)
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x03: // neutral
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, HIGH);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x04: // drive
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, HIGH);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x09: // 2nd
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, HIGH);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x08: // low
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, HIGH);
      break;
    
  }

  switch(frame.data[5])
  {
     case 0x04: //trailer
        digitalWrite(PWR_IND, HIGH);
        break;
     case 0x00: //regular
        digitalWrite(PWR_IND, LOW);
        break;
  }
}

void handlePRND2LLS(can_frame &frame)
{
  switch(frame.data[1])
  {
    case 0x10: // park;
      digitalWrite(PARK_IND, HIGH);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x20: // reverse (BCM controls reverse lights)
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x30: // neutral
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, HIGH);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x40: // drive
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, HIGH);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x50: // 2nd
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, HIGH);
      digitalWrite(LOW_IND, LOW);
      break;
    
    case 0x80: // low
      digitalWrite(PARK_IND, LOW);
      digitalWrite(NEUTRAL_IND, LOW);
      digitalWrite(DRIVE_IND, LOW);
      digitalWrite(SECOND_IND, LOW);
      digitalWrite(LOW_IND, HIGH);
      break; 
  }
  switch(frame.data[4])
  {
     case 0x20: //trailer
        digitalWrite(PWR_IND, HIGH);
        break;
     case 0x00: //regular
        digitalWrite(PWR_IND, LOW);
        break;
  }
}

void loop() {

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    __u32 arbID = getArbID(canMsg);
    if(arbID == 0x25)
    {
      handlePRND2LLS(canMsg);    
    }

    if(arbID == 0x2F)
    {
      handleCruiseStatusLS(canMsg);
    } 
  }

  MCP2515::ERROR err;
  
  //dash present messages
  if(millis() - timeSinceLastDashMessage > 1000)
  {
      err = mcp2515.sendMessage(&DASH_PRESENT);
      if(err != MCP2515::ERROR_OK)
      {
        SerialHWD.println("Failed to send dash present message!");
      }
      timeSinceLastDashMessage = millis();
      return;
  }

  if(millis() - timeSinceLastOtherDashMessage > 3000)
  {
      err = (MCP2515::ERROR)(mcp2515.sendMessage(&DASH_MESSAGE_1) | mcp2515.sendMessage(&DASH_MESSAGE_1));
      if(err != MCP2515::ERROR_OK)
      {
        SerialHWD.println("Failed to send other dash messages!");
      }
      timeSinceLastOtherDashMessage = millis();
      return;
  }
  
  //ac implementaiton
  boolean ac = pinVal < 500;
  if(millis() - timeSinceLastACMessage > 200 || ac != lastAC)
  {
    pinVal = analogRead(AC_IN);
    //SerialHWD.printf("AC_IN Value %d: ", pinVal);
    
    if(ac)
    {
      //err = mcp2515.sendMessage(MCP2515::TXB1, &AC_ON_MSG);
      //mcp2515.sendMessage(&AC_PRESENT);
      err = mcp2515.sendMessage(&AC_ON_MSG);
      if(err == MCP2515::ERROR_OK)
      {
        lastAC = ac;
      }
      else
      {
        SerialHWD.println("Failed to send AC ON Message!");
      }
    }
    else
    {
      //mcp2515.sendMessage(&AC_PRESENT);
      err = mcp2515.sendMessage(&AC_OFF_MSG);
      if(err == MCP2515::ERROR_OK)
      {
        lastAC = ac;
        //SerialHWD.println("Successfully sent AC OFF Message!");
      }
      else
      {
        SerialHWD.println("Failed to send AC OFF Message!");
      } 
    }
    timeSinceLastACMessage = millis();

    return;
  }

}
