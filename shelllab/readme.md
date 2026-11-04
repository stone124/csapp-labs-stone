# Shell Lab 实验记录

实现支持前后台作业、作业控制和信号转发的 Tiny Shell。以下记录为当前工作区实际重新编译后的测试输出。

## 验证环境与方法

- 记录日期：2026-11-04（Asia/Shanghai）。
- 系统：`Linux 6.6.114.1-microsoft-standard-WSL2 x86_64`。
- 编译器：`gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0`。
- 在 `shelllab` 目录先运行 `make -B`，再依次运行全部 16 条测试和对应参考测试。每条命令限制为 40 秒，退出码 124 表示超时。驱动退出码为 0 不代表通过。
- 比较普通输出时，仅忽略括号内的数字 PID 及 Makefile 回显的驱动命令；文字、作业编号、信号编号和顺序均参与比较。
- trace11～13 单独比较非 `ps` 输出，并核对本轮 `mysplit` 状态。运行前记录已有 `mysplit` PID，从本轮进程列表中排除旧测试残留；trace11 应无本轮存活进程，trace12 应有两个 `T` 状态进程，trace13 应先有两个 `T` 状态进程、恢复后均退出。
- 下方保留实际 PID 和进程列表。`ps` 中的编辑器连接令牌已脱敏，测试采集脚本的冗长命令参数已省略。
- 本记录不替代官方评分；注释质量和每次系统调用返回值的检查需要另行审阅。

## 编译输出

```text
$ make -B
gcc -Wall -O2    tsh.c   -o tsh
gcc -Wall -O2    myspin.c   -o myspin
gcc -Wall -O2    mysplit.c   -o mysplit
gcc -Wall -O2    mystop.c   -o mystop
gcc -Wall -O2    myint.c   -o myint
```

退出码：`0`。

## 对照结果

| Trace | tsh 退出码 | tshref 退出码 | 对照结果 |
| --- | --- | --- | --- |
| trace01 | 0 | 0 | 输出一致 |
| trace02 | 0 | 0 | 输出一致 |
| trace03 | 0 | 0 | 输出一致 |
| trace04 | 0 | 0 | 输出一致 |
| trace05 | 0 | 0 | 输出一致 |
| trace06 | 0 | 0 | 输出一致 |
| trace07 | 0 | 0 | 输出一致 |
| trace08 | 0 | 0 | 输出一致 |
| trace09 | 0 | 0 | 输出一致 |
| trace10 | 0 | 0 | 输出一致 |
| trace11 | 0 | 0 | 输出及本轮 mysplit 状态一致 |
| trace12 | 0 | 0 | 输出及本轮 mysplit 状态一致 |
| trace13 | 0 | 0 | 输出及本轮 mysplit 状态一致 |
| trace14 | 0 | 0 | 输出一致 |
| trace15 | 0 | 0 | 输出一致 |
| trace16 | 0 | 0 | 输出一致 |

## 本次结果说明

已完成 16/16 组对照，其中 16 条符合上述比较规则。

本轮全部 16 条 trace 的普通输出与参考一致；trace11～13 的本轮 `mysplit` 进程状态也符合预期。各测试驱动均正常结束。本结果为本地单轮验证，不代表官方评分或完整的并发正确性证明。

## 全部测试命令与输出

### trace01

trace01.txt - Properly terminate on EOF.

**当前实现**

```text
$ make test01
./sdriver.pl -t trace01.txt -s ./tsh -a "-p"
#
# trace01.txt - Properly terminate on EOF.
#
```

**参考实现**

```text
$ make rtest01
./sdriver.pl -t trace01.txt -s ./tshref -a "-p"
#
# trace01.txt - Properly terminate on EOF.
#
```

### trace02

trace02.txt - Process builtin quit command.

**当前实现**

```text
$ make test02
./sdriver.pl -t trace02.txt -s ./tsh -a "-p"
#
# trace02.txt - Process builtin quit command.
#
```

**参考实现**

```text
$ make rtest02
./sdriver.pl -t trace02.txt -s ./tshref -a "-p"
#
# trace02.txt - Process builtin quit command.
#
```

### trace03

trace03.txt - Run a foreground job.

**当前实现**

```text
$ make test03
./sdriver.pl -t trace03.txt -s ./tsh -a "-p"
#
# trace03.txt - Run a foreground job.
#
tsh> quit
```

**参考实现**

```text
$ make rtest03
./sdriver.pl -t trace03.txt -s ./tshref -a "-p"
#
# trace03.txt - Run a foreground job.
#
tsh> quit
```

### trace04

trace04.txt - Run a background job.

**当前实现**

```text
$ make test04
./sdriver.pl -t trace04.txt -s ./tsh -a "-p"
#
# trace04.txt - Run a background job.
#
tsh> ./myspin 1 &
[1] (170889) ./myspin 1 &
```

**参考实现**

```text
$ make rtest04
./sdriver.pl -t trace04.txt -s ./tshref -a "-p"
#
# trace04.txt - Run a background job.
#
tsh> ./myspin 1 &
[1] (170900) ./myspin 1 &
```

### trace05

trace05.txt - Process jobs builtin command.

**当前实现**

```text
$ make test05
./sdriver.pl -t trace05.txt -s ./tsh -a "-p"
#
# trace05.txt - Process jobs builtin command.
#
tsh> ./myspin 2 &
[1] (170917) ./myspin 2 &
tsh> ./myspin 3 &
[2] (170919) ./myspin 3 &
tsh> jobs
[1] (170917) Running ./myspin 2 &
[2] (170919) Running ./myspin 3 &
```

**参考实现**

```text
$ make rtest05
./sdriver.pl -t trace05.txt -s ./tshref -a "-p"
#
# trace05.txt - Process jobs builtin command.
#
tsh> ./myspin 2 &
[1] (170934) ./myspin 2 &
tsh> ./myspin 3 &
[2] (170936) ./myspin 3 &
tsh> jobs
[1] (170934) Running ./myspin 2 &
[2] (170936) Running ./myspin 3 &
```

### trace06

trace06.txt - Forward SIGINT to foreground job.

**当前实现**

```text
$ make test06
./sdriver.pl -t trace06.txt -s ./tsh -a "-p"
#
# trace06.txt - Forward SIGINT to foreground job.
#
tsh> ./myspin 4
Job [1] (170960) terminated by signal 2
```

**参考实现**

```text
$ make rtest06
./sdriver.pl -t trace06.txt -s ./tshref -a "-p"
#
# trace06.txt - Forward SIGINT to foreground job.
#
tsh> ./myspin 4
Job [1] (170973) terminated by signal 2
```

### trace07

trace07.txt - Forward SIGINT only to foreground job.

**当前实现**

```text
$ make test07
./sdriver.pl -t trace07.txt -s ./tsh -a "-p"
#
# trace07.txt - Forward SIGINT only to foreground job.
#
tsh> ./myspin 4 &
[1] (170988) ./myspin 4 &
tsh> ./myspin 5
Job [2] (170992) terminated by signal 2
tsh> jobs
[1] (170988) Running ./myspin 4 &
```

**参考实现**

```text
$ make rtest07
./sdriver.pl -t trace07.txt -s ./tshref -a "-p"
#
# trace07.txt - Forward SIGINT only to foreground job.
#
tsh> ./myspin 4 &
[1] (171006) ./myspin 4 &
tsh> ./myspin 5
Job [2] (171008) terminated by signal 2
tsh> jobs
[1] (171006) Running ./myspin 4 &
```

### trace08

trace08.txt - Forward SIGTSTP only to foreground job.

**当前实现**

```text
$ make test08
./sdriver.pl -t trace08.txt -s ./tsh -a "-p"
#
# trace08.txt - Forward SIGTSTP only to foreground job.
#
tsh> ./myspin 4 &
[1] (171032) ./myspin 4 &
tsh> ./myspin 5
Job [2] (171034) stopped by signal 20
tsh> jobs
[1] (171032) Running ./myspin 4 &
[2] (171034) Stopped ./myspin 5 
```

**参考实现**

```text
$ make rtest08
./sdriver.pl -t trace08.txt -s ./tshref -a "-p"
#
# trace08.txt - Forward SIGTSTP only to foreground job.
#
tsh> ./myspin 4 &
[1] (171059) ./myspin 4 &
tsh> ./myspin 5
Job [2] (171061) stopped by signal 20
tsh> jobs
[1] (171059) Running ./myspin 4 &
[2] (171061) Stopped ./myspin 5 
```

### trace09

trace09.txt - Process bg builtin command

**当前实现**

```text
$ make test09
./sdriver.pl -t trace09.txt -s ./tsh -a "-p"
#
# trace09.txt - Process bg builtin command
#
tsh> ./myspin 4 &
[1] (171080) ./myspin 4 &
tsh> ./myspin 5
Job [2] (171082) stopped by signal 20
tsh> jobs
[1] (171080) Running ./myspin 4 &
[2] (171082) Stopped ./myspin 5 
tsh> bg %2
[2] (171082) ./myspin 5 
tsh> jobs
[1] (171080) Running ./myspin 4 &
[2] (171082) Running ./myspin 5 
```

**参考实现**

```text
$ make rtest09
./sdriver.pl -t trace09.txt -s ./tshref -a "-p"
#
# trace09.txt - Process bg builtin command
#
tsh> ./myspin 4 &
[1] (171115) ./myspin 4 &
tsh> ./myspin 5
Job [2] (171117) stopped by signal 20
tsh> jobs
[1] (171115) Running ./myspin 4 &
[2] (171117) Stopped ./myspin 5 
tsh> bg %2
[2] (171117) ./myspin 5 
tsh> jobs
[1] (171115) Running ./myspin 4 &
[2] (171117) Running ./myspin 5 
```

### trace10

trace10.txt - Process fg builtin command.

**当前实现**

```text
$ make test10
./sdriver.pl -t trace10.txt -s ./tsh -a "-p"
#
# trace10.txt - Process fg builtin command. 
#
tsh> ./myspin 4 &
[1] (171147) ./myspin 4 &
tsh> fg %1
Job [1] (171147) stopped by signal 20
tsh> jobs
[1] (171147) Stopped ./myspin 4 &
tsh> fg %1
tsh> jobs
```

**参考实现**

```text
$ make rtest10
./sdriver.pl -t trace10.txt -s ./tshref -a "-p"
#
# trace10.txt - Process fg builtin command. 
#
tsh> ./myspin 4 &
[1] (171175) ./myspin 4 &
tsh> fg %1
Job [1] (171175) stopped by signal 20
tsh> jobs
[1] (171175) Stopped ./myspin 4 &
tsh> fg %1
tsh> jobs
```

### trace11

trace11.txt - Forward SIGINT to every process in foreground process group

本轮 `mysplit` 状态快照（按 `ps` 出现顺序）：当前实现 `[[]]`；参考实现 `[[]]`。

**当前实现**

```text
$ make test11
./sdriver.pl -t trace11.txt -s ./tsh -a "-p"
#
# trace11.txt - Forward SIGINT to every process in foreground process group
#
tsh> ./mysplit 4
Job [1] (171198) terminated by signal 2
tsh> /bin/ps a
    PID TTY      STAT   TIME COMMAND
      2 hvc0     Sl+    0:00 /init
      8 hvc0     Sl+    1:25 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
    263 tty1     Ss+    0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
    417 pts/1    Ss+    0:00 -bash
  96480 pts/2    Ss+    0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96481 pts/2    S+     0:00 sh /mnt/c/Users/g1395/.vscode/extensions/ms-vscode-remote.remote-wsl-1.104.7/scripts/wslServer.sh 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96487 pts/2    S+     0:00 sh /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/bin/code-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96491 pts/2    Sl+    0:33 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/server-main.js --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96505 pts/3    Ssl+   0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96514 pts/4    Ssl+   0:23 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96540 pts/2    Sl+    0:05 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=fileWatcher
  96862 pts/2    Sl+    9:44 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node --dns-result-order=ipv4first /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=true
  96868 pts/2    Sl+    0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=agentHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915 --user-data-dir /home/stone/.vscode-server/data --telemetry-level all
  96886 pts/2    Sl+    0:29 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=ptyHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915
  96901 pts/2    Sl+    0:04 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node_modules/@github/copilot-sdk-linux-x64/prebuilds/linux-x64/copilot-runtime --headless --no-auto-update --log-level info --stdio --no-auto-login
  96949 pts/2    Sl+    0:37 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex -c features.code_mode_host=true -c features.windows_sandbox_service=true app-server --analytics-default-enabled
  97529 pts/2    Sl+    1:29 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools
  97761 pts/8    Ss+    0:00 /bin/bash --init-file /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/vs/workbench/contrib/terminal/common/scripts/shellIntegration-bash.sh
  98014 pts/2    Sl+    3:50 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv 97529 {AE07E2A9-96D4-49E2-829F-97F393A165BA}
  98635 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {E0EBEA3C-124B-4050-A3CD-87EEEC0E1225} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
  98658 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {77022EC9-C66A-41B5-BEE7-BE67179451C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 107503 pts/2    Sl     0:00 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex-code-mode-host
 111866 pts/8    S      0:00 ./tsh -p
 111868 pts/8    T      0:00 ./myspin 4
 163839 pts/2    Sl+    0:00 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/extensions/markdown-language-features/dist/serverWorkerMain --node-ipc --clientProcessId=96862
 166629 pts/2    Sl+    0:00 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {5721A1E4-1A2C-43D0-ADBA-85817D6FD3C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 170824 pts/5    Ss+    0:00 [本次测试采集脚本，命令参数省略]
 170825 pts/5    S+     0:00 python3 -
 171193 pts/5    S+     0:00 make test11
 171194 pts/5    S+     0:00 /bin/sh -c ./sdriver.pl -t trace11.txt -s ./tsh -a "-p"
 171195 pts/5    S+     0:00 /usr/bin/perl ./sdriver.pl -t trace11.txt -s ./tsh -a -p
 171196 pts/5    S+     0:00 ./tsh -p
 171213 pts/5    R      0:00 /bin/ps a
```

**参考实现**

```text
$ make rtest11
./sdriver.pl -t trace11.txt -s ./tshref -a "-p"
#
# trace11.txt - Forward SIGINT to every process in foreground process group
#
tsh> ./mysplit 4
Job [1] (171219) terminated by signal 2
tsh> /bin/ps a
    PID TTY      STAT   TIME COMMAND
      2 hvc0     Sl+    0:00 /init
      8 hvc0     Sl+    1:25 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
    263 tty1     Ss+    0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
    417 pts/1    Ss+    0:00 -bash
  96480 pts/2    Ss+    0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96481 pts/2    S+     0:00 sh /mnt/c/Users/g1395/.vscode/extensions/ms-vscode-remote.remote-wsl-1.104.7/scripts/wslServer.sh 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96487 pts/2    S+     0:00 sh /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/bin/code-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96491 pts/2    Sl+    0:33 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/server-main.js --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96505 pts/3    Ssl+   0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96514 pts/4    Ssl+   0:23 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96540 pts/2    Sl+    0:05 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=fileWatcher
  96862 pts/2    Sl+    9:44 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node --dns-result-order=ipv4first /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=true
  96868 pts/2    Sl+    0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=agentHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915 --user-data-dir /home/stone/.vscode-server/data --telemetry-level all
  96886 pts/2    Sl+    0:29 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=ptyHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915
  96901 pts/2    Sl+    0:04 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node_modules/@github/copilot-sdk-linux-x64/prebuilds/linux-x64/copilot-runtime --headless --no-auto-update --log-level info --stdio --no-auto-login
  96949 pts/2    Sl+    0:37 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex -c features.code_mode_host=true -c features.windows_sandbox_service=true app-server --analytics-default-enabled
  97529 pts/2    Sl+    1:29 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools
  97761 pts/8    Ss+    0:00 /bin/bash --init-file /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/vs/workbench/contrib/terminal/common/scripts/shellIntegration-bash.sh
  98014 pts/2    Sl+    3:50 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv 97529 {AE07E2A9-96D4-49E2-829F-97F393A165BA}
  98635 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {E0EBEA3C-124B-4050-A3CD-87EEEC0E1225} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
  98658 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {77022EC9-C66A-41B5-BEE7-BE67179451C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 107503 pts/2    Sl     0:00 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex-code-mode-host
 111866 pts/8    S      0:00 ./tsh -p
 111868 pts/8    T      0:00 ./myspin 4
 163839 pts/2    Sl+    0:00 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/extensions/markdown-language-features/dist/serverWorkerMain --node-ipc --clientProcessId=96862
 166629 pts/2    Sl+    0:00 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {5721A1E4-1A2C-43D0-ADBA-85817D6FD3C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 170824 pts/5    Ss+    0:00 [本次测试采集脚本，命令参数省略]
 170825 pts/5    S+     0:00 python3 -
 171214 pts/5    S+     0:00 make rtest11
 171215 pts/5    S+     0:00 /bin/sh -c ./sdriver.pl -t trace11.txt -s ./tshref -a "-p"
 171216 pts/5    S+     0:00 /usr/bin/perl ./sdriver.pl -t trace11.txt -s ./tshref -a -p
 171217 pts/5    S+     0:00 ./tshref -p
 171228 pts/5    R      0:00 /bin/ps a
```

### trace12

trace12.txt - Forward SIGTSTP to every process in foreground process group

本轮 `mysplit` 状态快照（按 `ps` 出现顺序）：当前实现 `[[(171234, 'T'), (171235, 'T')]]`；参考实现 `[[(171254, 'T'), (171255, 'T')]]`。

**当前实现**

```text
$ make test12
./sdriver.pl -t trace12.txt -s ./tsh -a "-p"
#
# trace12.txt - Forward SIGTSTP to every process in foreground process group
#
tsh> ./mysplit 4
Job [1] (171234) stopped by signal 20
tsh> jobs
[1] (171234) Stopped ./mysplit 4 
tsh> /bin/ps a
    PID TTY      STAT   TIME COMMAND
      2 hvc0     Sl+    0:00 /init
      8 hvc0     Sl+    1:25 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
    263 tty1     Ss+    0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
    417 pts/1    Ss+    0:00 -bash
  96480 pts/2    Ss+    0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96481 pts/2    S+     0:00 sh /mnt/c/Users/g1395/.vscode/extensions/ms-vscode-remote.remote-wsl-1.104.7/scripts/wslServer.sh 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96487 pts/2    S+     0:00 sh /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/bin/code-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96491 pts/2    Sl+    0:33 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/server-main.js --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96505 pts/3    Ssl+   0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96514 pts/4    Ssl+   0:23 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96540 pts/2    Sl+    0:05 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=fileWatcher
  96862 pts/2    Sl+    9:44 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node --dns-result-order=ipv4first /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=true
  96868 pts/2    Sl+    0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=agentHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915 --user-data-dir /home/stone/.vscode-server/data --telemetry-level all
  96886 pts/2    Sl+    0:29 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=ptyHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915
  96901 pts/2    Sl+    0:04 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node_modules/@github/copilot-sdk-linux-x64/prebuilds/linux-x64/copilot-runtime --headless --no-auto-update --log-level info --stdio --no-auto-login
  96949 pts/2    Sl+    0:38 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex -c features.code_mode_host=true -c features.windows_sandbox_service=true app-server --analytics-default-enabled
  97529 pts/2    Sl+    1:29 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools
  97761 pts/8    Ss+    0:00 /bin/bash --init-file /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/vs/workbench/contrib/terminal/common/scripts/shellIntegration-bash.sh
  98014 pts/2    Sl+    3:50 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv 97529 {AE07E2A9-96D4-49E2-829F-97F393A165BA}
  98635 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {E0EBEA3C-124B-4050-A3CD-87EEEC0E1225} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
  98658 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {77022EC9-C66A-41B5-BEE7-BE67179451C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 107503 pts/2    Sl     0:00 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex-code-mode-host
 111866 pts/8    S      0:00 ./tsh -p
 111868 pts/8    T      0:00 ./myspin 4
 163839 pts/2    Sl+    0:00 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/extensions/markdown-language-features/dist/serverWorkerMain --node-ipc --clientProcessId=96862
 166629 pts/2    Sl+    0:00 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {5721A1E4-1A2C-43D0-ADBA-85817D6FD3C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 170824 pts/5    Ss+    0:00 [本次测试采集脚本，命令参数省略]
 170825 pts/5    S+     0:00 python3 -
 171229 pts/5    S+     0:00 make test12
 171230 pts/5    S+     0:00 /bin/sh -c ./sdriver.pl -t trace12.txt -s ./tsh -a "-p"
 171231 pts/5    S+     0:00 /usr/bin/perl ./sdriver.pl -t trace12.txt -s ./tsh -a -p
 171232 pts/5    S+     0:00 ./tsh -p
 171234 pts/5    T      0:00 ./mysplit 4
 171235 pts/5    T      0:00 ./mysplit 4
 171248 pts/5    R      0:00 /bin/ps a
```

**参考实现**

```text
$ make rtest12
./sdriver.pl -t trace12.txt -s ./tshref -a "-p"
#
# trace12.txt - Forward SIGTSTP to every process in foreground process group
#
tsh> ./mysplit 4
Job [1] (171254) stopped by signal 20
tsh> jobs
[1] (171254) Stopped ./mysplit 4 
tsh> /bin/ps a
    PID TTY      STAT   TIME COMMAND
      2 hvc0     Sl+    0:00 /init
      8 hvc0     Sl+    1:25 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
    263 tty1     Ss+    0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
    417 pts/1    Ss+    0:00 -bash
  96480 pts/2    Ss+    0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96481 pts/2    S+     0:00 sh /mnt/c/Users/g1395/.vscode/extensions/ms-vscode-remote.remote-wsl-1.104.7/scripts/wslServer.sh 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96487 pts/2    S+     0:00 sh /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/bin/code-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96491 pts/2    Sl+    0:33 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/server-main.js --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96505 pts/3    Ssl+   0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96514 pts/4    Ssl+   0:23 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96540 pts/2    Sl+    0:05 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=fileWatcher
  96862 pts/2    Sl+    9:44 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node --dns-result-order=ipv4first /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=true
  96868 pts/2    Sl+    0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=agentHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915 --user-data-dir /home/stone/.vscode-server/data --telemetry-level all
  96886 pts/2    Sl+    0:29 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=ptyHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915
  96901 pts/2    Sl+    0:04 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node_modules/@github/copilot-sdk-linux-x64/prebuilds/linux-x64/copilot-runtime --headless --no-auto-update --log-level info --stdio --no-auto-login
  96949 pts/2    Sl+    0:38 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex -c features.code_mode_host=true -c features.windows_sandbox_service=true app-server --analytics-default-enabled
  97529 pts/2    Sl+    1:29 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools
  97761 pts/8    Ss+    0:00 /bin/bash --init-file /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/vs/workbench/contrib/terminal/common/scripts/shellIntegration-bash.sh
  98014 pts/2    Sl+    3:50 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv 97529 {AE07E2A9-96D4-49E2-829F-97F393A165BA}
  98635 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {E0EBEA3C-124B-4050-A3CD-87EEEC0E1225} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
  98658 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {77022EC9-C66A-41B5-BEE7-BE67179451C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 107503 pts/2    Sl     0:00 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex-code-mode-host
 111866 pts/8    S      0:00 ./tsh -p
 111868 pts/8    T      0:00 ./myspin 4
 163839 pts/2    Sl+    0:00 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/extensions/markdown-language-features/dist/serverWorkerMain --node-ipc --clientProcessId=96862
 166629 pts/2    Sl+    0:00 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {5721A1E4-1A2C-43D0-ADBA-85817D6FD3C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 170824 pts/5    Ss+    0:00 [本次测试采集脚本，命令参数省略]
 170825 pts/5    S+     0:00 python3 -
 171249 pts/5    S+     0:00 make rtest12
 171250 pts/5    S+     0:00 /bin/sh -c ./sdriver.pl -t trace12.txt -s ./tshref -a "-p"
 171251 pts/5    S+     0:00 /usr/bin/perl ./sdriver.pl -t trace12.txt -s ./tshref -a -p
 171252 pts/5    S+     0:00 ./tshref -p
 171254 pts/5    T      0:00 ./mysplit 4
 171255 pts/5    T      0:00 ./mysplit 4
 171271 pts/5    R      0:00 /bin/ps a
```

### trace13

trace13.txt - Restart every stopped process in process group

本轮 `mysplit` 状态快照（按 `ps` 出现顺序）：当前实现 `[[(171277, 'T'), (171278, 'T')], []]`；参考实现 `[[(171308, 'T'), (171309, 'T')], []]`。

**当前实现**

```text
$ make test13
./sdriver.pl -t trace13.txt -s ./tsh -a "-p"
#
# trace13.txt - Restart every stopped process in process group
#
tsh> ./mysplit 4
Job [1] (171277) stopped by signal 20
tsh> jobs
[1] (171277) Stopped ./mysplit 4 
tsh> /bin/ps a
    PID TTY      STAT   TIME COMMAND
      2 hvc0     Sl+    0:00 /init
      8 hvc0     Sl+    1:25 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
    263 tty1     Ss+    0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
    417 pts/1    Ss+    0:00 -bash
  96480 pts/2    Ss+    0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96481 pts/2    S+     0:00 sh /mnt/c/Users/g1395/.vscode/extensions/ms-vscode-remote.remote-wsl-1.104.7/scripts/wslServer.sh 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96487 pts/2    S+     0:00 sh /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/bin/code-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96491 pts/2    Sl+    0:33 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/server-main.js --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96505 pts/3    Ssl+   0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96514 pts/4    Ssl+   0:23 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96540 pts/2    Sl+    0:05 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=fileWatcher
  96862 pts/2    Sl+    9:44 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node --dns-result-order=ipv4first /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=true
  96868 pts/2    Sl+    0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=agentHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915 --user-data-dir /home/stone/.vscode-server/data --telemetry-level all
  96886 pts/2    Sl+    0:29 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=ptyHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915
  96901 pts/2    Sl+    0:04 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node_modules/@github/copilot-sdk-linux-x64/prebuilds/linux-x64/copilot-runtime --headless --no-auto-update --log-level info --stdio --no-auto-login
  96949 pts/2    Sl+    0:38 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex -c features.code_mode_host=true -c features.windows_sandbox_service=true app-server --analytics-default-enabled
  97529 pts/2    Sl+    1:29 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools
  97761 pts/8    Ss+    0:00 /bin/bash --init-file /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/vs/workbench/contrib/terminal/common/scripts/shellIntegration-bash.sh
  98014 pts/2    Sl+    3:50 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv 97529 {AE07E2A9-96D4-49E2-829F-97F393A165BA}
  98635 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {E0EBEA3C-124B-4050-A3CD-87EEEC0E1225} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
  98658 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {77022EC9-C66A-41B5-BEE7-BE67179451C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 107503 pts/2    Sl     0:00 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex-code-mode-host
 111866 pts/8    S      0:00 ./tsh -p
 111868 pts/8    T      0:00 ./myspin 4
 163839 pts/2    Sl+    0:00 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/extensions/markdown-language-features/dist/serverWorkerMain --node-ipc --clientProcessId=96862
 166629 pts/2    Sl+    0:00 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {5721A1E4-1A2C-43D0-ADBA-85817D6FD3C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 170824 pts/5    Ss+    0:00 [本次测试采集脚本，命令参数省略]
 170825 pts/5    S+     0:00 python3 -
 171272 pts/5    S+     0:00 make test13
 171273 pts/5    S+     0:00 /bin/sh -c ./sdriver.pl -t trace13.txt -s ./tsh -a "-p"
 171274 pts/5    S+     0:00 /usr/bin/perl ./sdriver.pl -t trace13.txt -s ./tsh -a -p
 171275 pts/5    S+     0:00 ./tsh -p
 171277 pts/5    T      0:00 ./mysplit 4
 171278 pts/5    T      0:00 ./mysplit 4
 171288 pts/5    R      0:00 /bin/ps a
tsh> fg %1
tsh> /bin/ps a
    PID TTY      STAT   TIME COMMAND
      2 hvc0     Sl+    0:00 /init
      8 hvc0     Sl+    1:25 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
    263 tty1     Ss+    0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
    417 pts/1    Ss+    0:00 -bash
  96480 pts/2    Ss+    0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96481 pts/2    S+     0:00 sh /mnt/c/Users/g1395/.vscode/extensions/ms-vscode-remote.remote-wsl-1.104.7/scripts/wslServer.sh 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96487 pts/2    S+     0:00 sh /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/bin/code-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96491 pts/2    Sl+    0:33 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/server-main.js --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96505 pts/3    Ssl+   0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96514 pts/4    Ssl+   0:23 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96540 pts/2    Sl+    0:05 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=fileWatcher
  96862 pts/2    Sl+    9:44 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node --dns-result-order=ipv4first /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=true
  96868 pts/2    Sl+    0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=agentHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915 --user-data-dir /home/stone/.vscode-server/data --telemetry-level all
  96886 pts/2    Sl+    0:29 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=ptyHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915
  96901 pts/2    Sl+    0:04 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node_modules/@github/copilot-sdk-linux-x64/prebuilds/linux-x64/copilot-runtime --headless --no-auto-update --log-level info --stdio --no-auto-login
  96949 pts/2    Sl+    0:38 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex -c features.code_mode_host=true -c features.windows_sandbox_service=true app-server --analytics-default-enabled
  97529 pts/2    Sl+    1:29 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools
  97761 pts/8    Ss+    0:00 /bin/bash --init-file /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/vs/workbench/contrib/terminal/common/scripts/shellIntegration-bash.sh
  98014 pts/2    Sl+    3:50 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv 97529 {AE07E2A9-96D4-49E2-829F-97F393A165BA}
  98635 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {E0EBEA3C-124B-4050-A3CD-87EEEC0E1225} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
  98658 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {77022EC9-C66A-41B5-BEE7-BE67179451C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 107503 pts/2    Sl     0:00 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex-code-mode-host
 111866 pts/8    S      0:00 ./tsh -p
 111868 pts/8    T      0:00 ./myspin 4
 163839 pts/2    Sl+    0:00 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/extensions/markdown-language-features/dist/serverWorkerMain --node-ipc --clientProcessId=96862
 166629 pts/2    Sl+    0:00 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {5721A1E4-1A2C-43D0-ADBA-85817D6FD3C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 170824 pts/5    Ss+    0:00 [本次测试采集脚本，命令参数省略]
 170825 pts/5    S+     0:00 python3 -
 171272 pts/5    S+     0:00 make test13
 171273 pts/5    S+     0:00 /bin/sh -c ./sdriver.pl -t trace13.txt -s ./tsh -a "-p"
 171274 pts/5    S+     0:00 /usr/bin/perl ./sdriver.pl -t trace13.txt -s ./tsh -a -p
 171275 pts/5    S+     0:00 ./tsh -p
 171302 pts/5    R      0:00 /bin/ps a
```

**参考实现**

```text
$ make rtest13
./sdriver.pl -t trace13.txt -s ./tshref -a "-p"
#
# trace13.txt - Restart every stopped process in process group
#
tsh> ./mysplit 4
Job [1] (171308) stopped by signal 20
tsh> jobs
[1] (171308) Stopped ./mysplit 4 
tsh> /bin/ps a
    PID TTY      STAT   TIME COMMAND
      2 hvc0     Sl+    0:00 /init
      8 hvc0     Sl+    1:25 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
    263 tty1     Ss+    0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
    417 pts/1    Ss+    0:00 -bash
  96480 pts/2    Ss+    0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96481 pts/2    S+     0:00 sh /mnt/c/Users/g1395/.vscode/extensions/ms-vscode-remote.remote-wsl-1.104.7/scripts/wslServer.sh 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96487 pts/2    S+     0:00 sh /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/bin/code-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96491 pts/2    Sl+    0:33 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/server-main.js --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96505 pts/3    Ssl+   0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96514 pts/4    Ssl+   0:23 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96540 pts/2    Sl+    0:05 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=fileWatcher
  96862 pts/2    Sl+    9:44 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node --dns-result-order=ipv4first /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=true
  96868 pts/2    Sl+    0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=agentHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915 --user-data-dir /home/stone/.vscode-server/data --telemetry-level all
  96886 pts/2    Sl+    0:29 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=ptyHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915
  96901 pts/2    Sl+    0:04 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node_modules/@github/copilot-sdk-linux-x64/prebuilds/linux-x64/copilot-runtime --headless --no-auto-update --log-level info --stdio --no-auto-login
  96949 pts/2    Sl+    0:38 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex -c features.code_mode_host=true -c features.windows_sandbox_service=true app-server --analytics-default-enabled
  97529 pts/2    Sl+    1:29 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools
  97761 pts/8    Ss+    0:00 /bin/bash --init-file /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/vs/workbench/contrib/terminal/common/scripts/shellIntegration-bash.sh
  98014 pts/2    Sl+    3:50 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv 97529 {AE07E2A9-96D4-49E2-829F-97F393A165BA}
  98635 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {E0EBEA3C-124B-4050-A3CD-87EEEC0E1225} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
  98658 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {77022EC9-C66A-41B5-BEE7-BE67179451C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 107503 pts/2    Sl     0:00 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex-code-mode-host
 111866 pts/8    S      0:00 ./tsh -p
 111868 pts/8    T      0:00 ./myspin 4
 163839 pts/2    Sl+    0:00 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/extensions/markdown-language-features/dist/serverWorkerMain --node-ipc --clientProcessId=96862
 166629 pts/2    Sl+    0:00 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {5721A1E4-1A2C-43D0-ADBA-85817D6FD3C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 170824 pts/5    Ss+    0:00 [本次测试采集脚本，命令参数省略]
 170825 pts/5    S+     0:00 python3 -
 171303 pts/5    S+     0:00 make rtest13
 171304 pts/5    S+     0:00 /bin/sh -c ./sdriver.pl -t trace13.txt -s ./tshref -a "-p"
 171305 pts/5    S+     0:00 /usr/bin/perl ./sdriver.pl -t trace13.txt -s ./tshref -a -p
 171306 pts/5    S+     0:00 ./tshref -p
 171308 pts/5    T      0:00 ./mysplit 4
 171309 pts/5    T      0:00 ./mysplit 4
 171318 pts/5    R      0:00 /bin/ps a
tsh> fg %1
tsh> /bin/ps a
    PID TTY      STAT   TIME COMMAND
      2 hvc0     Sl+    0:00 /init
      8 hvc0     Sl+    1:25 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
    263 tty1     Ss+    0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
    417 pts/1    Ss+    0:00 -bash
  96480 pts/2    Ss+    0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96481 pts/2    S+     0:00 sh /mnt/c/Users/g1395/.vscode/extensions/ms-vscode-remote.remote-wsl-1.104.7/scripts/wslServer.sh 261c84cb1eddf11235f9e2c921425e9663fbe556 stable code-server .vscode-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96487 pts/2    S+     0:00 sh /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/bin/code-server --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96491 pts/2    Sl+    0:33 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/server-main.js --host=127.0.0.1 --port=0 --connection-token=<REDACTED> --use-host-proxy --without-browser-env-var --disable-websocket-compression --accept-server-license-terms --telemetry-level=all
  96505 pts/3    Ssl+   0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96514 pts/4    Ssl+   0:23 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node -e const net = require('net'); process.stdin.pause(); const client = net.createConnection({ host: '127.0.0.1', port: 43035 }, () => { client.pipe(process.stdout); process.stdin.pipe(client); }); client.on('close', function (hadError) { console.error(hadError ? 'Remote close with error' : 'Remote close'); process.exit(hadError ? 1 : 0); }); client.on('error', function (err) { process.stderr.write(err && (err.stack || err.message) || String(err)); });
  96540 pts/2    Sl+    0:05 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=fileWatcher
  96862 pts/2    Sl+    9:45 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node --dns-result-order=ipv4first /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=true
  96868 pts/2    Sl+    0:06 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=agentHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915 --user-data-dir /home/stone/.vscode-server/data --telemetry-level all
  96886 pts/2    Sl+    0:29 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/bootstrap-fork --type=ptyHost --logsPath /home/stone/.vscode-server/data/logs/20261104T095915
  96901 pts/2    Sl+    0:04 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node_modules/@github/copilot-sdk-linux-x64/prebuilds/linux-x64/copilot-runtime --headless --no-auto-update --log-level info --stdio --no-auto-login
  96949 pts/2    Sl+    0:38 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex -c features.code_mode_host=true -c features.windows_sandbox_service=true app-server --analytics-default-enabled
  97529 pts/2    Sl+    1:29 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools
  97761 pts/8    Ss+    0:00 /bin/bash --init-file /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/out/vs/workbench/contrib/terminal/common/scripts/shellIntegration-bash.sh
  98014 pts/2    Sl+    3:50 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv 97529 {AE07E2A9-96D4-49E2-829F-97F393A165BA}
  98635 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {E0EBEA3C-124B-4050-A3CD-87EEEC0E1225} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
  98658 pts/2    Sl+    0:07 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {77022EC9-C66A-41B5-BEE7-BE67179451C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 107503 pts/2    Sl     0:00 /home/stone/.vscode-server/extensions/openai.chatgpt-26.1021.22027-linux-x64/bin/linux-x86_64/codex-code-mode-host
 111866 pts/8    S      0:00 ./tsh -p
 111868 pts/8    T      0:00 ./myspin 4
 163839 pts/2    Sl+    0:00 /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/node /home/stone/.vscode-server/bin/261c84cb1eddf11235f9e2c921425e9663fbe556/extensions/markdown-language-features/dist/serverWorkerMain --node-ipc --clientProcessId=96862
 166629 pts/2    Sl+    0:00 /home/stone/.vscode-server/extensions/ms-vscode.cpptools-1.35.6-linux-x64/bin/cpptools-srv2 -s {5721A1E4-1A2C-43D0-ADBA-85817D6FD3C3} -p 97529 -i TagParser -t /tmp/.vscode-cpptools-1000/cpptools/crashes/2de5c61b1b644dad8cbc812dc35f6000
 170824 pts/5    Ss+    0:00 [本次测试采集脚本，命令参数省略]
 170825 pts/5    S+     0:00 python3 -
 171303 pts/5    S+     0:00 make rtest13
 171304 pts/5    S+     0:00 /bin/sh -c ./sdriver.pl -t trace13.txt -s ./tshref -a "-p"
 171305 pts/5    S+     0:00 /usr/bin/perl ./sdriver.pl -t trace13.txt -s ./tshref -a -p
 171306 pts/5    S+     0:00 ./tshref -p
 171327 pts/5    R      0:00 /bin/ps a
```

### trace14

trace14.txt - Simple error handling

**当前实现**

```text
$ make test14
./sdriver.pl -t trace14.txt -s ./tsh -a "-p"
#
# trace14.txt - Simple error handling
#
tsh> ./bogus
./bogus: Command not found
tsh> ./myspin 4 &
[1] (171335) ./myspin 4 &
tsh> fg
fg command requires PID or %jobid argument
tsh> bg
bg command requires PID or %jobid argument
tsh> fg a
fg: argument must be a PID or %jobid
tsh> bg a
bg: argument must be a PID or %jobid
tsh> fg 9999999
(9999999): No such process
tsh> bg 9999999
(9999999): No such process
tsh> fg %2
%2: No such job
tsh> fg %1
Job [1] (171335) stopped by signal 20
tsh> bg %2
%2: No such job
tsh> bg %1
[1] (171335) ./myspin 4 &
tsh> jobs
[1] (171335) Running ./myspin 4 &
```

**参考实现**

```text
$ make rtest14
./sdriver.pl -t trace14.txt -s ./tshref -a "-p"
#
# trace14.txt - Simple error handling
#
tsh> ./bogus
./bogus: Command not found
tsh> ./myspin 4 &
[1] (171375) ./myspin 4 &
tsh> fg
fg command requires PID or %jobid argument
tsh> bg
bg command requires PID or %jobid argument
tsh> fg a
fg: argument must be a PID or %jobid
tsh> bg a
bg: argument must be a PID or %jobid
tsh> fg 9999999
(9999999): No such process
tsh> bg 9999999
(9999999): No such process
tsh> fg %2
%2: No such job
tsh> fg %1
Job [1] (171375) stopped by signal 20
tsh> bg %2
%2: No such job
tsh> bg %1
[1] (171375) ./myspin 4 &
tsh> jobs
[1] (171375) Running ./myspin 4 &
```

### trace15

trace15.txt - Putting it all together

**当前实现**

```text
$ make test15
./sdriver.pl -t trace15.txt -s ./tsh -a "-p"
#
# trace15.txt - Putting it all together
#
tsh> ./bogus
./bogus: Command not found
tsh> ./myspin 10
Job [1] (171411) terminated by signal 2
tsh> ./myspin 3 &
[1] (171426) ./myspin 3 &
tsh> ./myspin 4 &
[2] (171428) ./myspin 4 &
tsh> jobs
[1] (171426) Running ./myspin 3 &
[2] (171428) Running ./myspin 4 &
tsh> fg %1
Job [1] (171426) stopped by signal 20
tsh> jobs
[1] (171426) Stopped ./myspin 3 &
[2] (171428) Running ./myspin 4 &
tsh> bg %3
%3: No such job
tsh> bg %1
[1] (171426) ./myspin 3 &
tsh> jobs
[1] (171426) Running ./myspin 3 &
[2] (171428) Running ./myspin 4 &
tsh> fg %1
tsh> quit
```

**参考实现**

```text
$ make rtest15
./sdriver.pl -t trace15.txt -s ./tshref -a "-p"
#
# trace15.txt - Putting it all together
#
tsh> ./bogus
./bogus: Command not found
tsh> ./myspin 10
Job [1] (171456) terminated by signal 2
tsh> ./myspin 3 &
[1] (171464) ./myspin 3 &
tsh> ./myspin 4 &
[2] (171466) ./myspin 4 &
tsh> jobs
[1] (171464) Running ./myspin 3 &
[2] (171466) Running ./myspin 4 &
tsh> fg %1
Job [1] (171464) stopped by signal 20
tsh> jobs
[1] (171464) Stopped ./myspin 3 &
[2] (171466) Running ./myspin 4 &
tsh> bg %3
%3: No such job
tsh> bg %1
[1] (171464) ./myspin 3 &
tsh> jobs
[1] (171464) Running ./myspin 3 &
[2] (171466) Running ./myspin 4 &
tsh> fg %1
tsh> quit
```

### trace16

trace16.txt - Tests whether the shell can handle SIGTSTP and SIGINT

**当前实现**

```text
$ make test16
./sdriver.pl -t trace16.txt -s ./tsh -a "-p"
#
# trace16.txt - Tests whether the shell can handle SIGTSTP and SIGINT
#     signals that come from other processes instead of the terminal.
#
tsh> ./mystop 2
Job [1] (171492) stopped by signal 20
tsh> jobs
[1] (171492) Stopped ./mystop 2
tsh> ./myint 2
Job [2] (171507) terminated by signal 2
```

**参考实现**

```text
$ make rtest16
./sdriver.pl -t trace16.txt -s ./tshref -a "-p"
#
# trace16.txt - Tests whether the shell can handle SIGTSTP and SIGINT
#     signals that come from other processes instead of the terminal.
#
tsh> ./mystop 2
Job [1] (171519) stopped by signal 20
tsh> jobs
[1] (171519) Stopped ./mystop 2
tsh> ./myint 2
Job [2] (171534) terminated by signal 2
```

