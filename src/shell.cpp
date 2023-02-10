#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <dirent.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

//Colors, XXm=color, 0=normal, 1=light, 2=faint, 3=normal, 4=underline, 5,6=blink, 7=background
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"
#define clear() printf("\033[H\033[J")
#define clearline() printf("\33[2K\r")
#define MAXBUF 255
#define home_dir getpwuid(getuid()) -> pw_dir

int getColumn() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

int ls_default = 0;      // default listing format
int bgPid[MAXBUF];       // used to save pid values of background processes
char *bgCommand[MAXBUF]; // used to save command names of background processes
int sp = 0;              // sp works as stack pointer
int p[2];                // pipe
bool pending_input = false;

// String hashing
constexpr unsigned int hash(const char *s, int off = 0) { return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off]; }
constexpr inline unsigned int operator "" _(char const * p, size_t) { return hash(p); }

// ------------ Pipe -------------- //
static inline void pipe_init() {
    if (pipe(p)) printf(LIGHT_RED "Pipe initialize Error\n" NONE); fcntl(p[0], F_SETFL, O_NONBLOCK); fcntl(p[1], F_SETFL, O_NONBLOCK);
}
static inline void read_sys_message() {
    char pip_msg[MAXBUF];
    while (read(p[0], pip_msg, MAXBUF) > 0) printf("%s\n", pip_msg);
}
static inline void write_sys_message(char message[MAXBUF]) {
    usleep(5000);
    if (write(p[1],message, MAXBUF) < 0) printf(LIGHT_RED "Pipe Error\n" NONE);
}
// -------------------------------- //
static inline void printCenter(const char *str, bool newLine) { printf("%*s%s", (int)((getColumn() - strlen(str))/2), "", str); if(newLine) printf("\n"); }
void shell_init() {
    clear(); printf(LIGHT_CYAN);
    char* username = getenv("USER"); int x = strlen(username);
    for (int i = 0; i <= 16; i++) {
        switch(i) {
            case 0 ... 5: case 7: printCenter("*                  *", true); break;
            case 6: printCenter("*   Initializing   *", true); break;
            case 8: printCenter("********************", true); break;
            case 9 ... 13: clearline(); printCenter("|", false); printf("\n"); printCenter("V", false); fflush(stdout); usleep(100000); break;
            case 14: clearline(); printCenter("V", true); break;
            case 15: printf(LIGHT_PURPLE "%*s%s\n" LIGHT_CYAN, (getColumn()+x+9)/2-x, "Welcome, ", username); break;
            case 16: printCenter("----------- My Shell -----------", true); break;
        }
        usleep(50000);
    } printf(NONE);
}

static inline bool checkDirectoryExist(char *path) {
    if (!strcmp(path, "~")) strcpy(path, home_dir);
    struct stat info;
    return (stat(path, &info) == 0);
}

static inline int cd(char *dest){
    if (checkDirectoryExist(dest)) return chdir(dest);
    else return -1;
}

static inline void setFlags(bool *a, bool *l) {
    switch (ls_default) {
        case 1: *l = true; break;
        case 2: *a = true; break;
        case 3: *a = true; *l = true; break;
    }
}
int ls(char **inst, int args, char **path) {
    bool a = false, l = false;
    setFlags(&a, &l);
    for (int i = 1; i < args; i++) { // Syntax parsing
        if (inst[i][0] == '-') { // Flag handling
            int j = 1;
            while(inst[i][j] != '\0') {
                switch(inst[i][j]) {
                    case 'a': a = true; break;
                    case 'l': l = true; break;
                    case 'd': {
                        int length = 0, mode = -1; char num[MAXBUF] = "";
                        while(inst[i][j+1] != '\0' && inst[i][j+1] >= '0' && inst[i][j+1] <= '9') {
                            num[length] = inst[i][j+1];
                            length++; j++;
                            mode = atoi(num);
                        }
                        if (mode >= 0 && mode <= 3) {
                            ls_default = mode;
                            setFlags(&a, &l);
                            printf(LIGHT_PURPLE "Changed Default Listing mode to: %d\n" NONE, mode);
                        } else return 1;
                        break;
                    }
                    default: return 1;
                }
                j++;
            }
            if (j == 1) return 1;
            else if (i == args - 1) *path = (char *) ".";
        } else if (i == args - 1) *path = inst[i];
        else return 1;
    }
    struct dirent **d; char filename[MAXBUF], format[10] = "%s";
    if (checkDirectoryExist(*path)) {
        char cur_Dir[MAXBUF];
        if (getcwd(cur_Dir, MAXBUF) == NULL) return 1;
        cd(*path);
        int n = scandir(".", &d, 0, versionsort), fileNo = 1, maxLength = 0, numPerLine = 100, lineSize = 0;
        if (!l) {
            for (int i = 0; i < n; i++) { //get longest file length
                strcpy(filename, d[i]->d_name);
                if (!a && filename[0] == '.') continue;
                int length = strlen(filename);
                lineSize += length + 2;
                if (maxLength < length) maxLength = length;
            }
            int x = getColumn();
            if (lineSize > x) { //if cant fit into 1 line, change formatting
                numPerLine = (x - maxLength) / (maxLength + 2) + 1;
                if (numPerLine < 1) numPerLine = 1;
                sprintf(format, "%%-%ds", maxLength);
            }
        }
        for (int i = 0; i < n; i++) {
            strcpy(filename, d[i]->d_name);
            if (!a && filename[0] == '.') continue;
            if (l) printf(LIGHT_PURPLE "%-7d " NONE, fileNo);
            if (d[i]->d_type == DT_DIR) printf(LIGHT_BLUE);
            else if (access(d[i]->d_name, X_OK) != -1) printf(LIGHT_CYAN);
            printf(format, filename); printf(NONE);
            if (l || (!l && fileNo % numPerLine == 0) || i == n-1) printf("\n");
            else printf("  ");
            fileNo++;
        }
        free(d);
        cd(cur_Dir);
        return 0;
    } else return -1;
}

void killProcess(int pid) {
    int i = 0; char msg[MAXBUF];
    for (i; i < sp; i++){ if (bgPid[i] == pid) break;}
    if (i < sp) {
        snprintf(msg, MAXBUF, "[%d]   Done\t%d:\t%s", i+1, pid, bgCommand[i]);
        sp --;
        for (int j = i; j < sp; j++) {
            bgPid[j] = bgPid[j+1];
            bgCommand[j] = bgCommand[j+1];
        }
        write_sys_message(msg);
    }
    kill(pid, SIGTERM);
}

static inline void show_jobs() {
    for (int i = 0; i < sp; i ++) printf("[%d]   Running\t%d:\t%s\n", i+1, bgPid[i], bgCommand[i]); 
}

typedef struct pthreadArgs { char **inst; char command[MAXBUF]; } pthreadArgs;
void *bgExecution(void *arg) {
    pthreadArgs pA = *((pthreadArgs*)arg);
    pid_t pid = fork();
    if (pid == 0) {
        int out = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        dup2(out, 1); close(out); 
        if (execvp(pA.command,pA.inst) == -1) {
            char msg[MAXBUF];
            sprintf(msg, LIGHT_RED "\n%s: Wrong Command: Can not be Executed" NONE, pA.inst[0]);
            write_sys_message(msg);
        }
    } else {
        printf("[%d] %d\n", sp+1, pid);
        bgPid[sp] = pid; bgCommand[sp] = pA.command; sp++;
        waitpid(pid, NULL, 0);
        killProcess(pid);
    }
    pthread_exit(NULL);
}
void runBackground(char **inst, int args) {
    if (strstr(inst[0], "shell") != NULL) printf(LIGHT_RED "Forbidded Command: Try other commands\n" NONE);
    else {
        pthread_t bgThread; pthreadArgs pA;
        pA.inst = inst;
        strcpy(pA.command, inst[0]);
        if (pthread_create(&bgThread, NULL, bgExecution, &pA)) printf(LIGHT_RED "Pthread Error: Please try again\n" NONE);
        usleep(1000);
    }
}

void runForeground(char **inst, int args) {
    args--;
    if (!strcmp(inst[args],"bg") || !strcmp(inst[args], "&")) {
        inst[args] = NULL;
        runBackground(inst, args);
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(inst[0], inst) == -1) printf(LIGHT_RED "\n%s: Wrong Command: Can not be Executed\n" NONE, inst[0]); 
            kill(getpid(), SIGTERM);
        }
        else { waitpid(pid, NULL, 0); }
    }
}

static inline void getPromptMsg(char *wd) {
    char cur_Dir[MAXBUF];
    strcat(strcat(strcat(wd, LIGHT_GREEN "MyShell" NONE ":" LIGHT_BLUE), getcwd(cur_Dir, MAXBUF)), NONE "> ");
}

inline int get_input(char *input) {
    char wd[MAXBUF] = "", *str;
    getPromptMsg(wd);
    pending_input = true;
    str = readline(wd);
    pending_input = false;
    if (strlen(str) != 0) {
        add_history(str);
        strcpy(input, str);
        return 0;
    } else return 1;
}

int handle_input(char *input) {
    char *inst[MAXBUF], *path; int args = 0, error_flag = 0; bool background = false;
    for (args; args < MAXBUF; args++) {
        inst[args] = strsep(&input, " ");
        if (inst[args] == NULL) break;
        else if (strlen(inst[args]) == 0) args--;
    }
    switch(hash(inst[0])) {
        case "quit"_: return 1;
        case "cd"_:
            if (args > 2) { error_flag = 1; break; }
            else if (args == 1) path = home_dir;
            else path = inst[1];
            error_flag = cd(path);
            break;
        case "ls"_:
            path = (char *)".";
            error_flag = ls(inst, args, &path);
            break;
        case "l"_:
            if (args == 1) {
                inst[1] = (char*) "-l"; args++;
                error_flag = ls(inst, args, &path);
            } else error_flag = 1;
            break;
        case "ll"_:
            if (args == 1) {
                inst[1] = (char*) "-al"; args++;
                error_flag = ls(inst, args, &path);
            } else error_flag = 1;
            break;
        case "bg"_: background = true;
        case "fg"_:
            if (args == 1) error_flag = 2;
            else {
                for (int i = 1; i < args; i++) inst[i-1] = inst[i];
                args--; inst[args] = NULL;
                if (background) runBackground(inst, args); else runForeground(inst, args);
            } 
            break;
        case "kill"_: for (int process = 1; process < args; process++) killProcess(atoi(inst[process])); break;
        case "jobs"_: if (args == 1) show_jobs(); else error_flag = 1; break;
        default: runForeground(inst, args);
    }
    switch(error_flag) {
        case 0: return 0;
        case -1: printf(LIGHT_RED "%s: No such file or directory\n" NONE, path); break;
        case 1: printf(LIGHT_RED "Wrong Syntax: Unrecognized Command\n" NONE); break;
        case 2: printf(LIGHT_RED "Nothing to run: Invalid Command\n" NONE); break;
        case 5: printf(LIGHT_RED "System Error\n" NONE); break;
    }
    return 0;
}

void handler(int num) { //handler for Ctrl C
    char wd[MAXBUF] = ""; getPromptMsg(wd);
    if (pending_input) printf("\n%s", wd);
    else printf("\n");
} 
int main() {
    char input[MAXBUF];
    pipe_init();
    shell_init();
    signal(SIGINT, handler);
    while(1) {
        read_sys_message();
        if (get_input(input)) continue;
        if (handle_input(input)) break;
    }
    return 0;
}
