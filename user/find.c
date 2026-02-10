// 引入xv6内核和用户态程序必需的头文件
#include "kernel/types.h"   // 定义基本数据类型（如int、char）
#include "kernel/stat.h"    // 定义文件状态结构体struct stat（文件类型、大小等）
#include "user/user.h"      // 提供用户态系统调用（open/read/stat/printf等）
#include "kernel/fs.h"      // 定义目录项结构体struct dirent、目录相关常量（如DIRSIZ）

// 递归查找函数：遍历指定路径，查找与target匹配的文件
// path：当前要遍历的目录/文件路径
// target：要查找的目标文件名
void find(char* path, char* target) {
    // buf：存储拼接后的完整路径（最大512字节，避免越界）
    // p：指向buf末尾，用于拼接新的路径
    char buf[512], *p;
    int fd;                 // 文件描述符（打开文件/目录后返回的标识）
    struct dirent de;       // 目录项结构体：存储文件名、inode号等目录项信息
    struct stat st;         // 文件状态结构体：存储文件类型、大小、inode属性等

    // 1. 打开目标路径（0表示只读模式）
    if((fd = open(path, 0)) < 0) {
        // 打开失败：输出错误信息到标准错误（fd=2）
        fprintf(2, "find: 无法打开路径 %s\n", path);
        return;
    }

    // 2. 获取当前路径的文件状态信息（填充struct stat）
    if(fstat(fd, &st) < 0) {//fstat return 0 represent success or 1 represent fail
        fprintf(2, "find: 无法获取文件状态 %s\n", path);
        close(fd);  // 打开失败需关闭文件描述符，避免资源泄漏
        return;
    }

    // 3. 根据文件类型分支处理
    switch (st.type) {
        // 3.1 如果是普通文件：检查文件名是否匹配目标
        case T_FILE:
            // 截取路径末尾的文件名（path + 总长度 - 目标文件名长度），与target对比
            // 例如path是"/home/test.txt"，target是"test.txt"，则path+strlen(path)-8 指向"test.txt"
            if(strcmp(path + strlen(path) - strlen(target), target) == 0) {
                printf("%s\n", path);  // 匹配成功，输出完整路径
            }
            break;

        // 3.2 如果是目录：递归遍历该目录下的所有子项
        case T_DIR:
            // 检查路径长度是否超出缓冲区（防止拼接后越界）
            //the last +1 is \0,beacuse in c , The end of string must have \0
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
                fprintf(2, "find: 路径 %s 过长\n", path);
                close(fd);
                return;
            }

            // 复制当前路径到buf，为拼接子目录/文件名做准备
            strcpy(buf, path);
            // p指向buf末尾（路径最后一个字符的下一位）
            p = buf + strlen(buf);
            // 在路径末尾添加目录分隔符 '/'（如原路径是"/home"，变为"/home/"）
            *p++ = '/';

            // 循环读取目录中的每一个目录项（每次读一个struct dirent）
            while(read(fd, &de, sizeof(de)) == sizeof(de)) {
                // 跳过无效inode（表示该目录项未使用）
                if(de.inum == 0)
                    continue;

                // 跳过 "."（当前目录）和 ".."（上级目录），避免无限递归
                if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;

                // 将当前目录项的文件名复制到buf中，拼接成完整路径
                // 例如buf当前是"/home/"，de.name是"test"，则变为"/home/test"
                memmove(p, de.name, DIRSIZ);
                // 手动添加字符串结束符（DIRSIZ是固定长度，文件名可能不足，需补'\0'）
                p[DIRSIZ] = 0;

                // 获取拼接后路径的文件状态，用于递归判断（是文件还是目录）
                if(stat(buf, &st) < 0) {
                    fprintf(2, "find: 无法获取文件状态 %s\n", buf);
                    continue;
                }

                // 递归调用find：遍历当前子目录/文件
                find(buf, target);
            }
            break;
    }

    // 关闭文件描述符，释放资源
    close(fd);
}

// 程序入口函数
int main(int argc, char* argv[]) {
    // 检查参数数量：必须传入 路径 和 目标文件名（如 find / tmp.txt）
    if(argc != 3) {
        fprintf(2, "用法: find <查找路径> <目标文件名>\n");
        // 退出程序并返回错误码1
        exit(1);
    }

    // 调用递归查找函数，传入用户指定的路径和目标文件名
    find(argv[1], argv[2]);

    // 正常退出程序，返回0
    exit(0);
}