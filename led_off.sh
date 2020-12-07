#!/bin/bash
mosquitto_pub -t "nodemcu/led" -i "publisher1" -m "{\"client\":\"publisher1\", \"led\": \"OFF\"}"