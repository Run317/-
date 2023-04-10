#ifndef USR_H
#define USR_H

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <stdbool.h>

// 用户信息结构体
typedef struct usrInfo
{
    char name[32];
    char password[32];
    struct usrInfo* next;
} node;

typedef struct onlineInfo
{
    char name[32];
    struct sockaddr_in userIP;
    struct onlineInfo* next;
} Line;


// 用户登录
int userLogin(node* local);
// 用户注册
void userSignup(node* local);
// 初始化节点
node* userInit(void);
//用户面板
int userPanel(Line* head, node* user);
//初始化在线信息
Line* onlineInit(void);

// 接收别的主机上线的广播，一直接收，服务器
void* rcv_broadcast(void* head);
// 发送广播
int send_broadcast(char* msg);
//接收信息
void* rcvMsg(void* user);
//删除节点
void onlineListDel(char* name);

//插入在线链表检查
bool onlineListAddCheck(char* name);
//在线链表尾插
void onlineListAdd(Line* new);
//单独发送消息
int sendIndividually(void);
//确认接收对象
Line* ConfirmRecipient(char* name);

#endif
