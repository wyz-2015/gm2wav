#include "converter.h"

void print_10_short(short* buffer, uint32_t _len) // 调试用，连续以short型打印_len个缓冲区中的数据
{
	putchar('[');
	for (int i = 0; i < _len; i += 1) {
		// printf("0x%04x, ", buffer[i]);
		printf("%d, ", buffer[i]);
	}
	puts("]");
}

void interger_roll(int* n, const int a, const int b) // 数字滚动，让数在[a, b)间滚动。
						     // 例：i初为1，a = 1, b = 3。则i变化：1, 2, 1, 2, ...
{
	*n += 1;
	if (*n >= b) {
		*n = a;
	}
}

// ms -> 采样帧数
#define ms2frame(t, sampleRate) ((uint64_t)((double)(t) / 1000 * (sampleRate)))

void print_txtFile(const char* txtPath)
{
	FILE* txtFile = fopen(txtPath, "rt");
	if (txtFile) { // 若文件未打开，则默不作声
		char chr;
		while ((chr = fgetc(txtFile)) != EOF) {
			putchar(chr);
		}
		fclose(txtFile);
	}
	putchar('\n');
}

// 作为上方函数的别名
#define print_unicodeArt(txtPath) print_txtFile((txtPath))

/*
// 设置当前播放item的何子曲目
#define C_set_current_track(c, track) (c)->currentTrack = (track)
// 返回当前item的子曲目总数
#define C_track_count(c) ((c)->gmInfo->gmTrackCount)
// 返回当前曲目是否支持stereo模式(多通道模式)
#define C_stereo_available(c) ((c)->gmInfo->gmHasMultiChannel)
*/

const char* errMessage = NULL;

void C_init(C* restrict c, const PList_Node* item, const Args* restrict args) // 初始化类
{
	c->item = item;
	c->args = args;

	// c->emu = (Music_Emu*)malloc(sizeof(Music_Emu));

	char filePath[FILENAME_LEN_MAX];
	sprintf(filePath, "%s/%s", c->item->fileDir, c->item->fileName);

	c->gmType = gme_identify_extension(filePath);
	c->emu = gme_new_emu_multi_channel(c->gmType, c->args->sampleRate);
	gme_enable_accuracy(c->emu, true);

	errMessage = gme_load_file(c->emu, filePath);
	if (errMessage) {
		error(EPERM, EPERM, "%s: %s", __func__, errMessage);
	}
	// 先载入文件，再读取信息

	c->gmInfo = (GmInfo*)malloc(sizeof(GmInfo));
	if (not c->gmInfo) {
		error(ENOMEM, ENOMEM, "%s：在c->gmInfo(%p)上malloc失败。", __func__, c->gmInfo);
	}
	C_set_current_track(c, 0);

	c->gmInfo->gmInfoSet = (gme_info_t*)malloc(sizeof(gme_info_t));
	if (not c->gmInfo->gmInfoSet) {
		error(ENOMEM, ENOMEM, "%s：在c->gmInfo->gmInfoSet(%p)上malloc失败。", __func__, c->gmInfo->gmInfoSet);
	}

	// 初始化.wav文件的元数据，但不一定只用一个指针，所以不在这里新建指针
	c->wavInfo = (SF_INFO*)malloc(sizeof(SF_INFO));
	if (not c->wavInfo) {
		error(ENOMEM, ENOMEM, "%s：在c->wavInfo(%p)上malloc失败。", __func__, c->wavInfo);
	}
	c->wavInfo->channels = c->args->channels;
	c->wavInfo->samplerate = c->args->sampleRate;
	c->wavInfo->format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	c->wavFile = NULL;

	// 初始化存放输出目录的char数组
	c->currentOutFilePath = (char*)malloc(sizeof(char) * FILENAME_LEN_MAX);
	if (not c->currentOutFilePath) {
		error(ENOMEM, ENOMEM, "%s：为%pmalloc失败。", __func__, c->currentOutFilePath);
	}
	memset(c->currentOutFilePath, '\0', FILENAME_LEN_MAX * sizeof(char));
}

void C_calc_outWavFilePath(const C* restrict c) // 计算路径，见上
{
	char fileName[FILENAME_LEN_MAX / 4], *dotPos = strrchr(c->item->fileName, '.');
	memset(fileName, '\0', FILENAME_LEN_MAX / 4);

	strncpy(fileName, c->item->fileName, dotPos - (c->item->fileName));
	if (c->gmInfo->gmHasMultiTrack) { // 对于无法包含多首曲目的gm文件，就无需在取名时带“-00n”的后缀了
		sprintf(fileName + strlen(fileName), "-%03u", c->currentTrack);
	}
	strcat(fileName, ".wav");

	if (c->args->outDir) {
		sprintf(c->currentOutFilePath, "%s/%s", c->args->outDir, fileName);
	} else {
		sprintf(c->currentOutFilePath, "%s/%s", c->item->fileDir, fileName);
	}
}

void C_new_wavFile(C* restrict c) // 计算当前曲目对应输出.wav文件的路径，并打开文件指针
{
	C_calc_outWavFilePath(c);
	c->wavFile = sf_open(c->currentOutFilePath, SFM_WRITE, c->wavInfo);
	if (not c->wavFile) {
		error(ENOENT, ENOENT, "%s：无法创建新文件%s之指针。", __func__, c->currentOutFilePath);
	} else {
		if (c->args->verbose) {
			printf("%s：新文件%s之指针已获得(%p)。\n", __func__, c->currentOutFilePath, c->wavFile);
		}
	}
}

void C_close_wavFile(C* restrict c) // 关闭文件指针，完成写入
{
	sf_close(c->wavFile);
	if (c->args->verbose) {
		printf("%s：%s(%p)文件指针已关闭。\n", __func__, c->currentOutFilePath, c->wavFile);
	}
	c->wavFile = NULL;
}

void C_getinfo(C* restrict c) // 获取当前item当前子曲目信息，无论将信息打印出来与否，都需要获取
{
	c->gmInfo->gmHasMultiChannel = gme_multi_channel(c->emu);
	c->gmInfo->gmHasMultiTrack = gme_type_multitrack(c->gmType);
	gme_track_info(c->emu, &(c->gmInfo->gmInfoSet), c->currentTrack);
	c->gmInfo->gmTrackCount = gme_track_count(c->emu);
	c->gmInfo->gmVoiceCount = gme_voice_count(c->emu);
	c->gmInfo->gmType = gme_type_system(c->gmType);
}

void C_showinfo(const C* restrict c) // 打印当前子曲目信息
{
	printf("[ %s/%s : %03d ]\n", c->item->fileDir, c->item->fileName, c->currentTrack);
	printf("文件类型：\t%s\n", c->gmInfo->gmType);
	printf("文件含有多曲目：\t%s\n", boolStr(c->gmInfo->gmHasMultiTrack));
	printf("含有曲目数：\t%d\n", c->gmInfo->gmTrackCount);
	printf("文件支持多通道：\t%s\n", boolStr(c->gmInfo->gmHasMultiChannel));

	putchar('\n');

	printf("曲目总长：\t%d ms\n", c->gmInfo->gmInfoSet->length);
	printf("循环节起始位置：\t%d ms处\n", c->gmInfo->gmInfoSet->intro_length);
	printf("循环节长：\t%d ms\n", c->gmInfo->gmInfoSet->loop_length);
	printf("播放时长：\t%d ms\n", c->gmInfo->gmInfoSet->play_length);
	/*
	// #warning "以下2行，第1行为libgme < 0.6.4时的替代语句，第2行为libgme >= 0.6.4(本段编写时最新版本为0.6.4)时的可用语句。可根据需求，通过注释与否，以使用不同的语句。默认使用“替代语句”。"
	#ifdef LIBGME063
		printf("输出音频时拟渐出时长：\t%d ms (libgme < 0.6.4)\n", c->args->fadeOut_ms);
	#endif
	*/

	printf("渐出时长：\t%d ms (libgme >= 0.6.4)\n", c->gmInfo->gmInfoSet->fade_length); // Ubuntu 24.04只提供到0.6.3版，其时gme_info_t结构内尚未添加此项。
	printf("限定最长时长：\t%d ms\n", c->args->lenMax_ms);

	putchar('\n');
	printf("系统：\t%s\n", c->gmInfo->gmInfoSet->system);
	printf("游戏名：\t%s\n", c->gmInfo->gmInfoSet->game);
	printf("曲名：\t%s\n", c->gmInfo->gmInfoSet->song);
	printf("作曲：\t%s\n", c->gmInfo->gmInfoSet->author);
	printf("版权所属：\t%s\n", c->gmInfo->gmInfoSet->copyright);
	printf("评论：\t%s\n", c->gmInfo->gmInfoSet->comment);
	printf("提取者：\t%s\n", c->gmInfo->gmInfoSet->dumper);

	putchar('\n');

	printf("发声元件数：\t%d\n", c->gmInfo->gmVoiceCount);
	for (int i = 0; i < c->gmInfo->gmVoiceCount; i += 1) {
		printf("[%d]\t%s\n", i, gme_voice_name(c->emu, i));
	}

	putchar('\n');
}

void C_clear(C* restrict c) // 释放类的内容的内存，但是类本身还需要手动free
{
	if (c->emu) {
		gme_delete(c->emu);
		c->emu = NULL;
	}
	if (c->gmInfo) {
		if (c->gmInfo->gmInfoSet) {
			// free(c->gmInfo->gmInfoSet);
			gme_free_info(c->gmInfo->gmInfoSet);
			c->gmInfo->gmInfoSet = NULL;
		}
		free(c->gmInfo);
		c->gmInfo = NULL;
	}
	if (c->wavInfo) {
		free(c->wavInfo);
		c->wavInfo = NULL;
	}
	if (c->wavFile) {
		c->wavFile = NULL;
	}
	if (c->currentOutFilePath) {
		free(c->currentOutFilePath);
		c->currentOutFilePath = NULL;
	}
}

/* ***************** 核心部分，与播放相关的函数 ****************** */

void C_play_control(C* restrict c) // Track载入后才能使用
				   // 播放控制函数，必须。相当于封装了一部分控制相关代码
{
	if (c->gmInfo->gmInfoSet->loop_length > 0) { // 含有循环节，则 播放时长 = 前导 + 循环节长 * 循环次数
		c->gmInfo->gmInfoSet->play_length = c->gmInfo->gmInfoSet->intro_length + c->args->loopTime * c->gmInfo->gmInfoSet->loop_length;
		// gme_set_fade(c->emu, c->gmInfo->gmInfoSet->play_length);
		//  c->gmInfo->gmInfoSet->fade_length=c->args->fadeOut_ms;
		gme_set_fade_msecs(c->emu, c->gmInfo->gmInfoSet->play_length, c->args->fadeOut_ms);
	} else { // 无循环节，则 播放时长 = 曲目本身长，并禁止淡出
		c->gmInfo->gmInfoSet->play_length = c->gmInfo->gmInfoSet->length;
		gme_set_fade(c->emu, -1);

		if (c->args->loopTime) {
			printf("%s：由于曲目[%s/%s : %03d]并无循环节，故对循环次数(--loop_time=%u)与淡出时长(--fadeout_ms=%d)的设定将失效。\n",
			    __func__,
			    c->item->fileDir,
			    c->item->fileName,
			    c->currentTrack,
			    c->args->loopTime,
			    c->args->fadeOut_ms);
		}
	}
}

void C_play_nsf_stereo(C* restrict c) // ……stereo模式播放
{
	const int channels = 2; // gme.h中载：程序输出的音频为stereo的。stereo必为2 channels，故写死。

	gme_set_stereo_depth(c->emu, 1.0);

	errMessage = gme_start_track(c->emu, c->currentTrack);
	if (errMessage) {
		error(EPERM, EPERM, "%s: %s", __func__, errMessage);
	}

	C_play_control(c); // 播放控制

	const int voices = 8;

	const size_t inBufferSize = channels * voices, outBufferSize = channels;
	short inBuffer[inBufferSize], outBuffer[outBufferSize];

	while ((not gme_track_ended(c->emu)) and gme_tell(c->emu) < c->args->lenMax_ms) {
		memset(outBuffer, 0, outBufferSize * sizeof(short));
		memset(inBuffer, 0, inBufferSize * sizeof(short));

		/* 在stereo模式下：
		 *
		 * gme生成的1采样帧的PCM数据为：
		 * short inBuffer[8 * 2] = {v1a, v1b, v2a, v2b, ... v8a, v8b};
		 * 8个voices(发声器件)，1个voice有2个channels
		 *
		 * 而将要写入.wav文件的1采样帧16bit PCM数据，其格式是：
		 * short outBuffer[2] = {l, r};
		 * l、r即左、右声道的数据
		 * {	l = (v1a + v1b) / 2 + (v3a + v3b) / 2 + ... + (v7a + v7b) / 2
		 * 	r = (v2a + v2b) / 2 + (v4a + v4b) / 2 + ... + (v8a + v8b) / 2	}
		 * 即将8个voice的声音分散到左右声道内
		 * 而，vna = vnb，目前没见过不等的情况，故取
		 * {	l = sum(vna)
		 * 	r = sum(vnb)
		 * 		(n = 1, 2, ..., 8)	}
		 * 是没有“立体声”意义的
		 */

		errMessage = gme_play(c->emu, inBufferSize, inBuffer);
		if (errMessage) {
			error(EPERM, EPERM, "%s: %s", __func__, errMessage);
		}

		int channelToWrite = 0, v = 0;

		while (v < voices * channels) {
			for (int ch = 0; ch < channels; ch += 1) {
				outBuffer[channelToWrite] += inBuffer[v] * c->args->volumeB / c->args->volumeA;
				v += 1;
			}

			interger_roll(&channelToWrite, 0, channels);
		}
		// sf_write_short(sndfile, out, outBufferSize);
		sf_writef_short(c->wavFile, outBuffer, 1);

		// print_10_short(inBuffer, 16);
		// print_10_short(outBuffer, 2);
	}
}

void C_play_nsf_normal(C* restrict c) // 对于支持多通道的item的非stereo模式播放
{

	errMessage = gme_start_track(c->emu, c->currentTrack);
	if (errMessage) {
		error(EPERM, EPERM, "%s: %s", __func__, errMessage);
	}

	C_play_control(c);

	const uint32_t gmeChannelsPerVoice = 2,
		       voices = 8;
	const size_t inBufferSize = gmeChannelsPerVoice * voices,
		     outBufferSize = c->args->channels;
	short inBuffer[inBufferSize],
	    outBuffer[outBufferSize];

	while ((not gme_track_ended(c->emu)) and gme_tell(c->emu) < c->args->lenMax_ms) {
		memset(inBuffer, 0, inBufferSize * sizeof(short));
		memset(outBuffer, 0, outBufferSize * sizeof(short));

		errMessage = gme_play(c->emu, inBufferSize, inBuffer);
		if (errMessage) {
			error(EPERM, EPERM, "%s: %s", __func__, errMessage);
		}

		uint32_t v = 0;
		int channelToWrite = 0;
		while (v < voices * gmeChannelsPerVoice) {
			outBuffer[channelToWrite] += inBuffer[v] * c->args->volumeB / c->args->volumeA;

			v += 1;
			interger_roll(&channelToWrite, 0, c->args->channels);
		}

		sf_writef_short(c->wavFile, outBuffer, 1);
	}
}

void C_play_normal(C* restrict c) // 对于一般种类item，使用此播放函数
{
	errMessage = gme_start_track(c->emu, c->currentTrack);
	if (errMessage) {
		error(EPERM, EPERM, "%s: %s", __func__, errMessage);
	}

	C_play_control(c);

	const uint32_t gmeChannelsPerVoice = 1, voices = 2;
	const size_t inBufferSize = gmeChannelsPerVoice * voices,
		     outBufferSize = c->args->channels;
	short inBuffer[inBufferSize], outBuffer[outBufferSize];

	while ((not gme_track_ended(c->emu)) and gme_tell(c->emu) < c->args->lenMax_ms) {
		memset(inBuffer, 0, inBufferSize * sizeof(short));
		memset(outBuffer, 0, outBufferSize * sizeof(short));

		/* 对于无多通道的文件，每帧数据，是1 voice * 2 channel，还是2 voice * 1 channel？
		 * 一帧，就只有short inBuffer[2];
		 * 于是，也根本无法构造立体声了。
		 */

		errMessage = gme_play(c->emu, inBufferSize, inBuffer);
		if (errMessage) {
			error(EPERM, EPERM, "%s: %s", __func__, errMessage);
		}

		int v = 0, channelToWrite = 0;
		while (v < gmeChannelsPerVoice * voices) {
			outBuffer[channelToWrite] += inBuffer[v] * c->args->volumeB / c->args->volumeA;

			v += 1;
			interger_roll(&channelToWrite, 0, c->args->channels);
		}

		sf_writef_short(c->wavFile, outBuffer, 1);
	}
}

/* ************************************ */

typedef struct LoopInfo {
	sf_count_t startFrame; // 指采样帧
	sf_count_t endFrame;
} LoopInfo;

void C_wavFile_add_loopMessage(C* restrict c) // 为.wav文件添加循环信息。TODO：目前尚不支持
{
	sf_count_t introFrame = ms2frame(c->gmInfo->gmInfoSet->intro_length, c->args->sampleRate),
		   loopEndFrame = introFrame + ms2frame(c->gmInfo->gmInfoSet->loop_length, c->args->sampleRate);
	LoopInfo info = { introFrame, loopEndFrame };

	// sf_command(c->wavFile, SFC_SET_LOOP_INFO, &info,sizeof(info));
	print_unicodeArt("./cannot_haqi.txt");
	puts("\nslm的，被AI糊弄了。AI告诉我libsndfile有给wav“添加”循环信息的功能，到了真正编写代码的时候发现这库实际上没有这功能，只有“获取”循环信息的功能！\n——但这样的功能对于VGM来说还是很实用的，该有此功能。目前请考虑使用《LoopAuditioneer》《GoldWave》《Audacity(当前Ubuntu版本下十分甚至九分地不推荐)》等工具自行添加循环信息，作为临时方案。");

	printf("循环节信息(/帧)：\n起始帧：\t%lu\n中止帧：\t%lu\n", info.startFrame, info.endFrame);
}

/* ************************************ */

void convert(const PList_Node* restrict item, const Args* restrict args) // 类C的一系列操作的一个……前端、示例？
{
	C c;
	C_init(&c, item, args);

	C_getinfo(&c);

	int start, end;
	switch (c.args->mode) {
	case 0: {
		start = 0;
		end = C_track_count(&c);
		break;
	}
	case 1: {
		start = c.args->trackNum;
		end = start + 1;
		break;
	}
	default: {
		error(EINVAL, EINVAL, "%s：无效的模式代码(%hhu)", __func__, c.args->mode);
		break;
	}
	}

	// printf("m = %hhu\n", c.args->mode);
	// printf("[%d, %d)\n", start, end);
	for (int i = start; i < end; i += 1) {
		C_set_current_track(&c, i);
		C_getinfo(&c);
		if (c.args->verbose) {
			C_showinfo(&c);
		}

		C_new_wavFile(&c);

		if (C_stereo_available(&c)) {
			if (c.args->stereo) {
				C_play_nsf_stereo(&c);
			} else {
				C_play_nsf_normal(&c);
			}
		} else {
			if (c.args->stereo) {
				printf("%s：由于曲目[%s/%s : %03d]本身不支持多通道模式，故“--stereo”选项失效。\n", __func__, c.item->fileDir, c.item->fileName, c.currentTrack);
			}
			C_play_normal(&c);
		}

		// TODO: 循环节信息写入wav
		if (c.args->loopWav) {
			C_wavFile_add_loopMessage(&c);
		}

		C_close_wavFile(&c);
	}

	C_clear(&c);
}
