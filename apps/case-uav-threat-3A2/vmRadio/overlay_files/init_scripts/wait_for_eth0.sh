#! /bin/sh

while ! [ "$(cat /sys/class/net/eth0/operstate)" == "up" ];
do
  echo "Waiting for eth0"
  sleep 2
done


