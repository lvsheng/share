# HTTP测试环境搭建 - 一枚bug的跟踪记录

<!--
charles里准备的请求：
* aaa.js
* 500.js
* 404.js
* hang
* hang-head
* large-length
* https://duer.bdstatic.com/saiya/swan/duershow/duershow.olympic_game.feb222.js
* chunked
* 
-->

---

## 前言
* 作为工程师，我们最多的时间花在了？
	* Debuging
	* Debugging is at the core of programming, because it takes up the largest percentage of your time while coding.
		* from [here](https://www.freecodecamp.org/news/how-to-improve-your-debugging-skills/)
		* [This is what your developers are doing 75% of the time](https://www.slideshare.net/Coralogix/this-is-what-your-developers-are-doing-75-of-the-time-and-this-is-the-cost-you-pay)
* 如何Debug？
	* 发现->复现->解决
		* 设想->验证

---

## 原始Bug: 奥运会页面点击无效

* 仅QA手头的一台设备出现。稳定复现，设备重启无效。
（可以滑动/翻页，但新内容不加载，有点击态但点击无效）

![](http://bos.bj.bce-internal.sdns.baidu.com/agroup-bos-bj/bj-4f0760f193e2179c883aa49c4c4e189a2c5312e7)

---

## 看日志

1. 可以连接adb：adb logcat | grep -E 'E V8Core|-Error'
	1. [release设备连adb](https://wiki.baidu.com/pages/viewpage.action?pageId=1295471702#adb%E5%AF%86%E9%92%A5%E5%8D%87%E7%BA%A7%E6%96%B9%E6%A1%88%E7%94%B3%E8%AF%B7%E6%96%B9%E6%B3%95&%E7%99%BD%E5%90%8D%E5%8D%95%E5%88%97%E8%A1%A8%E7%BB%9F%E8%AE%A1-%E7%94%B3%E8%AF%B7%E7%99%BD%E5%90%8D%E5%8D%95
)
2. 不能连接adb：“设置”->“本机信息”->“上传小度日志”后，在crab平台下载

* ![](http://bos.bj.bce-internal.sdns.baidu.com/agroup-bos-bj/bj-fe16a29c3235191e2d9240c981a368ca15d1d274)

---

## 损坏的JS文件？
* url不对？
	* https://duer.bdstatic.com/saiya/swan/duershow/duershow.olympic_game.feb222.js
* 文件损坏？(内容中断)
	* 验证：拉尸体
		* ![图片](http://bos.bj.bce-internal.sdns.baidu.com/agroup-bos-bj/bj-6f56855f6865dd6de024015c3cc3f05b600e3a90)
		* release设备：没办法（可以开启adb、但无法root、无法安装系统应用apk）
	* 验证：[模拟复现](cutted_duershow.olympic_game.feb222.html)

---

* 推论：设备上JS文件内容损坏
* WHY？
	* CDN某节点下此文件有问题？
		* pingo抓日志

---

![图片](http://bos.bj.bce-internal.sdns.baidu.com/agroup-bos-bj/bj-29af99e948a31cd9fccab71f727febe628ccf79c)

---

* 推论：问题出在端上
* WHY？
	* cache库/网络库问题？
		* flutter_cache_manager、http
		* content-length、chunked
* 验证：
	1. 代理、抓包
	2. 构造server，模拟各种场景
		* 40x/50x/错误的content-length/hang住中断/错误的chunked内容/解压失败等

---

## flutter配置代理：

```dart
class MyProxyHttpOverride extends HttpOverrides {
  @override
  HttpClient createHttpClient(SecurityContext context) {
    return super.createHttpClient(context)
      ..findProxy = (uri) {
        return "PROXY 172.24.222.13:8889;";
      }
      ..badCertificateCallback =
          (X509Certificate cert, String host, int port) => true;
  }
}

main() {
  // ...
  HttpOverrides.global = MyProxyHttpOverride();
  // ...
}
```

---

## charles
![图片](http://bos.bj.bce-internal.sdns.baidu.com/agroup-bos-bj/bj-9e666bd57cde2297c555d8f0654030ba228786c7)

	Proxy -> Proxy Settings -> Http Proxy -> Port
	https: Proxy -> SSL Proxy Settings -> Enable（证书：Help -> SSL Proxying -> Install xxx）

<!--	* 小度设备：
		* `adb shell am start -n com.android.settings/com.android.settings.Settings`
		* `adb shell am start -a android.intent.action.VIEW -d http://chls.pro/ssl`
![图片](http://bos.bj.bce-internal.sdns.baidu.com/agroup-bos-bj/bj-07d4615895cecbec21126db676d1659940b15dee)

问题：小度上默认打开网址的浏览器没有使用系统代理。应该需要单独安装下载一下证书
-->

---

## server
* [web-server.js](https://github.com/NathanMaton/Contribution-Tracker/blob/7ad861319a5530926d06100fa6cfc0a1f9a31f2c/public/scripts/web-server.js#L87)
	* from [github search StaticServerlet](https://github.com/search?p=1&q=StaticServlet&type=Code)
	* 后文分享中使用的版本：[testcase](https://console.cloud.baidu-int.com/devops/icode/repos/baidu/duer/atom-flutter-examples/blob/master:testcase/src/scriptloader-with-server/web-server.js#L466)
* [200](http://localhost:8888/aaa.js)
* [500](http://localhost:8888/500.js)、[404](http://localhost:8888/404.js)

---
### hang ?

* [hang](http://localhost:8888/hang.js)、[hang-head](http://localhost:8888/hang-head.js)
	* hang.js有cache文件，但下次不会被使用。hang-head.js无cache文件
	* kill server模拟连接中断：不会被使用<!--背后其实是chunked协议-->

---

```bash
lvsheng@lvsheng-mac ~/temp $ adb shell rm -rf  /data/user/0/com.example.atom_js_bridge/cache/libCachedImageData/                                                                                                                             [ruby-2.5.3p105]
lvsheng@lvsheng-mac ~/temp $ adb shell ls -lh /data/user/0/com.example.atom_js_bridge/cache/libCachedImageData/                                                                                                                              [ruby-2.5.3p105]
ls: /data/user/0/com.example.atom_js_bridge/cache/libCachedImageData/: No such file or directory
lvsheng@lvsheng-mac ~/temp $ echo "after load hang.js, hang-head.js"                                                                                                                                                                         [ruby-2.5.3p105]
after load hang.js, hang-head.js
lvsheng@lvsheng-mac ~/temp $ adb shell ls -lh /data/user/0/com.example.atom_js_bridge/cache/libCachedImageData/                                                                                                                              [ruby-2.5.3p105]
total 2.0K
-rw------- 1 u0_a55 u0_a55_cache 35 2021-08-23 14:34 2629e110-03dc-11ec-8916-a7ba508724d1.js
lvsheng@lvsheng-mac ~/temp $ adb shell cat /data/user/0/com.example.atom_js_bridge/cache/libCachedImageData/2629e110-03dc-11ec-8916-a7ba508724d1.js                                                                                          [ruby-2.5.3p105]
function aaa() {console.log("aaa");%
lvsheng@lvsheng-mac ~/temp $
```

---

### 未收完`Content-Length`长度内容 ?
* [large-length.js](http://localhost:8888/large-length.js)
	* flutter http: 持续等待，不复用cache
	* chrome: 等待数秒后ERROR

---

### 再看duershow.olympic_game.feb222.js
```
Transfer-Encoding: chunked
Content-Encoding: gzip```

* 看一眼协议中定义：https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding
* 怎么看不到chunked的长度？
* `curl -i --raw -H "accept-encoding: gzip" https://duer.bdstatic.com/saiya/swan/duershow/duershow.olympic_game.feb222.js > responce.txt`
	* `-i, --include       Include protocol response headers in the output`
	* `--raw           Do HTTP "raw"; no transfer decoding`
* `vim -b responce.txt`
	* search `\n<ctrl-v><ctrl-m>`(\r\n)
	* `:%!xxd`, `:%!xxd -r`

---

### chunked 未收完内容
* http://localhost:8888/chunked
* 未收完内容：`net::ERR_INCOMPLETE_CHUNKED_ENCODING 200 (OK)`
	* `curl -i --raw http://localhost:8888/hang.js`
* http://localhost:8888/chunked-large-length

---

### gzip失败？
* http://172.24.222.13:8888/chunked-gzip
* http://172.24.222.13:8888/chunked-wrong-zip

---
## 结论：与网络层无关

* 推论：不是网络原因
* IO？
	* dig into source code
	* 流式写入
	* 写入成功后，才作为有效cache存储

---

```
      final sink = file.openWrite();
      await response.content.map((s) {
        receivedBytes += s.length;
        receivedBytesResultController.add(receivedBytes);
        return s;
      }).pipe(sink);

// Stream#done -> IOSink.close -> resolve addStream Future -> (StreamConsumer)IOSink.close -> _FileStreamConsumer.close -> RandomAccessFile.close
```

* close的File一定安全可用吗？

---

![图片](http://bos.bj.bce-internal.sdns.baidu.com/agroup-bos-bj/bj-af7032b33aa28f3d8a3767062c5b041ae95605b9)

---
* https://man7.org/linux/man-pages/man2/close.2.html
	* Typically, filesystems do not flush buffers when a file is closed.
* crash?
* 拔电
	* 复现
