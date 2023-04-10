#include "main.h"
#include "usr.h"

//在线链表头节点
Line* onlineHead;

int main(int argc, char* argv[])
{
    //创建头节点，导入用户信息
    node* local = userInit();
    //创建头节点，存储在线用户信息
    onlineHead = onlineInit();
    //加载用户信息。将本地信息导入到程序
    loadInfo(local);

    // 显示菜单
    while (1)
    {
        menu();
        int cmd = 0;
        printf("Please input cmd:");
        scanf("%d", &cmd);
        switch (cmd)
        {
        case 1:
            userLogin(local);
            break;
        case 2:
            userSignup(local);
            break;
        case 9:
            return 0;
        default:
            printf("请输入正确指令\n");

        }
    }
}

// 用户面板
void menu(void)
{

    printf("****************************************************\n");
    printf("*                      WeChat                      *\n");
    printf("*                                                  *\n");
    printf("*                      1.登录                      *\n");
    printf("*                      2.注册                      *\n");
    printf("*                      9.退出                      *\n");
    printf("*                                                  *\n");
    printf("****************************************************\n");
}

// 加载用户信息
void loadInfo(node* local)
{
    // 打开本地文件，将用户信息导入到程序链表
    FILE* fp = fopen("./data/info.txt", "r");
    if (fp == NULL)
    {
        perror("loading info failed");
        return;
    }
    int flag = 0;
    while (!feof(fp))
    {
        node* pos = userInit();
        fscanf(fp, "%s %s\n", pos->name, pos->password);
        node* repeat = local;
        while (repeat->next != NULL)
        {
            flag = 0;
            if (strcmp(pos->name, repeat->name) == 0)
            {
                flag = 1;
                break;
            }
            repeat = repeat->next;
        }
        if (flag == 0)
        {
            linklistAdd(local, pos);
        }
        if (flag == 1)
        {
            continue;
        }
    }
    //================导入链表检测===================
    //    node* pos1 = local->next;
    //    while (pos1 != NULL)
    //    {
    //        printf("%s,%s\n", pos1->name, pos1->password);
    //        pos1 = pos1->next;
    //    }
    //=============================================
    {
        fclose(fp);
    }
}

// 将数据尾插到链表
void linklistAdd(node* local, node* new)
{
    node* pos = local;
    while (pos->next != NULL)
    {
        pos = pos->next;
    }
    pos->next = new;
}

// 查找结点
node* linklistFind(node* local, node* target)
{
    node* pos = local->next;
    if (local->next == NULL)
    {
        return NULL;
    }
    while (pos != NULL)
    {
        if (strcmp(pos->name, target->name) == 0)
        {
            return pos;
        }
        pos = pos->next;
    }
    return NULL;
}
