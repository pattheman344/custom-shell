#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char **argv){
    char buff[256];
    char *tokenarr[64];
    char cwd[100];
    int count = 0;
    int status;
    pid_t p;
    while(1){
        count = 0;
        printf("~%s\n", getcwd(cwd, 100));
        printf("> ");
        fflush(stdout);
        fgets(buff, sizeof(buff), stdin);
        buff[strcspn(buff, "\n")] = '\0';
        char *tokens = strtok(buff, " ");
        while(tokens != NULL){
            tokenarr[count] = tokens;
            count++;
            tokens = strtok(NULL, " ");
        }
        tokenarr[count] = NULL;
        if(strcmp(tokenarr[0], "exit") == 0){
            exit(0);
        } else if(strcmp(tokenarr[0], "cd") == 0){
            if(!tokenarr[1]){
                printf("Directory not specified.\n");
                continue;
            }
            chdir(tokenarr[1]);
            continue;
        }
        p = fork();
        if(p<0){
            fprintf(stderr, "fork fail");
        } else if(p == 0){
            execvp(tokenarr[0], tokenarr);
        } else{
            waitpid(p, &status, 0);
        }
    }
        return 0;
}