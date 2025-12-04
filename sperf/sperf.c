#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#define BUF_SIZE 2048
#define MAX_SYSCALLS 1024
#define TOP_N 5

typedef struct {
    char name[64];
    double time;
    int count; // 增加计数统计
} syscall_stat;

typedef struct {
    syscall_stat stats[MAX_SYSCALLS];
    int count;
    double total_time;
} syscall_stats;

// 解析 strace 的输出行
// 典型格式: openat(AT_FDCWD, "file", O_RDONLY) = 3 <0.000123>
int parse_strace_line(char *line, char *syscall_name, double *time) {
    // 1. 查找左括号，之前的即为系统调用名
    char *open_paren = strchr(line, '(');
    if (!open_paren) return 0;

    // 2. 查找最后一个 '<'，之后直到 '>' 为时间
    char *start_angle = strrchr(line, '<');
    char *end_angle = strrchr(line, '>');
    
    if (!start_angle || !end_angle || end_angle < start_angle) return 0;

    // 提取名字
    int name_len = open_paren - line;
    if (name_len >= 64) name_len = 63;
    strncpy(syscall_name, line, name_len);
    syscall_name[name_len] = '\0';

    // 去除名字可能存在的意外前导空格（通常strace输出没有，但为了健壮性）
    // (此处略，strace通常行首即名字)

    // 提取时间字符串
    *end_angle = '\0'; // 临时截断字符串以方便转换
    *time = strtod(start_angle + 1, NULL);

    return 1;
}

void add_syscall(syscall_stats *stats, const char *name, double time) {
    // 简单的线性查找（生产环境可用哈希表优化）
    for (int i = 0; i < stats->count; i++) {
        if (strcmp(stats->stats[i].name, name) == 0) {
            stats->stats[i].time += time;
            stats->stats[i].count++;
            stats->total_time += time;
            return;
        }
    }

    // 如果没找到且还有空间，添加新条目
    if (stats->count < MAX_SYSCALLS) {
        strncpy(stats->stats[stats->count].name, name, 63);
        stats->stats[stats->count].name[63] = '\0'; // 确保终止
        stats->stats[stats->count].time = time;
        stats->stats[stats->count].count = 1;
        stats->count++;
        stats->total_time += time;
    }
}

// 用于 qsort 的比较函数（按时间降序）
int compare_syscalls(const void *a, const void *b) {
    syscall_stat *statA = (syscall_stat *)a;
    syscall_stat *statB = (syscall_stat *)b;
    if (statB->time > statA->time) return 1;
    if (statB->time < statA->time) return -1;
    return 0;
}

void print_top_syscalls(syscall_stats *stats, int n) {
    // 排序
    qsort(stats->stats, stats->count, sizeof(syscall_stat), compare_syscalls);

    printf("\n=== Top %d Slowest System Calls ===\n", n);
    printf("%-20s %-12s %-10s %s\n", "Syscall", "Total Time", "Count", "Avg Time");
    printf("----------------------------------------------------------\n");

    int limit = (n < stats->count) ? n : stats->count;
    for (int i = 0; i < limit; i++) {
        double avg = stats->stats[i].time / stats->stats[i].count;
        printf("%-20s %-12.6f %-10d %.6f\n", 
            stats->stats[i].name, 
            stats->stats[i].time, 
            stats->stats[i].count,
            avg);
    }
    printf("----------------------------------------------------------\n");
    printf("Total monitored time: %.6f seconds\n", stats->total_time);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program_to_trace> [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 构建参数列表
    // 必须确保 exec_argv 足够大。
    // strace + -T + 用户命令 + NULL
    char *exec_argv[argc + 3]; 
    exec_argv[0] = "strace";
    exec_argv[1] = "-T"; // 关键参数：显示系统调用耗时
    
    // 复制用户输入的命令和参数
    for (int i = 1; i < argc; i++) {
        exec_argv[i + 1] = argv[i];
    }
    exec_argv[argc + 1] = NULL; // 必须以 NULL 结尾

    char *exec_envp[] = { "PATH=/usr/bin:/bin", NULL }; // 扩充 PATH 以防万一

    int fd[2];
    // 1. 【修复】必须先初始化管道
    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        close(fd[0]);
        close(fd[1]);
        exit(EXIT_FAILURE);

    } else if (pid == 0) {
        // 子进程
        close(fd[0]); // 关闭读端

        // 2. 【修复】strace 输出到 stderr，所以我们要重定向 stderr 到管道
        dup2(fd[1], STDERR_FILENO); 
        
        // 也可以选择把 stdout 也重定向，防止用户程序的输出干扰（可选）
        // int devnull = open("/dev/null", O_WRONLY);
        // dup2(devnull, STDOUT_FILENO);

        close(fd[1]); // 关闭原写端

        // 注意：/bin/strace 路径因系统而异，建议用 execvp 会更灵活
        // 但为了保持原题结构，这里使用 execve，路径设为 /usr/bin/strace 比较通用，或者尝试 /bin/strace
        execve("/usr/bin/strace", exec_argv, exec_envp);
        
        // 如果上面失败了，尝试 /bin/strace
        execve("/bin/strace", exec_argv, exec_envp);

        perror("execve failed (check strace path)");
        exit(EXIT_FAILURE);
    } else {
        // 父进程
        close(fd[1]); // 关闭写端
        
        syscall_stats stats = {0}; // 初始化结构体

        // 3. 【优化】使用 fdopen 将文件描述符转换为 FILE 流，方便按行读取
        FILE *stream = fdopen(fd[0], "r");
        if (!stream) {
            perror("fdopen failed");
            exit(EXIT_FAILURE);
        }

        char line_buf[BUF_SIZE];
        char name_buf[64];
        double time_val;

        printf("Profiler started. Analyzing '%s'...\n", argv[1]);

        while (fgets(line_buf, BUF_SIZE, stream) != NULL) {
            // 解析每一行
            if (parse_strace_line(line_buf, name_buf, &time_val)) {
                add_syscall(&stats, name_buf, time_val);
            }
            // 可选：实时打印原始输出
            // printf("[RAW] %s", line_buf);
        }

        fclose(stream); // 这也会关闭 fd[0]
        
        // 等待子进程完全结束
        wait(NULL);

        print_top_syscalls(&stats, TOP_N);
    }

    return 0;
}