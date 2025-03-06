# V8 Shape & Inline-Caches

> http://agroup.baidu.com/lvsheng/md/article/3356173#

## 背景1：V8
* [@岩之前关于V8的分享](http://agroup.baidu.com/atom_native/view/office/2792652)
	* JS是一门脚本语言
	* V8可以执行JS脚本(代码)
		* ![图片](https://agroup-bos-bj.cdn.bcebos.com/bj-5ab0dadf1072093a429f36d74055a6dcb0948158)
		* code --解析--> AST --解释--> 字节码 --> 执行(<-->JIT & 优化)
	* 怎么执行？：
		* V8(自身是一大坨c++代码)认识字节码，知道其中每一条指令要做什么，看到时通过对应的内部c++代码去完成相应的工作
			* (不准确，仅作为大致的理解)
	* 另一个视角：V8为JS管理内存

## 背景2：JS动态性
* java/c++/dart：强类型语言，对象基于类
	* 类的成员(field)<!--编译时/写代码时-->固定，实例(对象)的内存空间(大小)及分布固定
		* ![图片](https://agroup-bos-bj.cdn.bcebos.com/bj-9d1c2cb05a3e47a3e4d69a82644e6f68c53bda59)
			* [图片来源](http://lifegoo.pluskid.org/upload/doc/object_models/C++%20Object%20Model.pdf)
		* `foo.a`时成员的内存地址：对象首地址 + 字段偏移量(编译时确定)
			* **快！**
* js：弱类型语言，灵活动态
	* 对象：可以动态增删属性
	* 根本上：类似其他语言中的map（key-value键值对<!--、如HashMap-->）
	* `foo.a`时成员的内存地址：map查找
		* **慢！**

## V8对于成员访问的优化
* JS成员访问：
	* ~~快：对象首地址 + 字段偏移量~~
	* 慢：类map查找

* 事实：绝大多数时，对象的内存空间及分布是重复/固定(有限)的
* 假设：函数每次运行时的形参，其对象的内存空间及分布是重复的
	* 满足时：优化
	* 打破假设时：回退
		* 前面V8图中的deoptimize
			* **慢！**
* 满足时的优化：
	* ?

### Shape
* JS语言中没有class的概念，但V8在实现时引入Hidden Classes("隐藏类")/Shape的概念
	* 每一个对象在V8管理的内存空间中(V8为JS管理内存)，背后都指向一个Shape
	* ![图片](https://agroup-bos-bj.cdn.bcebos.com/bj-83d39caffcd2a91bf56a7fcf04f72f76dc7a0461)
	<!--* ![图片](https://agroup-bos-bj.cdn.bcebos.com/bj-aeac6c4031695ad1f0bb149924d39e2e3961b2f5)
		* Object.getOwnPropertyDescriptor-->
		* 增删属性时指向(创建)新的Shape、Shape树

### Inline-Caches
![图片](https://agroup-bos-bj.cdn.bcebos.com/bj-5f409021e7b9a1319ec9b9d8be65388e1cdd19e1)

* 字节码：`get_by_id`指令
	* 引入Inline-Cache
	* 首次调用之前：没有任何可假设的信息
	* 首次调用之后，假设其Shape：
		* ![图片](https://agroup-bos-bj.cdn.bcebos.com/bj-ca814415adcc048e3a2ad5bb206f7ce75a879970)
	* 再次调用满足假设时：
		* ![图片](https://agroup-bos-bj.cdn.bcebos.com/bj-52b8502a2281cc6013e6651b4451df07b68fcb0f)
		* JS成员访问：
			* 快：对象首地址 + 字段偏移量
			* ~~慢：map查找~~

### 什么样的JS代码才快？
* 快：基于假设
	* 不要打破V8的假设
		* 函数每次运行时的形参，其对象的内存空间及分布是重复的
	* 不要频繁引入Shape的变更

good:
```
var o = { // 字面量 or class，一次确定其Shape
  w: 1,
  x: someValue,
  y: 2,
};
```

bad:
```
var o = {};
// 频繁变更Shape
o.w = 1;
o.x = someValue;
o.y = 2;
```

good:
```
// 每次调用时实参的Shape相同
getX({w: someW, x: 'a', y: someY});
getX({w: someW, x: 'b', y: someY});
getX({w: someW, x: 'c', y: someY});
```

bad:
```
// 每次调用时实参的Shape不同
getX({x: 'a'});
getX({w: someW, x: 'b'});
getX({w: someW, x: 'c', y: someY});
```

## 参考：
* https://mathiasbynens.be/notes/prototypes#optimizing-prototype-property-access
* https://mathiasbynens.be/notes/shapes-ics#ics
	* 中文：https://zhuanlan.zhihu.com/p/38202123
* https://v8.dev/blog/react-cliff
	* react曾陷入的性能陷阱
