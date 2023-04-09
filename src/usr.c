#include "usr.h"
#include "main.h"

extern Line* onlineHead;
Line* currentUser = NULL;

// 初始化本地信息
node* userInit(void)
{
    // 申请堆空间，新建本地用户节点
    node* p = malloc(sizeof(node));
    if (p == NULL)
    {
        perror("userInit failed");
        return NULL;
    }
    // 清空节点数据
    bzero(p->name, sizeof(p->name));
    bzero(p->password, sizeof(p->password));
    p->next = NULL;

    return p;
}

//初始化在线信息
Line* onlineInit(void)
{
    // 申请堆空间，新建在线用户节点
    Line* p = malloc(sizeof(Line));
    if (p == NULL)
    {
        perror("onlineInit failed");
        return NULL;
    }
    // 清空节点数据
    bzero(p->name, sizeof(p->name));
    bzero(&p->userIP, sizeof(p->userIP));
    p->next = NULL;

    return p;
}


// 注册用户
void userSignup(node* local)
{
    // 注册，设置用户名及密码
    node* newUsr = userInit();
    printf("请输入用户名\n");
    scanf("%s", newUsr->name);
    printf("请输入密码\n");
    scanf("%s", newUsr->password);
    // 从链表中判断该用户是否已被注册
    if (linklistFind(local, newUsr) != NULL)
    {
        printf("该用户已被注册\n");
        return;
    }
    // 用户信息存储到本地文件
    FILE* fp = fopen("./data/info.txt", "a+");
    if (fp == NULL)
    {
        perror("loading info failed");
        return;
    }
    // 新用户信息追加写入本地文件
    fprintf(fp, "%s %s\n", newUsr->name, newUsr->password);
    //新用户信息插入到链表
    linklistAdd(local, newUsr);

    fclose(fp);
}

// 用户登录
int userLogin(node* local)
{
    //新建用户节点，存储用户信息，与本地信息比较
    node* new = userInit();
    printf("请输入用户名：");
    scanf("%s", new->name);
    printf("请输入密码：");
    scanf("%s", new->password);
    //创建遍历节点，用于比较本地信息
    node* user = linklistFind(local, new);
    if (user == NULL)
    {
        printf("该用户未注册\n");
        return -1;
    }
    else if (strcmp(user->password, new->password) != 0)
    {
        printf("密码错误\n");
        return -2;
    }
    currentUser = onlineInit();
    printf("登陆成功！\n");
    sprintf(currentUser->name, "%s", user->name);
    userPanel(onlineHead, user);
    return 0;
}

// 发送广播
int send_broadcast(char* msg)
{
    usleep(1);
    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1)
    {
        perror("socket");
        return -1;
    }

    // 设置套接字    允许发送广播
    int on = 1;
    if (setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)))
    {
        perror("setsockopt");
        return -2;
    }

    // 初始化对端地址结构体(server)
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;                                       // 地址族
    server_addr.sin_port = htons(65535);                           // 端口
    server_addr.sin_addr.s_addr = inet_addr("255.255.255.255");         // IPV4 地址  (服务器的地址)

    // 接收对方回传的信息
    usleep(1);
    sendto(client_socket, msg, strlen(msg) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    //关闭套接字
    close(client_socket);
    return 0;
}

// 接收别的主机上线的广播，一直接收，服务器
void* rcv_broadcast(void* arg)
{
    // 创建套接字
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1)
    {
        perror("socket");
        return NULL;
    }
    int on = 1;
    // 只要有绑定，设置套接字    允许重用地址和端口号
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        perror("setsockopt");
        return NULL;
    }

    // 初始化地址结构体
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; // 地址族
    server_addr.sin_port = htons(65535);	  // 端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY "0.0.0.0"直接获取本机IP地址

    // 绑定地址
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        perror("bind");
        return NULL;
    }

    // 接收数据
    char r_buf[1024];
    char name[32];
    struct sockaddr_in client_addr;
    int addr_size = sizeof(client_addr);
    while (1)
    {
        bzero(r_buf, 1024);
        bzero(name, sizeof (name));
        // 循环一直接收
        recvfrom(server_socket, r_buf, 1024, 0, (struct sockaddr*)&client_addr, &addr_size);
        //新建节点插入链表
        Line* newUser = onlineInit();
        newUser->userIP = client_addr;
        sscanf(r_buf, "%[^ ]", name);
        sprintf(newUser->name, "%s", name);

        //判断广播信息是否是本机发送,如果是本机发送，将本机地址存到当前用户全局变量
        if (strcmp(name, currentUser->name) == 0)
        {
            currentUser->userIP = client_addr;
        }
        //打印用户上线提醒,屏蔽本机信息
        if (strstr(r_buf, "Online") != NULL)
        {
            //如果接收到的不是本机的上线信息，回发本机在线信息
            if (strstr(r_buf, currentUser->name) != NULL)
            {
                char onlineFlag[64];
                bzero(onlineFlag, sizeof(onlineFlag));
                sprintf(onlineFlag, "%s Online", currentUser->name);
                send_broadcast(onlineFlag);
            }
            //检测在线用户链表中是否有该用户
            if (onlineListAddCheck(name))
            {
                //如果没有该用户，且不是本机发送的消息，打印用户上线信息
                if (strcmp(name, currentUser->name) != 0)
                {
                    printf("[%s][%d]:%s\n", inet_ntoa(newUser->userIP.sin_addr), newUser->userIP.sin_port, r_buf);
                }
                onlineListAdd(newUser);
            }
        }
        //打印下线信息
        if (strstr(r_buf, "Offline") != NULL)
        {
            if (strcmp(name, currentUser->name) != 0)
            {
                printf("%s\n", r_buf);
            }
            onlineListDel(name);
        }
        //===================================检查在线链表========================================================
        //        Line* pos = onlineHead->next;
        //        while (pos != NULL)
        //        {
        //            printf("[%d][%s]:%s\n", pos->userIP.sin_port, inet_ntoa(client_addr.sin_addr), pos->name);
        //            pos = pos->next;
        //        }
        //=====================================================================================================
    }
}

//用户面板
int userPanel(Line* head, node* user)
{
    char msg[1024];
    bzero(msg, sizeof(msg));
    pthread_t rcvBroadcastPid;
    //创建线程，接收广播
    pthread_create(&rcvBroadcastPid, NULL, rcv_broadcast, NULL);
    //发送上线广播
    sprintf(msg, "%s Online", currentUser->name);
    send_broadcast(msg);

    while (1)
    {
        printf("***************USER-PANEL*************\n");
        printf("当前用户:%s\n", currentUser->name);
        printf("1.单独发送消息        2.群发消息\n");
        printf("3.单独发送文件        4.群发文件\n");
        printf("5.下线\n");
        printf("****************END-LINE**************\n");

        //创建线程接收消息
        pthread_t rcvMsgPid;
        pthread_create(&rcvMsgPid, NULL, rcvMsg, NULL);

        int panelCmd = 0;
        scanf("%d", &panelCmd);
        switch (panelCmd)
        {
        case 1:
            sendIndividually(head);
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            bzero(msg, sizeof(msg));
            sprintf(msg, "%s Offline:[%s][%d]", currentUser->name, inet_ntoa(currentUser->userIP.sin_addr), currentUser->userIP.sin_port);
            send_broadcast(msg);
            usleep(10);
            return 0;
        default:
            printf("请输入正确指令~\n");
            return -1;
        }
    }

}

//单独发送消息
void sendIndividually(void* arg)
{
    //显示在线列表
    //打印表头
    printf("用户名\t IP\t 端口\n");
    //遍历链表
    Line* pos = onlineHead->next;
    while (pos != NULL)
    {
        printf("%s\t %s\t %d\n", pos->name, inet_ntoa(pos->userIP.sin_addr), pos->userIP.sin_port);
        pos = pos->next;
    }
    //发送消息
    while (1)
    {
        printf("请输入发送对象用户名\n");
        printf("输入“cancel”取消发送\n");
        char name[32];
        bzero(name, sizeof(name));
        scanf("%s", name);
        if (strcmp(name, "cancel") == 0)
        {
            break;
        }
        if (onlineListAddCheck(name)) //判断在线链表中是否有该用户
        {
            printf("没有该用户\n");
        }
        else
        {
            //遍历链表，从链表中获取发送对象信息
            Line* target = ConfirmRecipient(name);
            int sendSocket = socket(AF_INET, SOCK_DGRAM, 0);
            if (sendSocket == -1)
            {
                perror("sendIndividually socket failed");
            }
            char msg[1024];
            char buf[512];
            printf("消息发送栏：\n");
            while (1)
            {
                bzero(msg, sizeof (msg));
                bzero(buf, sizeof (buf));
                scanf("%s", buf);
                sprintf(msg, "%s :%s", currentUser->name, buf);
                if (strstr(msg, "exit") != NULL)
                {
                    printf("退出成功\n");
                    return;
                }
                sendto(sendSocket, msg, strlen(msg) + 1, 0, (struct sockaddr*)&target->userIP, sizeof (target->userIP));
            }
        }
    }
}

void sendMultiple(void)
{
    char group[512];
    bzero(group, sizeof (group));
    printf("请输入发送对象，多个对象之间用“，”隔开例如”a，b，c“\n");
}

//接收信息
void* rcvMsg(void* arg)
{
    int rcvSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (rcvSocket == -1)
    {
        perror("rcvMsg socket failed");
        return NULL;
    }
    int on = 1;
    // 只要有绑定，设置套接字    允许重用地址和端口号
    if (setsockopt(rcvSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        perror("rcvMsg setsockopt failed");
        return NULL;
    }

    if (bind(rcvSocket, (struct sockaddr*)&currentUser->userIP, sizeof (currentUser->userIP)))
    {
        perror("rcvMsg bind failed");
        return NULL;
    }

    char msg[1024];
    struct sockaddr_in senderAddr;
    int addrSize = sizeof (senderAddr);
    while (1)
    {
        bzero(msg, sizeof (msg));
        recvfrom(rcvSocket, msg, 1024, 0, (struct sockaddr*)&senderAddr, &addrSize);
        printf("%s\n", msg);
    }
}

//在线链表删除节点
void onlineListDel(char* name)
{
    Line* pos = onlineHead;
    Line* del = pos->next;
    while (del != NULL)
    {
        if (strcmp(del->name, name) == 0)
        {
            pos->next = del->next;
            free(del);
            break;
        }
        pos = pos->next;
        del = pos->next;
    }
}

//在线链表尾插
void onlineListAdd(Line* new)
{
    Line* pos = onlineHead;
    while ( pos->next != NULL)
    {
        pos = pos->next;
    }
    pos->next = new;
}

//插入在线链表检查,没有此用户返回真
bool onlineListAddCheck(char* name)
{
    Line* pos = onlineHead;
    while (pos != NULL)
    {
        if (strcmp(name, pos->name) == 0)
        {
            return false;
        }
        pos = pos->next;
    }
    return true;
}

//确认接收对象
Line* ConfirmRecipient(char* name)
{
    Line* target = onlineHead;
    while (target != NULL)
    {
        if (strcmp(name, target->name) == 0)
        {
            return target;
        }
        target = target->next;
    }
    return  NULL;
}