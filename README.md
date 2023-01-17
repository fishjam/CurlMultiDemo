# CurlMultiDemo

- 使用 libCurl 的 [Multi Interface](https://curl.se/libcurl/c/libcurl-multi.html) 的一个简单封装类,支持功能:
  - 单线程 + 纯异步 的 Get/Post 支持
  - 极高的性能(使用 POST 上传 600+ 文件,仅耗时6秒) 

- 使用方式:
  - 1.编译或下载 libcurl 库, 我采用的是 vcpkg
  - 2.搭建或找一个支持 MultiPart 上传文件的服务器, 如没有,可以使用 [vertx-upload](https://github.com/vert-x3/vertx-examples/blob/4.x/web-examples/src/main/java/io/vertx/example/web/upload/Server.java)
  - 3.Visual Studio(2015) 打开工程, 按需更改 CCurlMultiDemo::_curlMgr_multiPartPost 中的以下部分:
     - fileFinder.Find(TEXT("D:\\vcpkg\\installed\\")     <== 从该目录下扫描指定格式的所有文件,更改成自己想要的目录
     - strUrl = TEXT("http://127.0.0.1:8080/form")        <== 文件将被 POST 上传到以下路径
   - 4.运行程序, 即会扫描目录,并上传文件, 最后弹出对话框,显示长传文件数量和耗时.
   - 5.如有问题, Debug 模式下应该有断言, 或者可以查看 %LOCALAPPDATA%\Temp\FTL 目录下的日志文件
  
- 注意:
  - 由于是业余时间学习过程中编写,而且没有应用到实际项目代码中,因此尚不完善,而且可能存在问题(如 逻辑错误或内存泄露等),请小心使用,发现问题也可以提 PR.
  
- 补充:
  ut_files 下的文件是我原本的 UT 文件和部分笔记(包括安装说明), 可以参考