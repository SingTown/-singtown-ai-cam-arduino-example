/*
  Read Classify Score List from SingTown AI Cam

 The circuit:
 * Arduino pin 10 connect to SingTown AI Cam TX Pin
 * Arduioo GND connect to SingTown AI Cam GND Pin
 * Arduino 5V connect to SingTown AI Cam VIN Pin (option)

 */
#include <SoftwareSerial.h>

SoftwareSerial camSerial(10, -1);  // RX, TX

#define CLASSES_MAX 64  // Max 64 Classes, If you know the exact number, you can change smaller.

uint8_t cls[CLASSES_MAX];

int readByte() {
  int timeout = 2;
  while (timeout--) {
    if (camSerial.available())
      return camSerial.read();
    else
      delay(1);
  }
  return -1;
}

int check_crc(unsigned char* payload) {
  unsigned char num = payload[0];
  unsigned char crc = 0;
  int i = 0, j = 0;
  for (i = 0; i < num + 2; i++) {
    crc ^= payload[i];
    for (j = 0; j < 8; j++) {
      if (crc & 1)
        crc ^= 0x91;
      crc >>= 1;
    }
  }
  return crc;
}

int read_singtownaicam_objs() {
  int i;
  unsigned char num;
  unsigned char payload[CLASSES_MAX + 2];
  unsigned char* obj_ptr;
  int byte;
  while (1) {
    if (readByte() != 0xeb) continue;
    if (readByte() != 0x90) continue;

    num = readByte();
    if (num < 0 || num > CLASSES_MAX)
      continue;
    payload[0] = num;

    for (i = 1; i < num + 2; i++) {
      byte = readByte();
      if (byte == -1) break;
      payload[i] = byte;
    }

    if (check_crc(payload) != 0) continue;

    memcpy(cls, &payload[1], num);
    return num;
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // set the data rate for the SoftwareSerial port
  camSerial.begin(115200);
}

void loop() {  // run over and over
  int num;
  int i;
  if (camSerial.available()) {
    num = read_singtownaicam_objs();
    Serial.print("Classify Number: ");
    Serial.print(num);

    Serial.print("\t[");
    for (i = 0; i < num; i++) {
      Serial.print('\t');
      Serial.print(cls[i]);
      Serial.print(",");
    }
    Serial.println("]\n");
  }
}
