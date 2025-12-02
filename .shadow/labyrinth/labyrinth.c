#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"
char palyerId = 'q';
int main(int argc, char *argv[]) {
    for (size_t i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--map") == 0 || strcmp(argv[i], "-m") == 0) {
            i++;
            if (i >= argc) {
                break;
            }
            if (strcmp(argv[i], "map.txt") == 0) {
                printf("开始读取地图\n");
                printMap();
            }
        } else if (strcmp(argv[i], "--playerId") == 0 || strcmp(argv[i], "-p") == 0) {
            i++;
            if (i >= argc) {
                break;
            }
            palyerId = argv[i][0];
            printf(palyerId);
        }
    }
    return 0;
}

void printMap() {
    FILE *map = fopen("./maps/map.txt", "r");
    if (map == NULL) {
        perror("文件打开失败");
        return;
    }

    int ch;

    while ((ch = fgetc(map)) != EOF) {
        putchar(ch);
    }

    fclose(map);
    
}

void printUsage() {
    printf("Usage:\n");
    printf("  labyrinth --map map.txt --player id\n");
    printf("  labyrinth -m map.txt -p id\n");
    printf("  labyrinth --map map.txt --player id --move direction\n");
    printf("  labyrinth --version\n");
}

bool isValidPlayer(char playerId) {
    // TODO: Implement this function
    return false;
}

bool loadMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    return false;
}

Position findPlayer(Labyrinth *labyrinth, char playerId) {
    // TODO: Implement this function
    Position pos = {-1, -1};
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
