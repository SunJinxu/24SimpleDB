# 24SimpleDB
a simple db inspired by sqlite

## 1 Making a Simple REPL(read-execute-print loop)
关键点在与实现三个主要组件：
* prompt打印
* 控制台输入消息接收
* 消息接收处理逻辑

其中，prompt打印可以打印我们自己的`24SimpleDb`标识；控制台消息接收使用自定义的结构体进行读取和存储；消息处理逻辑则用if_else结构进行匹配。