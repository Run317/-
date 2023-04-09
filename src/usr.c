#include "usr.h"
#include "main.h"

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
int userLogin(node* local, Line* online)
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

    pthread_t rcv_pid, snd_pid;

    //创建线程用于发送数据
    pthread_create(&snd_pid, NULL, send_broadcast, (void*)user);

    //创建线程用于接受数据
    pthread_create(&rcv_pid, NULL, rcv_broadcast, (void*)online);
    sleep(1);
    printf("登陆成功！\n");

    userPanel(online, user);
}

// 发送广播
void* send_broadcast(void* user)
{
    usleep(1);
    node* sender = (node*)user;
    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1)
    {
        perror("socket");
        return NULL;
    }

    // 设置套接字    允许发送广播
    int on = 1;
    if (setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)))
    {
        perror("setsockopt");
        return NULL;
    }

    // 初始化对端地址结构体(server)
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;                                       // 地址族
    server_addr.sin_port = 65535;                                           // 端口
    server_addr.sin_addr.s_addr = inet_addr("255.255.255.255");         // IPV4 地址  (服务器的地址)

    // 发送一条上线的信息
    char w_buf[1024];
    sprintf(w_buf, "%s Online!\n", sender->name);

    struct sockaddr_in client_addr;
    int addr_size = sizeof(client_addr);
    // 接收对方回传的信息
    usleep(1);
    sendto(client_socket, w_buf, strlen(w_buf) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

}

// 接收别的主机上线的广播，一直接收，服务器
void* rcv_broadcast(void* head)
{
    //在线列表头结点
    Line* onlineHead = (Line* )head;
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
    server_addr.sin_port = 65535;	  // 端口
    // server_addr.sin_addr.s_addr = inet_addr("192.168.1.204"); // IPV4 地址  (服务器的地址)
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
        // 循环一直接收
        int ret = recvfrom(server_socket, r_buf, 1024, 0, (struct sockaddr*)&client_addr, &addr_size);
        // 链表没有才插入，插入链表

        //新建节点插入链表
        Line* newUser = onlineInit();
        newUser->userIP = client_addr;
        sscanf(r_buf, "%[^ ]", newUser->name);
        //打印用户上线提醒,屏蔽本机信息
        if (newUser->userIP.sin_port != client_addr.sin_port)
        {
            printf("[%s][%d]:%s\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, r_buf);
        }
        //修改指针指向
        Line* tail = onlineHead;
        while (tail->next != NULL)
        {
            tail = tail->next;
        }
        tail->next = newUser;
        //遍历链表
        Line* pos = onlineHead->next;
        while (pos != NULL)
        {
            printf("[%d][%s]:%s\n", pos->userIP.sin_port, inet_ntoa(client_addr.sin_addr), pos->name);
            pos = pos->next;
        }
    }
}

//用户面板
int userPanel(Line* head, node* user)
{
    while (1)
    {
        printf("***************USER-PANEL*************\n");
        printf("当前用户:%s\n", user->name);
        printf("1.单独发送消息        2.群发消息\n");
        printf("3.单独发送文件        4.群发文件\n");
        printf("5.下线\n");
        printf("****************END-LINE**************\n");
        //查找该用户对应在线链表节点
        Line* pos = head;
        while (pos != NULL)
        {
            if (strcmp(user->name, pos->name) == 0)
            {
                break;
            }
            pos = pos->next;
        }
        //创建线程接收消息
        //    pthread_t rcv_pid;
        //    pthread_create(&rcv_pid, NULL, rcvMsg, (void*)pos);

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
            return 0;
        default:
            printf("请输入正确指令~\n");
        }
    }
}

//单独发送
void sendIndividually(Line* head)
{
    //显示在线列表
    //打印表头
    printf("用户名\t IP\t 端口\n");
    //遍历链表
    Line* pos = head->next;
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
        int findFlag = 0;
        Line* find = head;
        while (find != NULL)
        {
            if (strcmp(find->name, name) == 0)
            {
                findFlag = 1;
                break;
            }
            find = find->next;
        }
        if (findFlag == 0)
        {
            printf("没有该用户\n");
        }
        if (findFlag == 1)
        {
            int sendSocket = socket(AF_INET, SOCK_DGRAM, 0);
            if (sendSocket == -1)
            {
                perror("sendIndividually socket failed");
            }
            char msg[1024];
            while (1)
            {
                bzero(msg, sizeof (msg));
                scanf("%[^n]", msg);
                while (getchar() != '\n');
                if (strcmp(msg, "exit") == 0)
                {
                    break;
                }
                sendto(sendSocket, msg, strlen(msg) + 1, 0, (struct sockaddr*)&find->userIP, sizeof (find->userIP));
            }
        }
    }
}

//接收信息
void* rcvMsg(void* user)
{
    Line* rcv = (Line*)user;
    int rcvSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (rcvSocket == -1)
    {
        perror("rcvMsg socket failed");
        return NULL;
    }

    if (bind(rcvSocket, (struct sockaddr*)&rcv->userIP, sizeof (rcv->userIP)))
    {
        perror("rcvMsg bind failed");
        return NULL;
    }
    char msg[1024];
    int addrSize = sizeof (rcv->userIP);
    while (1)
    {
        bzero(msg, sizeof (msg));
        recvfrom(rcvSocket, msg, 1024, 0, (struct sockaddr*)&rcv->userIP, &addrSize);
        printf("%s:%s\n", rcv->name, msg);
    }
}