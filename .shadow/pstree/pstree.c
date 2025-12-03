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
#define MAX_LINE_LEN 256
#define MAX_NAME_LEN 128

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
ProcessNode* create_node(const char* path);
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

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (isAllDigits(entry->d_name)) {
            char path[256];
            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
            ProcessNode *node = create_node(path);
            add_node_map(node);
        }
    }
    printf("\n");
    closedir(dir);

    ProcessNode *current_node, *tmp;
    HASH_ITER(hh, map, current_node, tmp) {
        ProcessNode *p_node = find_node(current_node->ppid);
        if (p_node != NULL) {
            add_child(p_node, current_node);
        }
    }

    ProcessNode *root = find_node(1);
    dfsPrintPstree(root, 0);
}

void dfsPrintPstree(ProcessNode *node, int deep) {
    
    char str[256];
    sprintf(str, sizeof(str), "%s", "");
    for (size_t i = 0; i < deep; i++) {
        sprintf(str, sizeof(str), "%s", "| ");
    }
    if (deep != 0) {
        sprintf(str, sizeof(str), "%s", "--");
    }
    
    printf("%s (%d, %s)", str, node->pid, node->name);
    for (size_t i = 0; i < node->child_count; i++) {
        dfsPrintPstree(node->children[i], deep + 1);
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

ProcessNode* create_node(const char* path) {

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("文件不存在");
    }

    // 用于存储提取结果的变量
    char name[MAX_NAME_LEN] = {0};
    int pid = -1;
    int ppid = -1;

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        
        // 2. 检查 "Name:" 字段
        if (strncmp(line, "Name:", 5) == 0) {
            // sscanf 会自动跳过 "Name:" 后的空白字符（空格或制表符）
            sscanf(line, "Name: %s", name);
        }
        // 3. 检查 "Pid:" 字段 (注意: Pid 只有3个字母，所以比较长度是4)
        else if (strncmp(line, "Pid:", 4) == 0) {
            sscanf(line, "Pid: %d", &pid);
        }
        // 4. 检查 "PPid:" 字段
        else if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line, "PPid: %d", &ppid);
        }

        // 优化：如果三个值都找到了，可以提前退出循环
        if (pid != -1 && ppid != -1 && name[0] != '\0') {
            break; 
        }
    }

    fclose(file);

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