/*
  Read Detected Ojbects from SingTown AI Cam

 The circuit:
 * Arduino pin 10 connect to SingTown AI Cam TX Pin
 * Arduioo GND connect to SingTown AI Cam GND Pin
 * Arduino 5V connect to SingTown AI Cam VIN Pin (option)

 */
#include <SoftwareSerial.h>

SoftwareSerial camSerial(10, -1);  // RX, TX

#define OBJECT_MAX 8    // Max Found 8 Objects
#define OBJECT_SIZE 10  // Every Object is 10 bytes

struct DetectObj {
  int score;  // max 255
  int idx;
  int x1;  // box left coordinate, max 640
  int y1;  // box top coordinate, max 480
  int x2;  // box right coordinate, max 640
  int y2;  // box bottom coordinate, max 480
};

struct DetectObj objs[OBJECT_MAX];

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
  for (i = 0; i < num * OBJECT_SIZE + 2; i++) {
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
  unsigned char payload[OBJECT_MAX * OBJECT_SIZE + 2];
  unsigned char* obj_ptr;
  int byte;
  while (1) {
    if (readByte() != 0xeb) continue;
    if (readByte() != 0x90) continue;

    num = readByte();
    if (num < 0 || num > OBJECT_MAX)
      continue;
    payload[0] = num;

    for (i = 1; i < num * OBJECT_SIZE + 2; i++) {
      byte = readByte();
      if (byte == -1) break;
      payload[i] = byte;
    }

    if (check_crc(payload) != 0) continue;

    for (i = 0; i < num; i++) {
      obj_ptr = payload + i * OBJECT_SIZE + 1;
      objs[i].score = obj_ptr[0];
      objs[i].idx = obj_ptr[1];
      objs[i].x1 = obj_ptr[2] | (obj_ptr[3] << 8);
      objs[i].y1 = obj_ptr[4] | (obj_ptr[5] << 8);
      objs[i].x2 = obj_ptr[6] | (obj_ptr[7] << 8);
      objs[i].y2 = obj_ptr[8] | (obj_ptr[9] << 8);
    }
    return num;
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // set the data rate for the SoftwareSerial port
  camSerial.begin(9600);
}

void loop() {  // run over and over
  int num;
  int i;
  if (camSerial.available()) {
    num = read_singtownaicam_objs();
    Serial.print("Found Object Number: ");
    Serial.println(num);

    for (i = 0; i < num; i++) {
      Serial.print('\t');
      Serial.print(objs[i].score);
      Serial.print("\t");
      Serial.print(objs[i].idx);
      Serial.print("\t[");
      Serial.print(objs[i].x1);
      Serial.print(",\t");
      Serial.print(objs[i].y1);
      Serial.print(",\t");
      Serial.print(objs[i].x2);
      Serial.print(",\t");
      Serial.print(objs[i].y2);
      Serial.print("]\n");
    }
  }
}
