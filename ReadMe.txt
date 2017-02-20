----
内容

UnixV6のファイルシステムを擬似的に再現したもの。
テストデータ(TestBlockDevice)は、UnixV6のファイル構造になっている。
当ファイルシステムは、(bashで使用されるような)各コマンドを実装している。

----
実行方法

$ make
$ ./dummyFS

or

$ make run

----
コマンドの種類
・ ls
・ ls -l
・ cd
・ cat

----
開発環境

macOS Sierra (10.12.3)
gcc version : 4.2.1

----
C++版も開発予定
