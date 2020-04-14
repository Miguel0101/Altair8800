//VT100 Emulator

void clrscr()
{
  Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);
  Serial.print("[H");     // cursor to home command
}

void clrlin()
{
  Serial.write(27);       // ESC command
  Serial.print("[2K");    // clear current line
}

void clrend()
{
  Serial.write(27);       // ESC command
  Serial.print("[K");  
}


void clrarea()
{
  Serial.write(27);       // ESC command
  Serial.print("[J");    // clear end screen
}

void savecur()
{
  Serial.write(27);       // ESC command
  Serial.print("[s");    // cursor save
}

void loadcur()
{
  Serial.write(27);       // ESC command
  Serial.print("[u");    // cursor restore
}

void color(uint8_t clr)
{
  Serial.write(27);       // ESC command
  Serial.print("[");    //
  switch (clr) {
    case 0: Serial.print("30");//
      break;
    case 1: Serial.print("31");//
      break;
    case 2: Serial.print("32");//
      break;
    case 3: Serial.print("33");//
      break;
    case 4: Serial.print("34");//
      break;
    case 5: Serial.print("35");//
      break;
    case 6: Serial.print("36");//
      break;
    case 7: Serial.print("37");//
      break;
    case 8: Serial.print("38");//
      break;
    case 9: Serial.print("39");//
      break;
    case 14: Serial.print("44");//
      break;
    case 17: Serial.print("47");//
      break;
    case 19: Serial.print("49");//
      break;
    case 21: Serial.print("21");//
      break;
  }
  Serial.print("m");    //
}

void cursor(uint8_t row, uint8_t col) {
  Serial.write(27);       // ESC command
  Serial.print("[");
  Serial.print(row, DEC);
  Serial.print(";");    // clear screen command
  Serial.print(col, DEC);
  Serial.print("H");    // clear screen command

}
