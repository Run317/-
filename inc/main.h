#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "usr.h"

// 用户面板
void menu(void);
// 将数据尾插到链表
void linklistAdd(node* local, node* new);
// 查找结点
node* linklistFind(node* local, node* target);
// 加载用户信息
void loadInfo(node* local);
//初始化头节点为广播地址
int broadcastInit(Line* online);

#endif
