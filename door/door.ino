
#include <ESP8266WiFi.h>
#ifndef STASSID
#define STASSID "HiWiFi_5C0B98"
#define STAPSK  "gzwl6009"
#endif

#define maxd 180
#define mind 5
#define buzzerPin 12 
//D6
#define ServoPin 14 //d5

long map(long x,long in_min,long in_max,long out_min,long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void ServoControl(int servoAngle)
{
  double thisAngle = map(servoAngle, 0, 180, 500, 2500);//等比例角度值范围转换高电平持续时间范围
  unsigned char i = 50;//50Hz 每秒的周期次数(周期/秒) 即1S 50 个周期 每个周期20ms
  while (i--)
  {
    digitalWrite(ServoPin, HIGH); 
    delayMicroseconds(thisAngle); //高电平时间
    digitalWrite(ServoPin, LOW); 
    delayMicroseconds(20000 - thisAngle);//每个周期20ms减去高电平持续时间
  }
}



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
  ServoControl(mind);
  delay(10*(maxd-mind));
  beep();
  delay(5000);
  ServoControl(maxd); 
  tone(buzzerPin,800,500);
  delay(2000);
}

void setup() {
  pinMode(buzzerPin,OUTPUT);
  pinMode(ServoPin,OUTPUT);
  Serial.begin(115200);//开启端口，速度为115200
  delay(10);
  // prepare GPIO2
  // Connect to WiFi network
  Serial.print("Connecting to ");  
  WiFi.setAutoReconnect(true);//设置断开连接后重连
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
  
  Serial.println(WiFi.localIP());
  open_door();
  Serial.println("servo test ok");
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
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
    // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n ok!\n";
  // Send the response to the client
  client.print(s);
  // 比对收到的讯息，确定执行什么操作
  if (req.indexOf("?gpio=on") != -1){
    Serial.println("open door");//打印出收到的讯息
    open_door();
  }else {
    Serial.println("invalid request");
  }
  client.flush();//刷新
  // Prepare the response
  //String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n ok!\n";
  // Send the response to the client
  //client.print(s);
  //delay(1);
  //Serial.println("Client disonnected");
}
