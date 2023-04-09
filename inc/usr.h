#ifndef USR_H
#define USR_H

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

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
int userLogin(node* local, Line* online);
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
void* send_broadcast(void* user);
//接收信息
void* rcvMsg(void* user);

//单独发送
void sendIndividually(Line* head);

#endif
