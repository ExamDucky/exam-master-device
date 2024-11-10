#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FFat.h>

const char *ssid = "test-ssid";
const char *password = "test-password";
AsyncWebServer server(80);


String urlDecode(const String &str) {
  String decoded = "";
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    
    if (c == '%') {
      if (i + 2 < str.length()) {
        char hex1 = str.charAt(i + 1);
        char hex2 = str.charAt(i + 2);
        int value = (hexToDec(hex1) << 4) + hexToDec(hex2);
        decoded += (char)value;
        i += 2;
      }
    } 
    else if (c == '+') {
      decoded += ' ';
    } 
    else {
      decoded += c;
    }
  }
  return decoded;
}

int hexToDec(char hex) {
  if (hex >= '0' && hex <= '9') {
    return hex - '0';
  } 
  else if (hex >= 'A' && hex <= 'F') {
    return hex - 'A' + 10;
  }
  return 0;
}

void handleFileUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.printf("Upload Start: %s\n", filename.c_str());
    FFat.remove("/" + filename);
  }
  File uploadFile = FFat.open("/" + filename, FILE_APPEND);
  if (uploadFile) {
    uploadFile.write(data, len);
    uploadFile.close();
  }
  if (final) {
    Serial.printf("Upload Complete: %s\n", filename.c_str());
  }
}

void ap_setup() {
  Serial.begin(115200);

  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  if (!FFat.begin(true)) {
    Serial.println("FFat Mount Failed");
    return;
  }

  // server.onFileUpload(handleFileUpload);

  server.serveStatic("/", FFat, "/");
  // server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   String fileName = request->arg("file");
  //   if (FFat.exists("/" + fileName)) {
  //     File downloadFile = FFat.open("/" + fileName, "r");
  //     request->send(downloadFile, fileName, "application/octet-stream");
  //     downloadFile.close();
  //   } else {
  //     request->send(404, "text/plain", "File not found");
  //   }
  // });

  server.on("/submit_exam", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        if (request->hasParam("code", true)) {
            message = request->getParam("code", true)->value();
            sendExamSubmission(message, request->getParam("s_id", true)->value());
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "OK");
    });

  server.on("/exists", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileName = urlDecode(request->arg("file"));
    if (FFat.exists("/" + fileName)) {
      request->send(200, "text/plain", "File found");
    } else {
      request->send(404, "text/plain", "File not found");
    }
  });

  server.on("/proc", HTTP_GET, [](AsyncWebServerRequest *request) {
    String student = urlDecode(request->arg("s_id"));
    sendProcessFile("processes_" + student + ".txt", student);
    sendScreenshot("scrt_" + student + ".png", student);
    request->send(200, "text/plain", "Processes uploaded successfully");
  });

  server.on("/login", [](AsyncWebServerRequest *request) {
    String student = urlDecode(request->arg("s_id"));
    String studentName = urlDecode(request->arg("s_name"));
    String mac = urlDecode(request->arg("mac"));
    Serial.println("STUDENT_ID;" + student);
    Serial.println("STUDENT_NAME;" + student + ";" + studentName);
    Serial.println("MAC;" + student + ";" + mac);
    request->send(200, "text/plain", "Student login successful");
  });

  server.on("/submit", [](AsyncWebServerRequest *request) {
    String student = urlDecode(request->arg("s_id"));
    sendExamSubmission("exam_" + student + ".zip", student);
    request->send(200, "text/plain", "Student submission successful");
  });

  server.on("/processes", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
        if (request->hasParam("proc", true)) {
            message = request->getParam("proc", true)->value();
            sendProcessFile(message, request->getParam("s_id", true)->value());
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void setup() {
  Serial.begin(115200);
  ap_setup();
}

void loop() {
  parse_serial();
}
