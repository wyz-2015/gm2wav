#ifndef _CONVERTER_H
#define _CONVERTER_H

// 还是把类定义及所有方法都放到.h文件这里吧，可任意使用

#include "args.h"
#include "common.h"
#include "playlist.h"
#include <gme/gme.h>
#include <sndfile.h>

typedef struct GmInfo {
	const char* gmType;	// gm类型(文字)
	bool gmHasMultiTrack;	// gm是支持多Track的类型
	bool gmHasMultiChannel; // gm允许多channel(stereo模式的基础)
	int gmTrackCount;	// gm文件共几首曲子

	gme_info_t* gmInfoSet;

	int gmVoiceCount; // 一共几种声音

} GmInfo;

typedef struct C {
	Music_Emu* emu;
	const PList_Node* item;
	const Args* args;

	// gm文件信息
	gme_type_t gmType;
	GmInfo* gmInfo;
	int currentTrack;

	// wav文件相关指针
	SF_INFO* wavInfo;
	SNDFILE* wavFile;
	char* currentOutFilePath;
} C;

// 类C(onverter)本身
extern void C_init(C* __restrict c, const PList_Node* item, const Args* __restrict args); // 初始化类
extern void C_clear(C* __restrict c);							  // 释放类的内容的内存，但是类本身还需要手动free

// 曲目信息与控制
extern void C_getinfo(C* __restrict c);	       // 获取当前item当前子曲目信息，无论将信息打印出来与否，都需要获取
extern void C_showinfo(const C* __restrict c); // 打印当前子曲目信息
// 设置当前播放item的何子曲目
#define C_set_current_track(c, track) (c)->currentTrack = (track)
// 返回当前item的子曲目总数
#define C_track_count(c) ((c)->gmInfo->gmTrackCount)
// 返回当前曲目是否支持stereo模式(多通道模式)
#define C_stereo_available(c) ((c)->gmInfo->gmHasMultiChannel)

//.wav文件读写相关函数
extern void C_new_wavFile(C* __restrict c);		  // 计算当前曲目对应输出.wav文件的路径，并打开文件指针
extern void C_close_wavFile(C* __restrict c);		  // 关闭文件指针，完成写入
extern void C_calc_outWavFilePath(const C* __restrict c); // 计算路径，见上
extern void C_wavFile_add_loopMessage(C* __restrict c);	  // 为.wav文件添加循环信息。TODO：目前尚不支持

// 模拟播放函数，所谓播放实为输出音频数据
extern void C_play_normal(C* __restrict c);	// 对于一般种类item，使用此播放函数
extern void C_play_nsf_normal(C* __restrict c); // 对于支持多通道的item的非stereo模式播放
extern void C_play_nsf_stereo(C* __restrict c); // ……stereo模式播放

extern void C_play_control(C* __restrict c); // 播放控制函数，必须。相当于封装了一部分控制相关代码

extern void convert(const PList_Node* __restrict item, const Args* __restrict args); // 类C的一系列操作的一个……前端、示例？

#endif
