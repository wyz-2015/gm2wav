#ifndef _PLAYLIST_H
#define _PLAYLIST_H

/* playlist.h
 * 用于处理、记载传入的将要处理的文件列表的类PList
 * 事一种一种一种一种一种一种deque
 */

#include "common.h"

typedef struct PList_Node {
	struct PList_Node* prev;
	struct PList_Node* next;
	char* fileName;
	char* fileDir; // filePath = ("%s/%s" % (fileDir, fileName))
} PList_Node;

typedef struct PList {
	uint32_t __len__;
	// char* dir;
	PList_Node* head;
	PList_Node* end;
} PList;

void PList_init(PList* __restrict pl); // PList类初始化

void PList_append(PList* __restrict pl, const char* __restrict inFilePath); // 在最后面添加元素，传入值为表示路径的字符串

void PList_pop(PList* __restrict pl, PList_Node** __restrict dest); // deque里弹出最后一项，并将结点地址返回到dest中。如果dest为NULL则直接释放其内存。

void PList_clear(PList* __restrict pl); // 清空。多为用完释放内存时使用

void PList_print(const PList* __restrict pl); // 向stdout打印PList的内容

PList_Node* PList_get(const PList* __restrict pl, uint32_t index); // 取得PList中第index项(从0数起)

#endif
