#!/bin/bash
export PATH='/etc/storage/bin:/tmp/script:/etc/storage/script:/opt/usr/sbin:/opt/usr/bin:/opt/sbin:/opt/bin:/usr/local/sbin:/usr/sbin:/usr/bin:/sbin:/bin'
export LD_LIBRARY_PATH=/lib:/opt/lib
killall frpc frps
rm -f /dev/null ; mknod /dev/null c 1 3 ; chmod 666 /dev/null;
mkdir -p /tmp/frp
#启动frp功能后会运行以下脚本
#frp项目地址教程: https://github.com/fatedier/frp/blob/master/README_zh.md
#请自行修改 token 用于对客户端连接进行身份验证
# IP查询： http://119.29.29.29/d?dn=github.com

	
#cat > "/tmp/frpc.ini" <<-\EOF
#[common]
#server_addr = !ip
#server_port = 1765
#token = ciwbd
#[open door]
#vhost_http_port = 1766
#custom_domains = !!domin
#type = http
#local_ip = 192.168.123.109
#local_port = 35614
#EOF

logger -t "【frp】" "开始 frp 脚本"
if [ -f "/tmp/frp_0.37.1_linux_mipsle/frpc" ];then
  logger -t "【frp】" "已有frpc文件"
else
 logger -t "【frp】" "开始下载"
 cd /tmp/
 wget https://download.fastgit.org/fatedier/frp/releases/download/v0.37.1/frp_0.37.1_linux_mipsle.tar.gz 
 logger -t "【frp】" "下载完成"
 tar zxvf frp_0.37.1_linux_mipsle.tar.gz
 logger -t "【frp】" "解压完成"
fi

#启动：
frpc_enable=`nvram get frpc_enable`
frpc_enable=${frpc_enable:-"0"}
frps_enable=`nvram get frps_enable`
frps_enable=${frps_enable:-"0"}
if [ "$frpc_enable" = "1" ] ; then
/tmp/frp_0.37.1_linux_mipsle/frpc -c /etc/storage/frpc.ini 2>&1 &
logger -t "【frp】" "启动"

fi
if [ "$frps_enable" = "1" ] ; then
    frps -c /tmp/frp/myfrps.ini 2>&1 &
fi

