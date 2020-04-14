/*
Intel 8080 Altair 8800 Emulator
Copyright © Miguel Magno

Tuesday, April 14, 2020
*/

extern "C" {
  #include "i8080.h"
}

//reset function
void(* sys_reset) (void) = 0;

#include "SPI.h"
#include "SD.h"
#include "disk.h"
#include "ESC.h"
#include "Setup.h"
#define Filename1 "cpm63k.dsk"
#define Filename2 "cpm3.dsk"
#define Filename3 "zork.dsk"
#define Filename4 "games.dsk"
#include <PS2Keyboard.h>

const int DataPin = 3;
const int IRQpin =  2;

PS2Keyboard keyboard;
const byte RAM_SELECT = 10;

enum State { 
  HLDA, WAIT, WO /*was INTE*/, STACK /*PROT*/, MEMR, INP, 
  M1, OUT, HLTA, PROT /*was STACK*/, INTE /*was WO*/, INT };

struct {
  int address;
  byte data;
  byte state;
} bus;

byte Console_Input() {
  return keyboard.available() ? keyboard.read() : 0; 
}

byte Console_Output(char c) {
  Serial.write(c & 0x7f);
}

int input(int port) {
  bitSet(bus.state,INP);
  
  static uint8_t character = 0;

  switch (port) {
    case 0x00:
      return 0;
    case 0x01: //serial read
      return Console_Input();
    case 0x08: 
      return disk_status();
    case 0x09:
      return disk_sector();
    case 0x0a: 
      return disk_read();
    case 0x10: //2SIO port 1 status
      if (!character) {
        character = Console_Input();
      }
      return (character ? 0b11 : 0b10); 
    case 0x11: //2SIO port 1, read
      if (character) {
        int tmp = character; 
        character = 0; 
        return tmp; 
      } else {
        return Console_Input();
      }
    case 0x12: // ???
      break;
    default:
      Serial.println(port);
      while(1);
  }
  return 0xff;
}

void output(int port, byte value) {
  bitSet(bus.state,OUT);

  switch (port) {
    case 0x01: 
      //Serial.print((char)(value & 0x7f));
      Console_Output(value);
      break;
    case 0x08:
      disk_select(value);
      break;
    case 0x09:
      disk_function(value);
      break;
    case 0x0a:
      disk_write(value);
      break;
    case 0x10: // 2SIO Controller
      // nothing
      break;
    case 0x11: // Console_Out
      Console_Output(value);
      break;
    case 0x12: // PC Speaker
      tone(4, value);
      break;
    default:
      Serial.println(port);
      while(1);
      break;
  }
}

byte readByte(int address) {
  digitalWrite(RAM_SELECT, LOW);
  PORTB &= ~B0100;
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(3); //read
  SPI.transfer((address >> 16) & 255);
  SPI.transfer((address >> 8) & 255);
  SPI.transfer((address) & 255);
  byte value = SPI.transfer(0x00);
  SPI.endTransaction();
  PORTB |= B0100;
  digitalWrite(RAM_SELECT, HIGH);
  bus.data = value;
  bus.address = address;
  bitSet(bus.state, MEMR);
  if (address==i8080_regs_sp()) bitSet(bus.state, STACK);
  return value;
}

void writeByte(int address, byte value) {
  digitalWrite(RAM_SELECT, LOW);
  PORTB &= ~B0100;
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(2); //write
  SPI.transfer((address >> 16) & 255);
  SPI.transfer((address >> 8) & 255);
  SPI.transfer((address) & 255);
  SPI.transfer(value);
  SPI.endTransaction();
  digitalWrite(RAM_SELECT, HIGH);
  PORTB |= B0100;
  bus.data = value;
  bus.address = address;
  bitClear(bus.state,WO); //inverted logic for write LED
  if (address==i8080_regs_sp()) bitSet(bus.state, STACK);
}

int readWord(int address) {
  return readByte(address) | (readByte(address + 1) << 8);
}

void writeWord(int address, int value) {
  writeByte(address, value & 0xff);
  writeByte(address + 1, (value >> 8) & 0xff);
}

extern "C" {
      
  //read/write byte
  int i8080_hal_memory_read_byte(int addr) {
    return readByte(addr);
  }
  
  void i8080_hal_memory_write_byte(int addr, int value) {
    writeByte(addr,value);
  }
  
  //read/write word
  int i8080_hal_memory_read_word(int addr) {
    return readWord(addr);
  }
  
  void i8080_hal_memory_write_word(int addr, int value) {
    writeWord(addr,value);
  }
  
  //input/output
  int i8080_hal_io_input(int port) {
    return input(port);
  }
  
  void i8080_hal_io_output(int port, int value) {
    output(port,value);
  }
  
  //interrupts
  void i8080_hal_iff(int on) {
    //nothing
  }
}

int loadFile(const char filename[], int offset = 0) {
  File file = SD.open(filename);
  if (!file) {
    Serial.print(ERR);
    Serial.println("Load!");
    return -2; 
    delay(3000);
    sys_reset();
  }
  while (file.available()) {
    writeByte(offset++, file.read());
  }
  file.close();
  return 0;
}

void loadFloppy1() {
  loadFile("88DSKROM.BIN", 0xff00);
  disk_drive.disk1.fp = SD.open("cpm63k.dsk");
  disk_drive.disk2.fp = SD.open("games.dsk");
  if(!disk_drive.disk2.fp || !disk_drive.disk1.fp)
    Serial.print(ERR);
    Serial.println("DSK!");
    delay(3000);
    clrscr();
    sys_reset();
  disk_drive.nodisk.status = 0xff;
  examine(0xff00);
}

void loadFloppy2() {
  loadFile("88DSKROM.BIN", 0xff00);
  disk_drive.disk1.fp = SD.open("cpm3.dsk");
  disk_drive.disk2.fp = SD.open("games.dsk");
  if(!disk_drive.disk2.fp || !disk_drive.disk1.fp)
    Serial.print(ERR);
    Serial.println("DSK!");
    delay(3000);
    clrscr();
    sys_reset();
  disk_drive.nodisk.status = 0xff;
  examine(0xff00);
}

void run() {
  bitClear(bus.state,WAIT);
}

void stop() {
  bitSet(bus.state,WAIT);
}

void examine(int address) {
  i8080_jump(address); //set program counter
  readByte(address);
}

void deposit(int address, byte data) {
  i8080_jump(address); //set program counter
  writeByte(address,data);
}

//Setup
void setup() {
  pinMode(RAM_SELECT, OUTPUT);
  pinMode(pinPower, INPUT_PULLUP);
  Serial.begin(9600);
  
  power:
  if (digitalRead(pinPower) == 0) {
  keyboard.begin(DataPin, IRQpin, PS2Keymap_US);
  PERIPHERALS();
  bus.state = 0x2; //all flags off except STOP
  bus.address = 0;
  bus.data = 0;
  i8080_init();
  examine(0);
  loader();
  }else {
    clrscr();
    delay(100);
    goto power;
  }
}

//Loop
void loop() {
  bitClear(bus.state,MEMR); //flag set by readByte()
  bitClear(bus.state,M1);  //flag set by step()
  bitClear(bus.state,OUT); //flag set by output()
  bitClear(bus.state,INP); //flag set by input()
  bitSet(bus.state,WO); //flag CLEARED by writeByte() inverted logic
  bitClear(bus.state, STACK); //set by readByte and writeByte if addr==SP

  if (!bitRead(bus.state,WAIT)) {
    for (int i=0; i < 50; i++) 
      i8080_instruction();
  }

  if (digitalRead(pinPower) == 0) {
      stop();
      clrscr();
      Serial.println("Shutting Down...");
      delay(4000);
      sys_reset();
  }
}
void loader() {
    Serial.print("1. Boot: ");
    Serial.println(Filename1);
    Serial.print("2. Boot: ");
    Serial.println(Filename2);
  
  kb:
  if (keyboard.available()) {
    switch(keyboard.read()) {
      
      case '1':
      clrscr();
      loadFloppy1();
      run();
      break;

      case '2':
      clrscr();
      loadFloppy2();
      run();
      break;
    }
  }else {
      goto kb;
    }
  }
