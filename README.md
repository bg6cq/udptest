## udpmtuserver udpmtusend 链路UDP MTU测试

工作正常的网络，IPv4/IPv6协议分别可以传输长度为65507/65527字节的UDP包。

本程序用来测试实际的网络中，各种大小的UDP数据包是否能正确传输。

如果无法传输长一些的UDP数据包，会严重影响到DNSSEC及基于UDP协议的隧道程序的正常运行。

#### 简单的测试

只要安装过gcc、git，就可以运行测试。

在 202.141.160.125(电信)、218.104.71.172(联通)、202.141.160.125(移动) 6000 端口运行有测试的服务器端，因此只要以下命令既可以简单进行测试
```
cd /usr/src/
git clone https://github.com/bg6cq/udptest
cd udptest
make
./udpmtusend 202.141.160.125 6000
./udpmtusend 218.104.71.172 6000
./udpmtusend 202.141.176.125 6000
```

服务器端网络，对IPv4/IPv6协议，应当支持长度为65507/65527字节的UDP数据包。

如果显示的结果如下，说明您的网络正常，可以传送任意符合协议的UDP数据包：

```
sending 1470 - 1500 bytes UDP to 202.141.176.125:6000
udp_len=1470  ip_pkt_len=1498  C-->S OK   S-->C OK   
udp_len=1471  ip_pkt_len=1499  C-->S OK   S-->C OK   
udp_len=1472  ip_pkt_len=1500  C-->S OK   S-->C OK   
udp_len=1477  ip_pkt_len=1505  C-->S OK   S-->C OK   
...
udp_len=1478  ip_pkt_len=1506  C-->S OK   S-->C OK   
udp_len=1479  ip_pkt_len=1507  C-->S OK   S-->C OK   
udp_len=1499  ip_pkt_len=1527  C-->S OK   S-->C OK   
udp_len=1500  ip_pkt_len=1528  C-->S OK   S-->C OK
```

我测试发现移动网络的IP地址，如果UDP数据包长度介于1473-1479、2953-2959、4433-4439。。。等之间，传输时最后一个IP分片<8字节，这个分片会被丢弃，导致UDP通信受阻，如下所示(从位于电信的某个IP地址测试):
```
sending 1470 - 1500 bytes UDP to 202.141.176.125:6000
udp_len=1470  ip_pkt_len=1498  C-->S OK   S-->C OK   
udp_len=1471  ip_pkt_len=1499  C-->S OK   S-->C OK   
udp_len=1472  ip_pkt_len=1500  C-->S OK   S-->C OK   
udp_len=1473  ip_pkt_len=1501  C-->S OK   .
udp_len=1474  ip_pkt_len=1502  C-->S OK   .
udp_len=1475  ip_pkt_len=1503  C-->S OK   .
udp_len=1476  ip_pkt_len=1504  C-->S OK   .
udp_len=1477  ip_pkt_len=1505  C-->S OK   .
udp_len=1478  ip_pkt_len=1506  C-->S OK   .
udp_len=1479  ip_pkt_len=1507  C-->S OK   .
udp_len=1480  ip_pkt_len=1508  C-->S OK   S-->C OK   
udp_len=1481  ip_pkt_len=1509  C-->S OK   S-->C OK   
udp_len=1482  ip_pkt_len=1510  C-->S OK   S-->C OK 
```

家庭宽带网络如果出现奇怪的结果，大部分是某个设备MTU设置错误引起的，修改后也会正常。

需要注意的是：

* 苹果macOS操作系统，默认限制最大UDP包为9216字节。macOS可以通过命令`sudo sysctl -w net.inet.udp.maxdgram=65535`修改。

* Linux 系统默认发出的UDP包带有DF标志，如果中间链路的MTU变小，中间路由器无法重新进行分片处理。udpmtuserver中通过下面的片段使得发出的UDP包不带DF标志：
```
int optval = 0;
if(setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &optval, sizeof(optval) != 0) {
	fprintf(stderr, "Error: setsockopt %d\n",errno);
}
```


#### UDP数据包与IP协议包的关系

IPv4/IPv6的协议头部长度字段为16bit，能存放的最大长度是65535。

UDP包在传输时，IP协议长度字段的内容有些不同。IPv4包头长度字段存放的是包含协议头的部分长度，而IPv6包的长度字段不含IP协议头部分。

| 协议  |  UDP包长度 | IP协议头长度  | UDP协议头长度 | IP包长度字段  |
| :---- | :--------- | :------------ | :------------ | :----------   |
| IPv4  | len字节    | 20字节        | 8字节         | len + 28 字节 |
| IPv6  | len字节    | 40字节        | 8字节         | len + 8 字节  |

#### 网络上能传输的最长UDP包是多少呢？

IP协议包头长度字段最大为65535，IPv4/IPv6协议能传输的最长UDP包为65507/65527字节：

| 协议  |  UDP包长度 | IP协议头长度  | UDP协议头长度 | IP包长度字段              |
| :---- | :--------- | :------------ | :------------ | :-----------------------  |
| IPv4  | 65507字节  | 20字节        | 8字节         | len + 28 字节 = 65535 字节|
| IPv6  | 65527字节  | 40字节        | 8字节         | len + 8 字节  = 65535 字节|

互联网链路的的MTU会远远小于64KB，在传输长的UDP包时会进行分片和重组操作。

由于种种原因，如某个设备配置错误，或某个设备未进行分片或重组操作，或某个设备过滤了分片数据包，实际能传输的会少于65507/65527字节。

#### 网络上确保可以传输的UDP包最长是多少呢？

| 协议  | 最小MTU  | IP协议头长度 | UDP协议头长度 | 可以确保传输的UDP包长度                             |
| :---- | :------- | :----------- | :------------ | :-------------------------------------------------- |
| IPv4  | 576字节  | 60字节       | 8字节         | 576 - 60 -8 = 508 字节                              |
| IPv6  | 1280字节 | 40字节       | 8字节         | 1280 - 40 - 8 = 123 字节(假定未使用任何IPv6头扩展)  |


### 测试过程：

服务器端：(x.x.x.x是服务器端IP地址，支持IPv6）
```
./udpmtuserver x.x.x.x 6000
```

测试端：
```
./udpmtusend x.x.x.x 6000  [ min_len max_len ]
```

测试原理：

udpmtusend 发送从 min_len - max_len 大小的UDP数据包，服务器端收到后应答，从而测试通信是否正常。
```
udpmtusend -> updmtuserver REQlen   (向服务器请求发送长度为len的字节)
udpmtuserver -> udpmtusend RET..... (UDP总长度为len, IP长度为len + 28或len + 8)，收到则显示 S-->C OK

udpmtusend -> updmtuserver PKT..... (总长度为len, IP长度为len +28或len + 8)
udpmtuserver -> udpmtusend ACKlen   (长度len为2个字节， 服务器声明自己收到过长度为len的包), 收到则显示 C-->S OK
```


## udpserver udpsend 测试UDP性能

###  Increasing Linux kernel network buffers

Increase the UDP receive buffer size from 128K to 32MB
````
sysctl -w net.core.rmem_max=33554432
````

### updsend

send udp packets for test
````
./udpsend -c 10000000 127.0.0.1 900
packet_len = 1472, packet_count = 10000000
42.969 seconds 10000000 packets 14720000000 bytes
PPS: 232727 PKT/S
UDP BPS: 2740594660 BPS
ETH BPS: 2792725537 BPS
WireBPS: 2863474584 BPS
done
````

### udpserver

receive udp packets for test
````
./udpserver -4 900
42.970 seconds 10000000 packets 14720000000 bytes
PPS: 232719 PKT/S
UDP BPS: 2740497342 BPS
ETH BPS: 2792626367 BPS
WireBPS: 2863372902 BPS
````
