# Server

## 目录结构

~~~
|-- build							# 构建脚本
|-- cmake							# 开源软件构建脚本
|-- src								# 
|	|-- channel						# 网络通讯
|	|-- defs						# 全局定义
|	|-- entry						# 编译模块
|	|	|-- server					#
|	|	|	|-- bin					# server模块
|	|-- protocol					# 消息定义
|	|-- module						# 
|	|   |-- base					# module共用基类 
|	|   |-- global					# 全局消息
|	|   |-- timeline				# timeline消息处理
|	|   |	|-- core				# 核心处理逻辑
|	|   |	|-- handler				# 消息处理
|	|   |	|-- protocol    		# 消息格式转换
|	|   |-- ...						#
|	|-- server						# server服务
|	|-- utils						# 工具类
|-- third_party						# 开源软件
	
~~~



## 构建说明

### 构建工具

1.cmake
https://clouddragon.huawei.com/cloudbuild/project/g05eddbe2f77d400ba43b805e8bca793a/buildToolManage/buildToolDetails?versionId=0591A8KY
2.mingw-w64（windows平台）
https://clouddragon.huawei.com/cloudbuild/project/g05eddbe2f77d400ba43b805e8bca793a/buildToolManage/buildToolDetails?versionId=0591A9HS
3.python3
https://clouddragon.huawei.com/cloudbuild/project/g05eddbe2f77d400ba43b805e8bca793a/buildToolManage/buildToolDetails?versionId=0591A9ND
4.ninja（linux平台）
https://clouddragon.huawei.com/cloudbuild/project/g05eddbe2f77d400ba43b805e8bca793a/buildToolManage/buildToolDetails?versionId=0591A6QK
5.xcode（mac平台）
https://clouddragon.huawei.com/cloudbuild/project/g05eddbe2f77d400ba43b805e8bca793a/buildToolManage/buildToolDetails?versionId=0591A7GJ



### 开源代码

群空间下载开源代码， 解压覆盖到thirdparty目录

https://onebox.huawei.com/p/7606b4ccd02cec637bc5f518c156a184



### 构建

```
cd build
python build.py build --release
```

