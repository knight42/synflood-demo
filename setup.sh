#!/bin/bash

docker network rm flood

set -e
docker network create \
  --attachable \
  --subnet=172.28.0.0/16 \
  --ip-range=172.28.5.0/24 \
  --gateway=172.28.5.254 \
  flood

docker run -tid --name victim \
    --entrypoint nginx \
    --sysctl 'net.ipv4.tcp_syncookies=0' \
    --network flood knight42/synflood \
    -g 'daemon off; pid /run/nginx.pid; error_log stderr;'

docker run -tid --name evil1 --network flood knight42/synflood
docker run -tid --name evil2 --network flood knight42/synflood
