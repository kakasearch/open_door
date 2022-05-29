# open_door
 利用 nodemcu 实现无线开门

# 功能

- 支持局域网控制，访问`http://{esp8266的IP}:{端口号}/gpio`时开门 
- 支持小爱同学控制
- 支持小度语音助手
- 可通过小度语音助手的app抓包实现外网控制开门。如果不抓包，就需要内网穿透才能在外网控制
- 支持刷卡开门

# 需要的元件

1. nodemcu 或者arduino+esp8266
2. wegend协议读卡模块或rc522模块
3. 舵机
4. 导线、电源（舵机需要至少2A的电源才能动起来）
5. led（非必要）、蜂鸣器（非必要）
# 连线
- rc522与nodemcu连线参考：https://blog.csdn.net/qq_31878883/article/details/88971935
- wegend协议的读卡器的data0、data1连接在d2、d3上
- 舵机 data线连GPIO15(nodemcu的d8),vcc连vin或者连外接电源，目的是提供5V和2A的电流
- led灯珠连在GPIO10上，开门后会亮起作为已开门的提醒

# 需要修改的地方

```c++
#define blinksk "blinker密钥" //blinker的sk,需要自己去blinker注册
#define STASSID "HiWiFi_5C0B98" //wifi名称
#define STAPSK  "60096009" //wifi密码
#define len_users 6 //用户数量
byte users[len_users][len_card] = { 
        {0x03,0x0B,0x31},//IC卡卡号
        {0x1C,0xE8,0xA7},
        {0x1E,0xC8,0x11},
        {0x17,0xAA,0xB0},
        {0x27,0xE7,0xAA},
        {0x9F,0x7A,0xCA},
};
ESP8266WebServer server(35614);//自定义的网络端口号
```
# 扩展

- 通过对小度app抓包，可模拟小度的指令，作为API提供给其他程序调用。如qq机器人、手机app、tasker自动化程序等等。。。
-API示例
```python
import requests
def openTheDoor():
    api = 'https://xiaodu.baidu.com/saiya/smarthome/directivesend?from=h5_control'
    headers = {
        'User-Agent': 'Mozilla/5.0 (Linux; Android 11; RMX3161 Build/RKQ1.201217.002; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/92.0.4515.159 Mobile Safari/537.36 FromApp/XiaoDuApp oneapp/4.1.0.1 sdk/0.7.0',
        'Content-Type': 'application/json;charset=UTF-8',
        'Accept': '*/*',
        'Origin': 'https://xiaodu.baidu.com',
        'X-Requested-With': 'com.baidu.duer.superapp',
        'Sec-Fetch-Site': 'same-origin',
        'Sec-Fetch-Mode': 'cors',
        'Sec-Fetch-Dest': 'empty',
        'Referer': 'https://xiaodu.baidu.com/saiya/smarthome/index.html',
        'Accept-Encoding': 'gzip, deflate',
        'Accept-Language': 'zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7',
        'Cookie': 'your cookie!!!!!!!!!!!!!!!',
    }
    data = '''{"header":{"namespace":"DuerOS.ConnectedHome.Control","name":"TurnOnRequest","payloadVersion":1},"payload":{"appliance":{"applianceId":["your ID!!!请自行抓取！！！！！！！！！！"]}}}'''
    request.post(api,headers=headers,data=data)

```

# 其他

<Wiegand.h>库可以在库管理器中搜索"Yet Another Arduino Wiegand Library" 安装
