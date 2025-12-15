#ifndef _COMMON_H
#define _COMMON_H

#include <errno.h>
#include <error.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 默认程序里所有与路径相关的字符串占用空间最大为： */
#define FILENAME_LEN_MAX 1024

/* bool型对应字符串 */
#define boolStr(b) ((b) ? ("True") : ("False"))

#endif
