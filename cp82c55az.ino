// Port test program for Intersil CP82C55AZ peripheral interface chip
// Board: Arduino ATmega 2560
const int dataPins[] = {22, 24, 26, 28, 30, 32, 34, 36}; // D0 - D7
const int portPins[] = {23, 25, 27, 29, 31, 33, 35, 37}; // Px0 - Px7
const int addressPins[] = {53, 51}; // A0, A1
const int chipSelectPin = 49; // CS
const int readPin = 47; // RD
const int writePin = 46; // WR
const int resetPin = 48; // RESET

const int portNum = 0; // 0 = A, 1 = B, 2 = C, 3 - control word

const uint8_t dataPatterns[] = {
  0x55, 0xAA, 0x00, 0xFF,
  0x01, 0x02, 0x04, 0x08,
  0x10, 0x20, 0x40, 0x80,
  ~0x01, ~0x02, ~0x04, ~0x08,
  ~0x10, ~0x20, ~0x40, ~0x80
};

void setup() {
  Serial.begin(9600);
  Serial.println("Setting up.");

  for (int i = 0; i < 2; i++) {
    pinMode(addressPins[i], OUTPUT);
  }

  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], OUTPUT);
  }

  pinMode(chipSelectPin, OUTPUT);
  pinMode(readPin, OUTPUT);
  pinMode(writePin, OUTPUT);
  pinMode(resetPin, OUTPUT);

  digitalWrite(chipSelectPin, HIGH);
  digitalWrite(writePin, HIGH);
  digitalWrite(readPin, HIGH);
  digitalWrite(resetPin, LOW);
  delay(1);

  // write control word
  writeBits(addressPins, 2, 3); // set address to control word
  writeBits(dataPins, 8, B10000000); // set control word to all ports output

  digitalWrite(chipSelectPin, LOW);
  digitalWrite(writePin, LOW);
  delay(1);

  digitalWrite(chipSelectPin, HIGH);
  digitalWrite(writePin, HIGH);
  writeBits(addressPins, 2, portNum); // set address to port
  delay(1);

  Serial.println("Ready.");
}

void writeBits(const int *pins, const int pinCount, const uint8_t data) {
  for (int i = 0; i < pinCount; i++) {
    digitalWrite(pins[i], data & 1 << i);
  }
}

uint8_t readBits(const int *pins, const int pinCount) {
  uint8_t res = 0;
  for (int i = 0; i < pinCount; i++) {
    res |= digitalRead(pins[i]) << i;
  }

  return res;
}

void loop() {
  int passCount = 0, failCount = 0;

  // output loopback test
  for (int m = 0; m < 2; m++) {
    const int *readPins = (m == 0 ? portPins : dataPins);
    const int *writePins = (m == 0 ? dataPins : portPins);

    if (m == 1) {
      // write control word
      writeBits(addressPins, 2, 3); // set address to control word
      writeBits(dataPins, 8, B10011011); // set control word to all ports input
    
      digitalWrite(chipSelectPin, LOW);
      digitalWrite(writePin, LOW);
      delay(1);

      digitalWrite(chipSelectPin, HIGH);
      digitalWrite(writePin, HIGH);
      writeBits(addressPins, 2, portNum); // set address to port
    }

    Serial.print("Read pins: ");
    for (int i = 0; i < 8; i++) {
      Serial.print(readPins[i]);
      if (i == 7)
        Serial.println("");
      else
        Serial.print(", ");
    }
    Serial.print("Write pins: ");
    for (int i = 0; i < 8; i++) {
      Serial.print(writePins[i]);
      if (i == 7)
        Serial.println("");
      else
        Serial.print(", ");
    }

    // set pin mode
    for (int i = 0; i < 8; i++) {
      pinMode(writePins[i], OUTPUT);
      pinMode(readPins[i], INPUT);
    }

    for (int i = 0; i < sizeof (dataPatterns) / sizeof (dataPatterns[0]); i++) {
      Serial.print("[W] ");
      Serial.println(dataPatterns[i], BIN);

      // write data pattern
      writeBits(writePins, 8, dataPatterns[i]);

      digitalWrite(chipSelectPin, LOW);
      digitalWrite(writePin, LOW);
      delay(1);

      digitalWrite(writePin, HIGH);
      digitalWrite(readPin, LOW);
      delay(1);

      // read back data pattern
      uint8_t value = readBits(readPins, 8);

      digitalWrite(chipSelectPin, HIGH);
      digitalWrite(readPin, HIGH);
      delay(1);

      Serial.print("[R] ");
      Serial.print(value, BIN);
      if (value == dataPatterns[i]) {
        Serial.println(" PASS");
        passCount++;
      } else {
        Serial.println(" FAIL");
        failCount++;
      }
    }
  }

  Serial.println("End of test.");
  Serial.print(passCount);
  Serial.print(" tests passed, ");
  Serial.print(failCount);
  Serial.println(" tests failed.");
  while (1)
  {
    delay(100);
  }
}
