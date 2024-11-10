#include <FFat.h>  

typedef enum {
  EXAM_READING,
  NONE
} SerialState;

SerialState serialState = NONE;

void writeToFile(const String &filename, const char *data, size_t len) {
  uint8_t data_u[600];
  for (int i = 0; i < len; i++)
  {
    data_u[i] = uint8_t(data[i]);
  }
  File uploadFile = FFat.open("/" + filename, FILE_APPEND);
  uploadFile.write(data_u, len);
  uploadFile.close();
  
  Serial.printf("Upload Complete: %s\n", filename.c_str());
}

void parse_serial() {
  if(Serial.available() > 0) {
    String message = "";
    while(Serial.available() > 0) {
      char incomingByte = Serial.read();
      message += incomingByte;
    }

    if (message.indexOf("EXSTART") >= 0) {
      serialState = EXAM_READING;
      File uploadFile = FFat.open("/exam.zip", FILE_WRITE, true);
      uploadFile.close();

    } else if(message.indexOf("EXEND") >= 0) {
      serialState = NONE;
    } else if (serialState == EXAM_READING) {
      writeToFile("exam.zip", message.c_str(), message.length());
    }
  }
}

void sendProcessFile(String proc, String s_id) {
  Serial.println("PSTART;" + s_id);
  delay(2000);
  Serial.println(proc);
  delay(2000);
  Serial.println("PEND");
}

void sendScreenshot(String filename, String s_id) {

  if (!FFat.exists(filename)) {
    Serial.println("File not found!");
    return;
  }
  Serial.println("File found, sending data...");

  File file = FFat.open(filename, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file!");
    return;
  }

  String scstart = "SCSTART;" + s_id + "\r\n";
  Serial.write(scstart.c_str());
  sendFile(file);
  Serial.write("SCEND\r\n");

  file.close();
  Serial.println("File sent successfully.");
}

void sendExamSubmission(String code, String s_id) {
  Serial.println("EXSUB;" + s_id);
  delay(2000);
  Serial.println(code);
  delay(2000);
  Serial.println("EXFIN");
}

void sendFile(File &file) {
  byte buffer[128];

  while (file.available()) {
    size_t bytesRead = file.read(buffer, sizeof(buffer));
    Serial.write(buffer, bytesRead);
    delay(10);
  }
}
