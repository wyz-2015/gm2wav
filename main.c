#include "args.h"
#include "converter.h"
#include "playlist.h"
#include <argp.h>

PList* playlist = NULL;

static const struct argp_option options[] = {
	{ "out_dir", 'o', "str", OPTION_ARG_OPTIONAL, "输出文件目录" },
	{ "sample_rate", 's', "int32_t", OPTION_ARG_OPTIONAL, "音频采样率(单位Hz，默认44100)" },
	{ "loop_time", 'l', "uint32_t", OPTION_ARG_OPTIONAL, "循环节执行次数(新增功能)，如果曲目本身不循环，则失效(默认2)" },
	{ "loop_message", 'i', 0, OPTION_ARG_OPTIONAL, "是否为wav文件写入循环信息(目前尚不支持，默认不启用)" },
	{ "channels", 'c', "uint8_t", OPTION_ARG_OPTIONAL, "通道数(默认2)" },
	{ "fadeout_ms", 'f', "int", OPTION_ARG_OPTIONAL, "若曲目本身不循环，则失效(单位ms，默认1000)" },
	{ "stereo", 'd', 0, OPTION_ARG_OPTIONAL, "stereo模式，此选项会使channels选项失效(强制改为2)(默认不启用)" },
	{ "mode", 'm', "uint8_t", OPTION_ARG_OPTIONAL, "对包含多曲目的单文件的转换模式。0为全部切出(默认，此模式下-t / --track选项失效)，1为选取其中之一" },
	{ "track", 't', "int", OPTION_ARG_OPTIONAL, "如果指定了n取1模式，则取第几Track(0数起，默认0)" },
	{ "verbose", 'v', 0, OPTION_ARG_OPTIONAL, "显示详细信息" },
	// { 0, '\0', "str", 0, "传入文件路径" }
	{ 0 }
};

static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
	Args* args = state->input;

	switch (key) {
	case 'o': {
		args->outDir = arg;
		break;
	}
	case 'l': {
		if (arg == NULL or sscanf(arg, "%u", &(args->loopTime)) != 1) {
			error(EINVAL, EINVAL, "读取-l / --loop_time的参数“%s”失败", arg);
		}
		break;
	}
	case 's': {
		if (arg == NULL or sscanf(arg, "%ld", &(args->sampleRate)) != 1) {
			error(EINVAL, EINVAL, "读取-s / --sample_rate的参数“%s”失败", arg);
		}
		break;
	}
	case 'i': {
		args->loopWav = true;
		break;
	}
	case 'c': {
		if (arg == NULL or sscanf(arg, "%hhu", &(args->channels)) != 1) {
			error(EINVAL, EINVAL, "读取-c / --channels的参数“%s”失败", arg);
		}
		break;
	}
	case 'f': {
		if (arg == NULL or sscanf(arg, "%d", &(args->fadeOut_ms)) != 1) {
			error(EINVAL, EINVAL, "读取-f / --fadeout_ms的参数“%s”失败", arg);
		}
		break;
	}
	case 'd': {
		args->stereo = true;
		break;
	}
	case 'm': {
		if (arg == NULL or sscanf(arg, "%hhu", &(args->mode)) != 1) {
			error(EINVAL, EINVAL, "读取-s / --sample_rate的参数“%s”失败", arg);
		}
		if (args->mode > 1) {
			error(EINVAL, EINVAL, "传入了不在范围内的参数(%hhu)，范围：{0, 1}", args->mode);
		}
		break;
	}
	case 't': {
		if (arg == NULL or sscanf(arg, "%d", &(args->trackNum)) != 1) {
			error(EINVAL, EINVAL, "读取-t / --track的参数“%s”失败", arg);
		}
		break;
	}
	case 'v': {
		args->verbose = true;
		break;
	}
	case ARGP_KEY_ARG: {
		PList_append(playlist, arg);
		break;
	}
	}
	return 0;
}

static struct argp argp = { options, parse_opt, NULL, NULL };

int main(int argc, char** argv)
{
	playlist = (PList*)malloc(sizeof(PList));
	if (not playlist) {
		error(ENOMEM, ENOMEM, "%s：为playlist(%p)malloc失败。", __func__, playlist);
	}
	PList_init(playlist);

	Args args = { NULL, 44100, 2, false, 2, 10000, false, 0, 0, false };
	argp_parse(&argp, argc, argv, 0, 0, &args);
	if (args.stereo) {
		args.channels = 2;
	}
	if (args.verbose) {
		PList_print(playlist);
	}

	PList_Node* item = playlist->head;
	while (item) {
		convert(item, &args);
		item = item->next;
	}

	PList_clear(playlist);
	if (playlist) {
		free(playlist);
		playlist = NULL;
	}
	return 0;
}
