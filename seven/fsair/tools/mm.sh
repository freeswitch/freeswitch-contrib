#!/bin/sh

echo > /tmp/a
echo > /tmp/b
#tail -f /tmp/b &
tail -f /tmp/a | nc localhost 8021 | tee2 /tmp/b | nc -l -k 8022 | tee /tmp/a 

