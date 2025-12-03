#include <testkit.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

int main() {
    DIR *dir = opendir("/proc");

    if (dir == NULL) {
        perror("无法打开mul");
        return 0;
    }

    printf("目录内容:\n");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char *fileName = entry->d_name;
        printf("\n %s", fileName);
    }
    
    closedir(dir);
}
