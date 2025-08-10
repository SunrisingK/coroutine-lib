#### 安装测试工具

```shell
sudo apt update
sudo apt install apache2-utils
```

#### 编译执行

```shell
cd coroutine-lib && cd fiber_lib && cd 6hook 
g++ *.cpp -std=c++17 -o main -ldl -lpthread
./main
```


#### 启动测试工具

```shell
ab -n 100 -c 10  http://127.0.0.1:8080/
```
