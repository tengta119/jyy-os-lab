#include <testkit.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>  // 包含 isdigit 函数
#include <stdbool.h> // 包含 bool 类型 (C99 标准)

bool isAllDigits(const char *str);

int main() {
    DIR *dir = opendir("/proc");

    if (dir == NULL) {
        perror("无法打开mul");
        return 0;
    }

    printf("进程号:\n");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char *fileName = entry->d_name;
        if (isAllDigits(fileName)) {
            printf("%s ", fileName);
        }
    }
    
    closedir(dir);
}

bool isAllDigits(const char *str) {
    if (str == NULL || *str == '\0') {
        return false;
    }

    while (*str != '\0') {
        if (!isdigit(*str)) {
            return false;
        }
        str++;
    }

    return true;
    
}
