# Dummy Unix File System (beta)

Note:  
UnixV6のファイルシステムをC言語で擬似的に再現したもの。  
テストデータ(TestBlockDevice)は、UnixV6のファイル構造になっている。  
当ファイルシステムは、(bashで使用されるような)各コマンドを実装している。  
※beta版のため、稚拙なコードです。  

## 実行方法
```
$ make  
$ ./dummyFS
```
or
```
$ make run
```

## コマンドの種類
- ls
- ls -l
- cd
- cat

## 開発環境
macOS Sierra (10.12.3)  
gcc version : 4.2.1

##  ※追記
リファクタリングして、C++版で正式版を作成しました。  
<https://github.com/kohga/dummy-unix-filesystem>
