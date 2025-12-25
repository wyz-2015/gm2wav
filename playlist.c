#include "playlist.h"
#include <libgen.h>

void PList_init(PList* restrict pl)
{
	pl->head = NULL;
	pl->end = NULL;
	pl->__len__ = 0;
}

void PList_append(PList* restrict pl, const char* restrict inFilePath)
{
	PList_Node* newNode = (PList_Node*)malloc(sizeof(PList_Node));
	if (not newNode) {
		error(ENOMEM, ENOMEM, "%s：在%p上内存分配失败。", __func__, newNode);
	}

	// 先填写新结点的内容

	char* inFilePath_abs = realpath(inFilePath, NULL); // 记得手动free掉
	if (not inFilePath_abs) {
		error(EINVAL, EINVAL, "%s：无法求\"%s\"的绝对路径。", __func__, inFilePath);
	}
	const size_t inFilePath_abs_len = strlen(inFilePath_abs) + 1;

	// 绝对路径解析的处理很麻烦，依赖的dirname(s)与basename(s)本身会对传入的s作改变
	char tempBuffer[inFilePath_abs_len];											 // 临时缓冲区，多次利用，自动清除
	char *dir = (char*)malloc(inFilePath_abs_len * sizeof(char)), *fname = (char*)malloc(inFilePath_abs_len * sizeof(char)); // 用于存放结果
	if (not(dir and fname)) {
		error(ENOMEM, ENOMEM, "%s：在%p或%p上内存分配失败。", __func__, dir, fname);
	}

	strcpy(tempBuffer, inFilePath_abs); // 先拷贝一份原始字符串到缓冲区，对缓冲区的副本进行操作
	dirname(tempBuffer);		    // dirname(s)会修改s指向的字符串的内容，擅自添加'\0'。
	strcpy(dir, tempBuffer);	    // 再把处理完的结果拷贝到目标地址

	strcpy(tempBuffer, inFilePath_abs);
	strcpy(fname, basename(tempBuffer)); // basename(s)乖一点~不修改s，只返回基于s(即没有额外分配空间)的结果字符串起点地址。

	newNode->fileDir = dir;
	newNode->fileName = fname;

	free(inFilePath_abs);

	newNode->next = NULL;
	newNode->prev = NULL;

	// 再处理链表问题

	if (pl->__len__ == 0) {
		pl->head = newNode;
	} else {
		pl->end->next = newNode;
	}
	newNode->prev = pl->end;
	pl->end = newNode;
	pl->__len__ += 1;
}

void PList_pop(PList* restrict pl, PList_Node** restrict dest) // deque里弹出最后一项，并将结点地址返回到dest中。如果dest为NULL则直接释放其内存。
{
	PList_Node* lastNode = pl->end;
	if (lastNode) {
		/*
		if (inFilePath) {
			sprintf(inFilePath, "%s/%s", lastNode->fileDir, lastNode->fileName);
		}
		*/

		if (lastNode->prev) {
			lastNode->prev->next = NULL;
		}
		pl->end = lastNode->prev;
		pl->__len__ -= 1;

		if (dest) {
			lastNode->next = NULL;
			lastNode->prev = NULL;
			*dest = lastNode;
		} else {
			if (lastNode->fileDir) {
				free(lastNode->fileDir);
			}
			if (lastNode->fileName) {
				free(lastNode->fileName);
			}
			free(lastNode);
		}
	} else {
		error(EPERM, EPERM, "%s：deque(%p)为空，无法pop。", __func__, pl);
	}
}

void PList_clear(PList* restrict pl)
{
	while (pl->__len__ > 0) {
		PList_pop(pl, NULL);
	}
	pl->head = NULL;
}

void PList_print(const PList* restrict pl)
{
	puts("[");
	PList_Node* currentNode = pl->head;
	while (currentNode) {
		printf("Path(\"%s/%s\"),\n", currentNode->fileDir, currentNode->fileName);
		currentNode = currentNode->next;
	}
	puts("]");
}

PList_Node* PList_get(const PList* restrict pl, uint32_t index)
{
	if (index < 0 or index >= pl->__len__) {
		error(EINVAL, EINVAL, "%s：对list(%p)取值无效，“%u”不是有效的索引值。", __func__, pl, index);
		return NULL;
	}

	PList_Node* currentNode = pl->head;
	for (uint32_t i = 0; i < index; i += 1) {
		currentNode = currentNode->next;
	}

	return currentNode;
}
