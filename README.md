# gm2wav

基于libgme(game-music-emu)的将游戏音乐(提取)文件(Game Music，简称GM)按某几项特定需求转换为.wav音频文件的小工具。或许也可以认为是libgme的一个前端？

命令行程序，提供多种可选参数。

## 用法、参数、作用

```
Usage: /path/2/gm2wav [-div] [-A[uint16_t]] [-B[uint16_t]] [-c[uint8_t]] [-f[int]]
            [-l[uint32_t]] [-L[int]] [-m[uint8_t]] [-o[str]] [-s[int32_t]]
            [-t[int]] [--volumeA[=uint16_t]] [--volumeB[=uint16_t]]
            [--channels[=uint8_t]] [--stereo] [--fadeout_ms[=int]]
            [--loop_message] [--loop_time[=uint32_t]] [--len_max_ms[=int]]
            [--mode[=uint8_t]] [--out_dir[=str]] [--sample_rate[=int32_t]]
            [--track[=int]] [--verbose]

  -A, --volumeA[=uint16_t]
                             此数的倒数为输出音频的响度的系数，默认2，即1/2
  -B, --volumeB[=uint16_t]
                             此数为输出音频的响度的系数，默认1。A、B参数同时生效，即输出音频时响度*(B/A).
  -c, --channels[=uint8_t]   通道数(默认2)
  -d, --stereo
                             stereo模式，此选项会使channels选项失效(强制改为2)(默认不启用)
  -f, --fadeout_ms[=int]
                             若曲目本身不循环，则失效(单位ms，默认1000)
  -i, --loop_message
                             是否为wav文件写入循环信息(目前尚不支持，默认不启用)
  -l, --loop_time[=uint32_t]
                             循环节执行次数(新增功能)，如果曲目本身不循环，则失效(默认2)
  -L, --len_max_ms[=int]
                             若libgme无法正确处理会循环的gm(情况还不少)，程序则会陷入无限生成的死路。故限定一个最大播放时长。(单位ms，默认7
                             * 60 * 1000)
  -m, --mode[=uint8_t]
                             对包含多曲目的单文件的转换模式。0为全部切出(默认，此模式下-t
                             / --track选项失效)，1为选取其中之一
  -o, --out_dir[=str]        输出文件目录
  -s, --sample_rate[=int32_t]   音频采样率(单位Hz，默认44100)
  -t, --track[=int]
                             如果指定了n取1模式，则取第几Track(0数起，默认0)
  -v, --verbose              显示详细信息
  -?, --help                 Give this help list
      --usage                Give a short usage message

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

## 编译

*目前可能只在\*nix或其模拟环境`MSYS2`、`Cygwin`下可编译使用。其他平台未测试。*

### 依赖

`libsndfile`、`glibc(还有使用其error()函数、argp库)`


### 克隆仓库

```
$ git clone --recurse-submodules https://github.com/wyz-2015/gm2wav
```

### cmake构建

```
$ cd /path/2/m2wav/
$ mkdir ./build/
$ cd ./build/
$ cmake ./../
$ make -j # 未知原因，可能需要执行make命令两次
```

## 许可证

### 代码部分

LGPL >= 3

LGPL-3与GPL-3的协议文本已随附。

### `connot_haqi.txt`

`cannot_haqi.txt`是作为彩蛋用的图片文件，源为Bilibili UP主“[浅羽千绘](https://space.bilibili.com/45829182)”在网络分享的表情系列中的“不能哈气”。

我总不能宣称其为我所作，或擅自将其强行划到上述许可证下吧？
