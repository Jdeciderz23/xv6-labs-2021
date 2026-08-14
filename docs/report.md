# 操作系统课程设计报告：xv6 及 Labs 课程项目

## 项目概述

本课程设计选择 MIT 6.S081 / 6.828 的 xv6 Labs 课程项目，基于 RISC-V 版本 xv6 完成 2021 年实验。项目通过阅读 xv6 源码并完成多个实验，理解 Unix-like 操作系统中的用户程序、系统调用、页表、中断、写时复制、线程、网络、锁、文件系统和内存映射等机制。

## 实验环境

- 操作系统：Windows + WSL2
- Linux 发行版：Ubuntu 24.04.3 LTS
- 主要工具：`git`、`make`、`riscv64-linux-gnu-gcc`、`binutils-riscv64-linux-gnu`、`qemu-system-riscv64`、`gdb-multiarch`
- 实验仓库：`https://github.com/Jdeciderz23/xv6-labs-2021`

环境验证方式为在 `util` 分支执行 `make qemu`，xv6 能启动并进入 `$` shell，说明内核编译、文件系统镜像生成和 QEMU 运行链路正常。

## Tools 与 Guidance 阅读总结

Tools 部分主要说明 xv6 Labs 需要的实验工具链。Windows 环境下推荐使用 WSL + Ubuntu，安装 RISC-V 交叉编译器、QEMU、GDB 等工具后，通过 `make qemu` 验证环境。

Guidance 部分强调实验方法：先阅读实验要求和相关源码，再小步修改、小步测试；每完成一个阶段及时提交 Git；遇到内核或用户程序问题时，结合 `printf`、`make grade` 输出和 GDB 定位。

## Lab util：Unix 用户程序

### 实验目的

熟悉 xv6 用户态程序的编写、系统调用的使用、进程创建、管道通信、递归进程模型、目录遍历和标准输入处理。

### 实验内容

本实验需要完成五个用户程序：

- `sleep`：调用系统调用 `sleep`，让进程休眠指定 tick 数。
- `pingpong`：通过两个 pipe 在父子进程之间传递字节，实现 ping/pong 通信。
- `primes`：使用 pipe 和 fork 实现并发素数筛。
- `find`：递归遍历目录，查找指定文件名。
- `xargs`：从标准输入读取行，将其追加到命令参数后执行。

### 关键实现思路

`sleep` 解析命令行参数后直接调用 xv6 已有的 `sleep` 系统调用。`pingpong` 使用两条管道分别完成父到子、子到父的单字节通信。`primes` 将每一级筛选器放入独立进程，用管道把未被当前素数整除的数字传给下一级。`find` 参考 `ls` 的目录读取方式，跳过 `.` 和 `..` 后递归处理目录项。`xargs` 逐字符读取标准输入，按行拆分参数，随后 `fork` 子进程调用 `exec`。

### 测试结果

在 Ubuntu 中执行：

```bash
make grade 2>&1 | tee docs/results/grade-util.txt
```

最终评分结果：

```text
sleep, no arguments: OK
sleep, returns: OK
sleep, makes syscall: OK
pingpong: OK
primes: OK
find, in current directory: OK
find, recursive: OK
xargs: OK
time: OK
Score: 100/100
```

完整评分输出已保存至 `docs/results/grade-util.txt`。

### 问题与解决

- WSL 与 Windows 混用仓库时，Git 状态曾出现大量文件被误判为修改。原因是 Windows Git 的 CRLF 自动转换与 Ubuntu Git 的 LF 预期不一致。处理方式是在本仓库中关闭自动 CRLF，并统一文本文件换行为 LF，避免后续脚本和 Git 状态异常。
- Ubuntu 24.04 的 GCC 对 xv6 既有代码中的递归警告更严格，可能导致 `-Werror` 下编译失败。处理方式是在 `Makefile` 的 `CFLAGS` 中加入 `-Wno-error=infinite-recursion`，保留警告但不把该警告作为错误。
- 初次运行 `make grade` 时，功能测试均通过，但缺少 MIT 要求的 `time.txt`，导致 `time` 项失败。补充 `time.txt` 后，评分达到 `100/100`。
- 运行评分前发现旧的 QEMU 进程仍在后台运行。评分前停止旧进程，避免 `make clean` 和重新生成 `fs.img` 时受到影响。

### 实验心得

本实验主要训练 xv6 用户态程序编写能力。通过 `sleep` 熟悉系统调用入口，通过 `pingpong` 练习 `fork` 与 `pipe` 的配合，通过 `primes` 理解用进程和管道构造并发流水线，通过 `find` 掌握 xv6 目录项读取方式，通过 `xargs` 练习标准输入处理与 `exec` 参数组织。整体体会是 xv6 提供的用户态库非常精简，很多在 Linux C 编程中习惯使用的库函数不可用，因此实现时要更关注底层系统调用、缓冲区大小和文件描述符关闭。
