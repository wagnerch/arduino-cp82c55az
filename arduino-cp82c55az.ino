// Port test program for Intersil CP82C55AZ peripheral interface chip
// Board: Arduino ATmega 2560
const int dataPins[] = {22, 24, 26, 28, 30, 32, 34, 36}; // D0 - D7
const int portPins[] = {23, 25, 27, 29, 31, 33, 35, 37}; // Px0 - Px7
const int addressPins[] = {53, 51}; // A0, A1
const int chipSelectPin = 49; // CS
const int readPin = 47; // RD
const int writePin = 46; // WR

const int portNum = 0; // 0 = A, 1 = B, 2 = C, 3 - control word

struct {
  const uint8_t controlWord;
  const int *readPins;
  const int *writePins;
} portModes[] = {
  { B10000000, portPins, dataPins }, // OUTPUT
  { B10011011, dataPins, portPins }  // INPUT
};

const uint8_t dataPatterns[] = {
  0x55, 0xAA, 0x00, 0xFF,
  0x01, 0x02, 0x04, 0x08,
  0x10, 0x20, 0x40, 0x80,
  (uint8_t)~0x01, (uint8_t)~0x02, (uint8_t)~0x04, (uint8_t)~0x08,
  (uint8_t)~0x10, (uint8_t)~0x20, (uint8_t)~0x40, (uint8_t)~0x80
};

void setup() {
  Serial.begin(9600);
}

void pinModeArray(const int *pins, const int pinCount, int mode) {
  for (int i = 0; i < pinCount; i++) {
    pinMode(pins[i], mode);
  }
}

void digitalWriteArray(const int *pins, const int pinCount, const uint8_t data) {
  for (int i = 0; i < pinCount; i++) {
    digitalWrite(pins[i], data & 1 << i);
  }
}

uint8_t digitalReadArray(const int *pins, const int pinCount) {
  uint8_t res = 0;
  for (int i = 0; i < pinCount; i++) {
    res |= digitalRead(pins[i]) << i;
  }

  return res;
}

void loop() {
  int passCount = 0, failCount = 0;
  int bitHistogram[] = {0, 0, 0, 0, 0, 0, 0, 0};

  pinModeArray(addressPins, 2, OUTPUT);
  pinModeArray(dataPins, 8, OUTPUT);

  pinMode(chipSelectPin, OUTPUT);
  pinMode(readPin, OUTPUT);
  pinMode(writePin, OUTPUT);

  digitalWrite(chipSelectPin, HIGH);
  digitalWrite(writePin, HIGH);
  digitalWrite(readPin, HIGH);
  delay(1);

  // input/output loopback test
  for (int m = 0; m < 2; m++) {
    // write control word
    digitalWriteArray(addressPins, 2, 3); // set address to control word
    digitalWriteArray(dataPins, 8, portModes[m].controlWord); // set control word
    
    digitalWrite(chipSelectPin, LOW);
    digitalWrite(writePin, LOW);
    delay(1);

    digitalWrite(chipSelectPin, HIGH);
    digitalWrite(writePin, HIGH);
    digitalWriteArray(addressPins, 2, portNum); // set address to port

    // set pin mode
    pinModeArray(portModes[m].writePins, 8, OUTPUT);
    pinModeArray(portModes[m].readPins, 8, INPUT);

    // log read/write pins
    Serial.print("Read pins: ");
    for (int i = 0; i < 8; i++) {
      Serial.print(portModes[m].readPins[i]);
      if (i == 7)
        Serial.println("");
      else
        Serial.print(", ");
    }
    Serial.print("Write pins: ");
    for (int i = 0; i < 8; i++) {
      Serial.print(portModes[m].writePins[i]);
      if (i == 7)
        Serial.println("");
      else
        Serial.print(", ");
    }

    // write data patterns
    for (int i = 0; i < sizeof (dataPatterns) / sizeof (dataPatterns[0]); i++) {
      // print write result
      Serial.print("[W] ");
      Serial.println(dataPatterns[i], BIN);

      // write data pattern
      digitalWriteArray(portModes[m].writePins, 8, dataPatterns[i]);

      digitalWrite(chipSelectPin, LOW);
      digitalWrite(writePin, LOW);
      delay(1);
      digitalWrite(writePin, HIGH);


      // read data pattern
      digitalWrite(readPin, LOW);
      delay(1);

      // read back data pattern
      uint8_t value = digitalReadArray(portModes[m].readPins, 8);

      digitalWrite(chipSelectPin, HIGH);
      digitalWrite(readPin, HIGH);
      delay(1);

      // print read result
      Serial.print("[R] ");
      Serial.print(value, BIN);

      // log pass/file on read result
      if (value == dataPatterns[i]) {
        Serial.println(" PASS");
        passCount++;
      } else {
        Serial.print(" FAIL");
        for (int j = 0; j < 8; j++) {
          uint8_t bitMask = 1 << j;
          if ((value & bitMask) != (dataPatterns[i] & bitMask)) {
            bitHistogram[j]++;
            Serial.print(" ");
            Serial.print(j);
          }
        }
        Serial.println("");
        failCount++;
      }
    } // data patterns
  } // port mode

  // write out test summary
  Serial.println("End of test.");
  Serial.print(passCount);
  Serial.print(" tests passed, ");
  Serial.print(failCount);
  Serial.println(" tests failed.");
  if (failCount > 0) {
    Serial.print("Failed bits:");
    for (int i = 0; i < 8; i++) {
      if (bitHistogram[i] > 0) {
        Serial.print(" ");
        Serial.print(i);
      }
    }
    Serial.println("");
  }

  // set pins back to input
  pinModeArray(addressPins, 2, INPUT);
  pinModeArray(dataPins, 8, INPUT);
  pinModeArray(portPins, 8, INPUT);

  pinMode(chipSelectPin, INPUT);
  pinMode(readPin, INPUT);
  pinMode(writePin, INPUT);

  while (1)
  {
    delay(100);
  }
}
