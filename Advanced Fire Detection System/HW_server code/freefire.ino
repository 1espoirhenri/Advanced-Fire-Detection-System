#include <WebServer.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <esp32cam.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "Your_wifi";
const char* WIFI_PASS = ""Your_wifi_password";

const char* server_address = "your_computer_ip_address"; // Địa chỉ máy chủ HTTP
const int server_port = 80; // Port mặc định cho HTTP là 80
const String endpoint = "/trigger"; // Đường dẫn tới endpoint hoặc API trên máy chủ

int buttonPin = 13;
int buzzerPin = 12;
int angle = 0;
int loopnum = 0;
bool check = false;
bool initState = true;
int conentiep = 0;
WebServer server(80);

#define servo_LeftRight_Pin 14
#define servo_UpDown_Pin 15

 
Servo servo_LeftRight;
Servo servo_UpDown;
 
static auto pos1 = esp32cam::Resolution::find(640, 640);
void handleFireAlert() {
  if(initState == false){
  String message = server.arg("message");
  Serial.print("Received fire alert message: ");
  Serial.println(message);
  // Xử lý message tại đây (ví dụ: hiển thị trên màn hình LCD, kích hoạt cảnh báo, ...)
  server.send(200, "text/plain", "Message received successfully");
  check = true;
  if(message=="Chay")
  {
    digitalWrite(buzzerPin, 1);
    delay(1000);
    digitalWrite(buzzerPin, 0);
    initState = true;
    angle = 0;
    conentiep=0;
  }
  else if(message=="Nothing"){
    xoaycam();
    
    }
  }

}



void xoaycam(){
if (conentiep > 9)
    {
      initState = true;
      angle = 0;
      conentiep=0;
      digitalWrite(4, 1);
      delay(1000);
      digitalWrite(4, 0);
      delay(1000);
    }
    else{
      servo_LeftRight.write(angle);
      // Serial.println("Xoay angle");
      angle += 90;
      if( angle > 180) 
      {
        angle = 0;
      }

        sendSignal();
        conentiep+=1;
    }
}

void serveJpg()
{
  auto frame = esp32cam::capture();
  delay(1000);
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
 
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpgPos1()
{
  
  delay(100); // Short delay to allow servo to stabilize
  if (!esp32cam::Camera.changeResolution(pos1)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
  delay(800);
}
 

void sendSignal(){
  
  HTTPClient http;
  String url = "http://" + String(server_address) + ":" + String(server_port) + endpoint;
  Serial.print("Sending HTTP request to: ");
  Serial.println(url);
  
  http.begin(url);
  int httpCode = http.GET();
  delay(1000);
  http.end(); 
  // http.setTimeout(10000);
  if(httpCode >0 ) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } 
  else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    sendSignal();
  }
  
}


void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(buzzerPin, LOW);


  servo_LeftRight.attach(servo_LeftRight_Pin, 1000, 2000);
  // servo_UpDown.attach(servo_UpDown_Pin, 1000, 2000);
  servo_LeftRight.write(0);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); // Prints dots while waiting for connection
  }

  Serial.println("\nConnected to WiFi. IP Address: ");
  Serial.println(WiFi.localIP());

  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(pos1);
  cfg.setBufferCount(2);
  cfg.setJpeg(80);

  if (Camera.begin(cfg)) {
    Serial.println("CAMERA OK");
  } else {
    Serial.println("CAMERA FAIL");
  }

  server.on("/position1.jpg", handleJpgPos1);
  // server.on("/position2.jpg", handleJpgPos2);
  // server.on("/position3.jpg", handleJpgPos3);
  server.on("/fire_alert", handleFireAlert);
  server.begin();

  Serial.println("HTTP server started");
  Serial.println("Visit the following addresses for position control:");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/position1.jpg");
  servo_LeftRight.write(0);
}


void loop() {
    server.handleClient();
    if(digitalRead(buttonPin) == LOW && initState == true){
      sendSignal();
      initState = false;
    }
}