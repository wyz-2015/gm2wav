#ifndef _CONVERTER_H
#define _CONVERTER_H

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

/*
extern void C_init(C* __restrict c, PList_Node* item, Args* __restrict args);
extern void C_getinfo(C* __restrict c, const int trackIndex);
extern void C_showinfo(const C* __restrict c, const int trackIndex);
extern void C_clear(C* __restrict c);
*/

// void convert(const Args* __restrict args);
// extern void core_stereo(const long int sampleRate, const int trackIndex, const char* inFilePath, const char* outFilePath);
extern void convert(const PList_Node* __restrict item, const Args* __restrict args);

#endif
