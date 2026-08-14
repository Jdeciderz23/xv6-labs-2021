# xv6 Labs 工作日志

## 2026-08-15：Lab util

- 当前分支：`util`；GitHub 仓库：<https://github.com/Jdeciderz23/xv6-labs-2021>。
- 环境确认：WSL2 + Ubuntu 24.04.3 LTS，`make`、`riscv64-linux-gnu-gcc`、`qemu-system-riscv64`、`gdb-multiarch` 可用。
- `make qemu` 已能启动 xv6，并进入 `$` shell。
- 为适配 Ubuntu 24.04 的 GCC 警告策略，`Makefile` 增加 `-Wno-error=infinite-recursion`。
- 完成 `user/sleep.c`、`user/pingpong.c`、`user/primes.c`、`user/find.c`、`user/xargs.c`，并加入 `Makefile` 的 `UPROGS`。
- 初次评分功能测试全部通过，但因缺少 MIT 要求的 `time.txt`，结果为 `99/100`。
- 补充 `time.txt` 后重新运行 `make grade`，最终结果为 `100/100`。
- 评分文本保存至 `docs/results/grade-util.txt`。
- 关键运行截图保存至 `docs/results/screenshots/util/util-demo.png`。
- 评分截图保存至 `docs/results/screenshots/util/util-grade-100.png`。
- Word 报告保存至 `docs/xv6-labs-2021-report.docx`，Markdown 报告同步更新至 `docs/report.md`。

## 后续每个实验固定留存

1. `make grade` 完整输出和最终分数。
2. 关键功能运行截图，通常 1-2 张即可。
3. 关键代码修改记录和问题解决过程。
4. 报告对应章节和本工作日志条目。
5. 当前 Git 分支、commit 号和 GitHub 同步状态。
