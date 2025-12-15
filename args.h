#ifndef _ARGS_H
#define _ARGS_H

#include "common.h"

typedef struct Args {
	// const char* inGMFilePath;
	// const char* inMCUFilePath;
	const char* outDir;

	long int sampleRate;
	uint32_t loopTime; // 循环节执行次数(新增功能)，如果曲目本身不循环，则失效
	bool loopWav;	   // 是否为wav文件写入循环信息(目前尚不支持，只能显示彩蛋)
	uint8_t channels;  // channel数高起来都不太合理，限[0, 256]好了
	int fadeOut_ms;	   // 若曲目本身不循环，则失效
	bool stereo;	   // stereo模式，此选项会使channels选项失效(强制为2)

	// const char* baseName; // 输出文件时的base name
	uint8_t mode; // 模式：n 转 n (默认)、
		      // 1 * N 切 n * 1、N 取 1、(针对nsf此类文件)
		      // mcu 合 1、mcu 转 n (放弃支持)
	int trackNum; // 如果指定了n取1模式，则取第几Track(1数起)

	bool verbose; // 展示过程
} Args;

#endif
