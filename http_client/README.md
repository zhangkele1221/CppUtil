# Http Client

## 简介

简易Http客戶端。

## 依赖

```bash
# curl
$wget https://curl.se/download/archeology/curl-7.29.0.tar.gz --no-check-certificate
$tar xvf curl-7.29.0.tar.gz
$cd curl-7.29.0
$./configure
$make
$make install
```

## 特性

* 支持设置连接超时和传输超时
* 支持设置debug模式（设置宏_HTTP_CLIENT_DEBUG为true）
* 支持CA证书
* 支持POST和GET

## 例子

```c++
#include "httpclient.h"

int main() {
    std::string resp;
    httpclient::Get("www.baidu.com", 400, 200, resp);
    printf("%s", resp.c_str());
}
```