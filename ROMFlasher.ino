#define CLOCK_PIN 2
#define LATCH_PIN 3
#define DATA_PIN 4

#define IO_0 5
#define IO_1 6
#define IO_2 7
#define IO_3 8
#define IO_4 9
#define IO_5 10
#define IO_6 11
#define IO_7 12
#define WE_PIN 13

#define EEPROM_SIZE 8192

//todo use hexadec instead of pure ascii
bool oe;
void setup() {
  digitalWrite(WE_PIN, 1);
  pinMode(WE_PIN, OUTPUT);
  digitalWrite(WE_PIN, 1);
  setOE(false);

  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  setPinMode(INPUT);


  Serial.begin(115200);
  Serial.setTimeout(2);
  Serial.println("Ready");

}
bool wholeMode;
char buff[100];
uint16_t buffIndex;
bool superWrite;
void loop() {
  setPinMode(INPUT);
  setOE(false);
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'o') {
      buffIndex = 0;
    } else if (c == 'p') {
      char buffi[buffIndex + 2];
      for (int i = 0; i < buffIndex + 1; i++) {
        buffi[i] = buff[i];
      }
      buffi[buffIndex + 1] = '\0';
      loo(String(buffi));
      buffIndex = 0;
    } else {
      buff[buffIndex] = c;
      buffIndex++;
      if (buffIndex == 100) {
        buffIndex = 0;
      }
    }
  }
}
uint16_t currentAddress;
void loo(String s) {
  if (s.charAt(0) == 'b') {
    Serial.println(toHex(s.substring(1)));
  }
  else if (s.charAt(0) == 's') {
    Serial.print('s');
    Serial.println("8192");
  }
  else if (s.charAt(0) == 'r') {
    if (s.length() < 5) {
      Serial.println("not five chars");
      return;
    }
    uint8_t msb = toHex(s.substring(1, 3));
    uint8_t lsb = toHex(s.substring(3, 5));
    setOE(true);
    setAddress(msb, lsb);
    Serial.print('r');
    Serial.println(String((int)readInput(), HEX));
  }
  else if (s.charAt(0) == 't') {
    setOE(true);
    Serial.println('a');
    for (uint16_t i = 0; i < EEPROM_SIZE; i++) {
      uint8_t msb = (uint8_t)((i >> 8) & 255);
      uint8_t lsb = (uint8_t)((i) & 255);
      setAddress(msb, lsb);
      Serial.print("t");
      Serial.println(String((int)readInput(), HEX));
    }
    Serial.println("ty");
  }
  else if (s.charAt(0) == 'w') {
    if (s.length() < 7) {
      Serial.println('x');
      return;
    }
    for (int i = 0; i < s.length(); i++) {
      if (s.charAt(i) == 'w') {
        uint8_t msb = toHex(s.substring(i+1, i+3));
        uint8_t lsb = toHex(s.substring(i+3, i+5));
        uint8_t val = toHex(s.substring(i+5, i+7));
        setAddress(msb, lsb);
        setOutput(val);
        delayMicroseconds(200);
        WE(true);
        delayMicroseconds(300);
        WE(false);
        delayMicroseconds(200);
        setOE(true);
        if (readInput() != val) {
          Serial.print("nn");
        } else
          Serial.print('y');
      }
    }
    Serial.println();
    

  }
  else if (s.startsWith("u")) {
    superWrite = true;
    currentAddress = 0;
    Serial.println('y');

  }
  else if (s.startsWith("cccccc")) {//OE has to be conneted to 12V
    WE(true);
    delay(10);
    WE(false);
    Serial.println('y');
  }
  else Serial.println('n');

}

void WE(bool enable) {
  digitalWrite(WE_PIN, !enable);
}

uint8_t lastMSB;
uint8_t lastLSB;
void setAddress(uint8_t msb, uint8_t lsb) {
  digitalWrite(LATCH_PIN, 0);
  if (oe) {
    msb &= (~(1 << 7));
  }
  else msb |= (1 << 7);
  lastMSB = msb;
  lastLSB = lsb;
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, msb);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, lsb);
  digitalWrite(LATCH_PIN, 1);
}
void setOE(bool b) {
  setPinMode(b ? INPUT : OUTPUT);
  oe = b;
  setAddress(lastMSB, lastLSB);
}
void setOutput(uint8_t value) {
  setPinMode(OUTPUT);
  digitalWrite(IO_0, (value & (1 << 0)) != 0);
  digitalWrite(IO_1, (value & (1 << 1)) != 0);
  digitalWrite(IO_2, (value & (1 << 2)) != 0);
  digitalWrite(IO_3, (value & (1 << 3)) != 0);
  digitalWrite(IO_4, (value & (1 << 4)) != 0);
  digitalWrite(IO_5, (value & (1 << 5)) != 0);
  digitalWrite(IO_6, (value & (1 << 6)) != 0);
  digitalWrite(IO_7, (value & (1 << 7)) != 0);
}
uint8_t readInput() {
  setPinMode(INPUT);
  uint8_t out = 0;
  if (digitalRead(IO_0))
    out |= (1 << 0);
  if (digitalRead(IO_1))
    out |= (1 << 1);
  if (digitalRead(IO_2))
    out |= (1 << 2);
  if (digitalRead(IO_3))
    out |= (1 << 3);
  if (digitalRead(IO_4))
    out |= (1 << 4);
  if (digitalRead(IO_5))
    out |= (1 << 5);
  if (digitalRead(IO_6))
    out |= (1 << 6);
  if (digitalRead(IO_7))
    out |= (1 << 7);
  return out;
}

void setPinMode(bool out) {
  pinMode(IO_0, out);
  pinMode(IO_1, out);
  pinMode(IO_2, out);
  pinMode(IO_3, out);
  pinMode(IO_4, out);
  pinMode(IO_5, out);
  pinMode(IO_6, out);
  pinMode(IO_7, out);
}
uint16_t toHex(String s) {
  s += " ";
  char buf[s.length()];
  s.toCharArray(buf, s.length());
  return (uint16_t) strtol(buf, 0, 16);
}





















