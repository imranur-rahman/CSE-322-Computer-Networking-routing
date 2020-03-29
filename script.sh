#!usr/bin/bash

g++ -std=c++11 router_1305015.cpp -o router_1305015

xterm -title "router 1" -e "./router_1305015 192.168.10.1" &
xterm -title "router 2" -e "./router_1305015 192.168.10.2" &
xterm -title "router 3" -e "./router_1305015 192.168.10.3" &
xterm -title "router 4" -e "./router_1305015 192.168.10.4"
