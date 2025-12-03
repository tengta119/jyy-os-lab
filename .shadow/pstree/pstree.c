#include <testkit.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>  // 包含 isdigit 函数
#include <stdbool.h> // 包含 bool 类型 (C99 标准)
#include <uthash.h>
#define MAX_NAME_LEN 256

typedef struct ProcessNode {

    int pid;
    int ppid;
    char name[MAX_NAME_LEN];
    struct ProcessNode **children; 
    int child_count;  
    int capacity;  

    UT_hash_handle hh;

} ProcessNode;

ProcessNode* map = NULL;

bool isAllDigits(const char *str);

// 创建一个新的进程节点
ProcessNode* create_node(int pid, int ppid, const char* name);
// 向父节点添加一个子节点
void add_child(ProcessNode* parent, ProcessNode* child);
// 释放节点及其所有子节点（递归释放）
void free_tree(ProcessNode* node);

void add_node_map(ProcessNode *node);
ProcessNode* find_node(int pid);
void delete_node(int pid);

int main() {
    DIR *dir = opendir("/proc");

    if (dir == NULL) {
        perror("无法打开 dir");
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
    printf("\n");
    closedir(dir);

    // 创建一个节点
    ProcessNode *p1 = (ProcessNode*)malloc(sizeof(ProcessNode));
    p1->pid = 100;
    p1->ppid = 1;
    
    // 添加到哈希表
    add_node_map(p1);
    
    // 查找
    ProcessNode *found = find_node(100);
    if (found) {
        printf("Found PID: %d\n", found->pid);
    } else {
        printf("Not Found\n");
    }
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

ProcessNode* create_node(int pid, int ppid, const char* name) {
    ProcessNode* node = (ProcessNode*)malloc(sizeof(ProcessNode));
    if (!node) return NULL;

    node->pid = pid;
    node->ppid = ppid;
    
    // 安全复制字符串
    strncpy(node->name, name, MAX_NAME_LEN - 1);
    node->name[MAX_NAME_LEN - 1] = '\0'; // 确保以 null 结尾    

    node->child_count = 0;
    node->capacity = 2;
    node->children = (ProcessNode**)malloc(sizeof(ProcessNode*) * node->capacity);

    return node;
}

void add_child(ProcessNode* parent, ProcessNode* child) {
    if (!parent || !child) return;
    if (parent->child_count >= parent->capacity) {
        parent->capacity *= 2;

        ProcessNode** new_children = realloc(
            parent->children,
            sizeof(ProcessNode*) * parent->capacity
        );

        if (!new_children) {
            printf("内存分配失败！\n");
            return;
        }
        parent->children = new_children;
    }

    parent->children[parent->child_count++] = child;
}

void free_tree(ProcessNode* node) {
    if (!node) return;

    // 先释放所有子节点
    for (int i = 0; i < node->child_count; i++) {
        free_tree(node->children[i]);
    }

    // 释放存放子节点指针的数组
    free(node->children);
    
    // 最后释放自己
    free(node);
}

// 3. 查找节点
ProcessNode* find_node(int pid) {
    ProcessNode* node = NULL;
    
    // 注意这里是 &pid (取地址)
    HASH_FIND_INT(map, &pid, node); 
    
    return node;
}

// 4. 添加节点
void add_node_map(ProcessNode *insert) {
    ProcessNode* node = NULL;

    // 先检查是否已经存在
    HASH_FIND_INT(map, &insert->pid, node);

    if (node == NULL) {
        // pid 是结构体内的字段名
        HASH_ADD_INT(map, pid, insert); 
    }
}

// 5. 删除节点
void delete_node(int pid) {
    ProcessNode* node = find_node(pid);

    if (node != NULL) {
        HASH_DEL(map, node); // 从哈希表中移除
        // 注意：这里是否 free(node) 取决于你的内存管理策略
        // 如果你希望彻底删除，这里应该调用 free_tree(node) 或 free(node)
        free(node);
    }
}