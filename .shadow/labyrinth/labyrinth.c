#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"
char palyerId = 'q';
FILE* getMapFile();
void initMap(Labyrinth *labyrinth);
void printMap(Labyrinth *labyrinth);

int main(int argc, char *argv[]) {

    Labyrinth* labyrinth = (Labyrinth*)malloc(sizeof(Labyrinth));
    if (labyrinth == NULL) {
        perror("内存分配失败");
        exit(1);
    }

    for (size_t i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--map") == 0 || strcmp(argv[i], "-m") == 0) {
            i++;
            if (i >= argc) {
                break;
            }
            if (strcmp(argv[i], "map.txt") == 0) {
                loadMap(labyrinth, "map.txt");
                printMap(labyrinth);
            }
        } else if (strcmp(argv[i], "--playerId") == 0 || strcmp(argv[i], "-p") == 0) {
            i++;
            if (i >= argc) {
                break;
            }
            palyerId = argv[i][0];
            if (isValidPlayer(palyerId)) {
                printf("Player ID: %c\n", palyerId);
            } else {
                printf("用户 id 不合法");
                return 0;
            }

            Position pos = findPlayer(labyrinth, palyerId);
            if (pos.row != -1) {
                printf("当前用户的位置为 {%d, %d}", pos.row, pos.col);
            }
        }
    }
    return 0;
}

void printUsage() {
    printf("Usage:\n");
    printf("  labyrinth --map map.txt --player id\n");
    printf("  labyrinth -m map.txt -p id\n");
    printf("  labyrinth --map map.txt --player id --move direction\n");
    printf("  labyrinth --version\n");
}

bool isValidPlayer(char playerId) {
    
    if (palyerId <= '0' || palyerId >= '9') {
        return false;
    }
    return true;
}

bool loadMap(Labyrinth *labyrinth, const char *filename) {
    initMap(labyrinth);
    return true;
}

Position findPlayer(Labyrinth *labyrinth, char playerId) {
    Position pos = {-1, -1};
    for (size_t i = 0; i < labyrinth->rows; i++) {
        for (size_t j = 0; j < labyrinth->cols; j++) {
            if (labyrinth->map[i][j] == palyerId) {
                pos.row = i, pos.col = j;
            }
        }
        printf("\n");
    }

    return pos;
}

Position findFirstEmptySpace(Labyrinth *labyrinth) {
    // TODO: Implement this function
    Position pos = {-1, -1};
    return pos;
}

bool isEmptySpace(Labyrinth *labyrinth, int row, int col) {
    // TODO: Implement this function
    return false;
}

bool movePlayer(Labyrinth *labyrinth, char playerId, const char *direction) {
    // TODO: Implement this function
    return false;
}

bool saveMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    return false;
}

// Check if all empty spaces are connected using DFS
void dfs(Labyrinth *labyrinth, int row, int col, bool visited[MAX_ROWS][MAX_COLS]) {
    // TODO: Implement this function
}

bool isConnected(Labyrinth *labyrinth) {
    // TODO: Implement this function
    return false;
}


FILE* getMapFile() {
    FILE *map = fopen("./maps/map.txt", "r");
    if (map == NULL) {
        perror("文件打开失败");
        return NULL;
    }

    return map;
}

void initMap(Labyrinth *labyrinth) {
    FILE *map = getMapFile();
    if (map == NULL) {
        // getMapFile 内部已经打印了错误信息，这里直接返回即可
        return;
    }

    // 初始化行列
    labyrinth->rows = 0;
    labyrinth->cols = 0;

    char buffer[MAX_COLS + 2];
    while (fgets(buffer, sizeof(buffer), map) != NULL) {
        // 安全检查：防止超过最大行数
        if (labyrinth->rows >= MAX_ROWS) {
            printf("警告：地图行数超过最大限制 (%d)，停止读取。\n", MAX_ROWS);
            break;
        }

        buffer[strcspn(buffer, "\n")] = '\0';
        // 如果是空行，跳过（可选逻辑）
        if (strlen(buffer) == 0) continue;

        strcpy(labyrinth->map[labyrinth->rows], buffer);

        // 更新列数（假设是矩形地图，取最长的一行或者第一行的长度）
        int currentLineLen = strlen(buffer);
        if (currentLineLen > labyrinth->cols) {
            labyrinth->cols = currentLineLen;
        }

        labyrinth->rows++;
    }
    

    fclose(map);
}

void printMap(Labyrinth *labyrinth) {
    for (size_t i = 0; i < labyrinth->rows; i++) {
        for (size_t j = 0; j < labyrinth->cols; j++) {
            printf("%c", labyrinth->map[i][j]);
        }
        printf("\n");
    }
    
}
