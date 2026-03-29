#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    char buff[256];
    char *tokenarr[64];
    int count = 0;
    int status;
    pid_t p;
    while(1){
        count = 0;
        fgets(buff, sizeof(buff), stdin);
        buff[strcspn(buff, "\n")] = '\0';
        char *tokens = strtok(buff, " ");
        while(tokens != NULL){
            tokenarr[count] = tokens;
            count++;
            tokens = strtok(NULL, " ");
        }
        tokenarr[count] = NULL;
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