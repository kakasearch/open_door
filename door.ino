
#include <ESP8266WiFi.h>
#include <Servo.h>
#ifndef STASSID
#define STASSID "HiWiFi_5C0B98"
#define STAPSK  "gzwl6009"
#endif

#define maxd 180
#define mind 25
#define buzzerPin 12 
//D6


Servo myservo;
const char* ssid = STASSID;
const char* password = STAPSK;
WiFiServer server(35614);//开启板子的port 80

void beep(){
  int i;
  for (i=0; i<3; i++) { // goes from 180 degrees to 0 degrees
    tone(buzzerPin,800,500);
    delay(1000);
  };
  } 
void open_door() {
  int pos;
  myservo.write(mind); 
  delay(12*(maxd-mind));
  beep();
  delay(5000);
  for (pos = mind; pos <= maxd; pos += 5) { // goes from 0 degrees to 180 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(60);                       // waits 15ms for the servo to reach the position
  };
}

void setup() {
  pinMode(buzzerPin,OUTPUT);
  Serial.begin(115200);//开启端口，速度为115200
  delay(10);
  // prepare GPIO2
  // Connect to WiFi network
  Serial.print("Connecting to ");  
  WiFi.begin(ssid, password);//使用WiFi开始连线
  while (WiFi.status() != WL_CONNECTED) {//连线成功后停止跳点
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Start the server
  server.begin();
  Serial.println("Server started");
  myservo.attach(2);
  open_door();
  Serial.println("servo test ok");
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);//打印出收到的讯息
  client.flush();//流刷新，如果不flush，那么可能就堵塞了
  
  // 比对收到的讯息，确定执行什么操作
  int val;
  if (req.indexOf("gpio=1") != -1){
    Serial.println('open door');//打印出收到的讯息
    open_door();
  }else {
    Serial.println("invalid request");
    client.stop();
    return;
  }
  client.flush();//刷新
  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val)?"high":"low";
  s += "</html>\n";
  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
}
