//Setup
const byte SD_SELECT = 8;
const byte pinPower = 9;

#define ERR "[\033[1;31mFAILED\033[0m] "
#define OK "[\033[1;32mOK\033[0m] "

void PERIPHERALS() {
  color(9);
  clrscr();
  tone(4, 1010);
  delay(500);
  noTone(4);
  Serial.println("PC-XT BIOS VER: 8.6 © Miguel Magno");
  Serial.println("----------------------------------");
  cursor(4,0);
  Serial.println("Status:");
  cursor(6,0);
  delay(2000);
  if (!SD.begin(SD_SELECT)) {
      Serial.print(ERR);
      Serial.println("SD Card Not Found!");
      delay(3000);
      sys_reset();
   } else {
      Serial.print(OK);
      Serial.println("SD Card Found!");
   }
   delay(1000);
   SPI.begin();
   delay(1000);
   color(9);
   Serial.println("BOOTING...");
   delay(5000);
   clrscr();
}
