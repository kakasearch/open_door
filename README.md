# open_door
 利用 nodemcu 实现无线开门
# 功能
- 支持局域网控制，访问`http://{esp8266的IP}?gpio=on`时开门 
- 支持小爱同学控制
- 支持小度语音助手
- 可通过小度语音助手的app抓包实现外网控制开门。如果不抓包，就需要内网穿透才能在外网控制
- 支持刷卡开门

# 需要的元件
1. nodemcu 或者arduino+esp8266
2. rc522模块
3. 舵机
4. 导线、电源（舵机需要至少2A的电源才能动起来）
5. led（非必要）、蜂鸣器（非必要）
# 连线
- rc522与nodemcu连线参考：https://blog.csdn.net/qq_31878883/article/details/88971935
- 舵机 data线连GPIO15(nodemcu的d8),vcc连vin或者连外接电源，目的是提供5V和2A的电流
- led灯珠连在GPIO10上，开门后会亮起作为已开门的提醒
# 需要修改的地方
```c++
#define blinksk "blink的密钥" //12行
#define STASSID "wifi名字" //13行
#define STAPSK  "wifi密码"  //14行

byte data[5][4] = { 0x19,0x04,0x24,0xBE, //在代码28行。管理员的16进制卡号，可通过rc522库的示例代码dumpinfo获得
                    0x01,0xC1,0x1E,0x1C, //每张卡都是4字节
};
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
