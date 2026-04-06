#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

typedef struct{
    pid_t pid;
    char *jobname; 
} jobs;

jobs arr[64];

int job_count = 0;

char *cmd_history[100];
int cmd_counter = 0;

void handle_sigchld(int sig){
    int x;
    while((x = waitpid(-1, NULL, WNOHANG)) > 0){
        for(int i=0; i < job_count; i++){
            if(arr[i].pid == x){
                for(int j=i; j < job_count - 1; j++){
                    arr[j] = arr[j+1];
                }
                job_count--;
            }
        }

    }
}

int main(int argc, char **argv){
    char buff[256];
    char *tokenarr[64];
    char cwd[100];
    int fd[2];
    int count = 0;
    int status;
    pid_t p, p2, p3;
    char *cmd2;
    char *file1, *file2, *file3;
    int pipe_index;
    int filefd;
    int backgroundprocess;
    system("clear");
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);
    
    while(1){
        cmd2 = NULL;
        file1 = NULL;
        file2 = NULL;
        file3 = NULL;
        count = 0;
        backgroundprocess = 0;
        printf("~%s\n", getcwd(cwd, 100));
        printf("> ");
        fflush(stdout);
        if(fgets(buff, sizeof(buff), stdin) == NULL) exit(0);
        buff[strcspn(buff, "\n")] = '\0';
        if(buff[0] == '\0') continue;
        if(strcmp(buff, "history") != 0){
            cmd_history[cmd_counter]= strdup(buff);
            cmd_counter++;
        }
        char *tokens = strtok(buff, " ");
        while(tokens != NULL){
            tokenarr[count] = tokens;
            count++;
            tokens = strtok(NULL, " ");
        }
        tokenarr[count] = NULL;
        if(tokenarr[0] == NULL){
            continue;
        }
        if(strcmp(tokenarr[0], "exit") == 0){
            exit(0);
        } else if(strcmp(tokenarr[0], "cd") == 0){
            if(!tokenarr[1]){
                printf("Directory not specified.\n");
                continue;
            }
            chdir(tokenarr[1]);
            continue;
        } else if(strcmp(tokenarr[0], "jobs") == 0){
            for(int i=0; i < job_count; i++){
                printf("[%d] %d %s\n", i+1, arr[i].pid, arr[i].jobname);
            }
            continue;
        } else if(strcmp(tokenarr[0], "history") == 0){
            for(int i=0; i < cmd_counter; i++){
                printf("%d. \"%s\"\n", i+1, cmd_history[i]);
            }
            continue;
        }
        count = 0;
        while(tokenarr[count] != NULL){
            if(strcmp(tokenarr[count], "|") == 0){
                tokenarr[count] = NULL;
                cmd2 = tokenarr[count+1];
                pipe_index = count;
            } else if(strcmp(tokenarr[count], ">>") == 0){
                tokenarr[count] = NULL;
                file3 = tokenarr[count+1];
            } else if(strcmp(tokenarr[count], ">") == 0){
                tokenarr[count] = NULL;
                file1 = tokenarr[count+1];
            } else if(strcmp(tokenarr[count], "<") == 0){
                tokenarr[count] = NULL;
                file2 = tokenarr[count+1];
            } else if(strcmp(tokenarr[count], "&" ) == 0){
                tokenarr[count] = NULL;
                backgroundprocess = 1;
            }
            count++;
        }
        if(cmd2 != NULL){
            if(pipe(fd) == -1){
                fprintf(stderr, "pipe fail");
                return 1;
            }
            p = fork();
            if(p<0){ fprintf(stderr, "fork fail"); exit(1);
            } else if(p == 0){
                dup2(fd[1], 1);
                close(fd[0]);
                execvp(tokenarr[0], tokenarr);
                exit(1);
            } else{
                close(fd[1]);
            }
            p2 = fork();
            if(p2<0){ fprintf(stderr, "fork fail"); exit(1);
            } else if(p2 == 0){
                dup2(fd[0], 0);
                close(fd[1]);
                execvp(cmd2, &tokenarr[pipe_index+1]);
                exit(1);
            } else{
                close(fd[1]);
                close(fd[0]);
                waitpid(p, &status, 0);
                waitpid(p2, &status, 0);
            }
        } else {
            p3 = fork();
            if(p3<0){
                fprintf(stderr, "fork fail");
                exit(1);
        } else if(p3 == 0){
            if(file1 != NULL){ // output >
                filefd = open(file1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(filefd != -1){
                dup2(filefd, 1);
                close(filefd);
                execvp(tokenarr[0], tokenarr);
                exit(1);
                }
            } else { execvp(tokenarr[0], tokenarr); exit(1); }
            if(file2 != NULL){ // input <
                filefd = open(file2, O_RDONLY);
                if(filefd != -1){
                dup2(filefd, 0);
                close(filefd);
                execvp(tokenarr[0], tokenarr);
                exit(1);
                }
            } else { execvp(tokenarr[0], tokenarr); exit(1); }
            if(file3 != NULL){ // append >>
                filefd = open(file3, O_WRONLY | O_CREAT | O_APPEND, 0644);
                if(filefd != -1){
                dup2(filefd, 1);
                close(filefd);
                execvp(tokenarr[0], tokenarr);
                exit(1);
                }
            } else {
                execvp(tokenarr[0], tokenarr);
                exit(1);
            }
        } else{
                if(backgroundprocess){
                    printf("pid: %d\n", p3);
                    arr[job_count].pid = p3;
                    arr[job_count].jobname = strdup(tokenarr[0]);
                    job_count++;
                } else waitpid(p3, &status, 0);
        }
        }

    }
        return 0;
}