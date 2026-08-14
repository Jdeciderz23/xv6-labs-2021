# xv6 Labs 工作日志

## 2026-08-15

- 开始 Lab util，当前分支为 `util`。
- 实验环境确认：WSL2 + Ubuntu 24.04.3 LTS，`make`、`riscv64-linux-gnu-gcc`、`qemu-system-riscv64`、`gdb-multiarch` 可用。
- `make qemu` 已能启动 xv6，并进入 `$` shell。
- 为适配 Ubuntu 24.04 的 GCC 警告策略，`Makefile` 中增加 `-Wno-error=infinite-recursion`，避免新版编译器把 xv6 既有递归警告当作错误。
- 计划完成 `sleep`、`pingpong`、`primes`、`find`、`xargs` 五个用户程序，并保存 `make grade` 输出。
- 新增 `user/sleep.c`、`user/pingpong.c`、`user/primes.c`、`user/find.c`、`user/xargs.c`，并在 `Makefile` 的 `UPROGS` 中加入对应用户程序。
- 第一次评分功能测试全部通过，但因缺少 MIT 要求的 `time.txt`，结果为 `99/100`。
- 补充 `time.txt` 后重新运行 `make grade`，最终结果为 `100/100`。
- 最终评分输出保存到 `docs/results/grade-util.txt`。

## 后续每个实验固定留存

- `make grade` 文本输出与分数。
- 关键功能运行截图。
- 关键代码修改记录。
- 当前 Git 分支、commit 号、GitHub 同步状态。
