# udptest

##  Increasing Linux kernel network buffers

Increase the UDP receive buffer size from 128K to 32MB
````
sysctl -w net.core.rmem_max=33554432
````

## updsend

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

## udpserver

receive udp packets for test
````
./udpserver -4 900
42.970 seconds 10000000 packets 14720000000 bytes
PPS: 232719 PKT/S
UDP BPS: 2740497342 BPS
ETH BPS: 2792626367 BPS
WireBPS: 2863372902 BPS
````

