#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "ESP8266-AP";
const char* password = "12345678";
const int vibrationPin = 0;  // D0引脚，接入震动模块的IN引脚
const int motorPin = 0;      // 现在马达控制引脚接入 D0，修改为 0
int motorSpeed = 64;         // 初始速度，范围0 - 255，降低初始速度
unsigned long previousMillis = 0;  // 用于记录上一次时间
const long interval = 1000;        // 间隔时间，单位毫秒
bool motorOn = true;               // 马达状态，true 表示开启，false 表示关闭

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  // 初始化显示屏
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // 死循环，无法初始化则不继续执行
  }
  display.clearDisplay();

  // 显示启动信息
  displayStartupMessage();

  // 设置ESP8266为AP模式
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // 启动Web服务器
  setupWebServer();

  pinMode(vibrationPin, INPUT);  // 将连接震动模块的引脚设置为输入模式
  pinMode(motorPin, OUTPUT);     // 将控制马达的引脚设置为输出模式
  analogWrite(motorPin, motorSpeed); // 初始化马达速度

  Serial.begin(9600);  // 初始化串口通信，用于输出调试信息
}

void loop() {
  server.handleClient(); // 处理客户端请求
  unsigned long currentMillis = millis(); // 获取当前时间

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    motorOn = !motorOn; // 切换马达状态

    if (motorOn) {
      analogWrite(motorPin, motorSpeed); // 开启马达
      Serial.println("马达开启");
    } else {
      analogWrite(motorPin, 0); // 关闭马达
      Serial.println("马达关闭");
    }
  }

  int vibrationState = digitalRead(vibrationPin);  // 读取震动模块的状态
  if (vibrationState == HIGH) {
    Serial.println("检测到震动");
    // 这里可以添加检测到震动后的处理逻辑，比如发送网络请求等
  } else {
    Serial.println("无震动");
  }
}

void displayStartupMessage() {
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // 第一行
  display.setCursor(0, 0);
  display.println("Connect WiFi ");
  display.display();
  delay(500);  // 延迟 0.5 秒

  // 第二行
  display.setCursor(0, 8);  // 下一行的起始位置 (每行高度为 8 像素)
  display.println(ssid);
  display.display();
  delay(500);

  // 第三行
  display.setCursor(0, 16);
  display.println("Welcome to use");
  display.display();
  delay(500);

  // 第四行
  display.setCursor(0, 24);
  display.println("the Pulse Diagnosis");
  display.display();
  delay(500);

  // 第五行
  display.setCursor(0, 32);
  display.println("Instrument!");
  display.display();
  delay(500);
}

void handleRoot() {
    String message = "Welcome to use the Pulse Diagnosis Instrument! <br>";
    message += "Enter message to display: <form action=\"/display\"><input type=\"text\" name=\"message\"><input type=\"submit\"></form>";
    server.send(200, "text/html", message);
}

void handleDisplay() {
  String message = server.arg("message"); // 获取用户输入的消息
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(message);
  display.display();
  server.send(200, "text/plain", "Message displayed: " + message);
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/display", HTTP_GET, handleDisplay); // 处理显示消息的路径
  server.begin();
  Serial.println("HTTP server started");
}    
