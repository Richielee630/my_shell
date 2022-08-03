//Zhaoqin Li

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

#define clear() printf("\033[H\033[J")

static int FORG_CHILD_PID = 0;
static int NUMBER_OF_BG_CHILD = 0;
static pid_t PGID = 0;
static pid_t SEQ_PID = 0;
static bool SEQ_PS_STATUS = false;

struct p_type{
	int type_id;
};

typedef struct p_type process_t;

static process_t cur_process = {.type_id = 0};
/* Splits the string by space and returns the array of tokens
*
*/

int executeCommand(char **tokens, char *line);
int ownCmdHandler(char** parsed);

void openHelp()
{
	puts("\n***WELCOME TO MY_SHELL HELP***"
		"\nList of Commands supported:"
		"\n>cd"
		"\n>ls"
		"\n>exit"
		"\n>all other general commands available in UNIX shell"
		);

	return;
}

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int tokenIndex = 0, tokenNo = 0;
  size_t i;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
		tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
		strcpy(tokens[tokenNo++], token);
		tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

int countTokens(char **tokens){
	int count = 0;
	while (tokens[count] != NULL){
		count++;
	}
	return count;
}

int execArgs(char** parsed){
	// Forking a child
	pid_t pid = fork();
	cur_process.type_id = 0;
	if (pid == -1) {
		printf("\nFailed forking child..");
		return 0;
	} else if (pid == 0) {	
		if (execvp(parsed[0], parsed) < 0) {
			printf("Shell: Incorrect command");
			return 0;
		}
		exit(0);
		return 1;
	} else {
		FORG_CHILD_PID = (int)pid;
		///printf("\nchild pid is %d", (int)pid);
		wait(NULL);
		FORG_CHILD_PID = 0;
		return 1;
	}
}

void child_continue(int sig){
	if(sig == SIGCONT){
		return;
	}
}

int exec_backg_Args(char* cmd_w_space[], int cmd_count){
	
	int pid; 
	int i;
	cmd_count = cmd_count;
	cur_process.type_id = 1;
	for (i = 0; i < 1; i++){
		pid = fork();	
		if(pid == -1){ //fail
			return 0;
		}else if (pid == 0){  //child
			setpgid(getpid(),PGID); // the child's GPID is same as PID
			kill(getppid(), SIGCONT);
			char **parsed_0;
			parsed_0 = tokenize(cmd_w_space[i]);
			if (execvp(parsed_0[0], parsed_0) < 0) {
				printf("Shell: Incorrect command");
				exit(EXIT_SUCCESS);
			}
		}else{
			signal(SIGCONT, child_continue);
			pause();
			//PGID = getpgid(pid);
			if(!PGID){
				PGID = getpgid(pid);
			}
			NUMBER_OF_BG_CHILD += 1;
		}
	}
	return 1;
}

int exec_seq_Args(char* cmd_w_space[], int cmd_count){
    int pid;
	int i;
	cur_process.type_id = 2; 
	for(i = 0; i < cmd_count; i++){

    	pid = fork();
		if (pid == -1) {
			return 0;
		} else if (pid == 0) {
			char **parsed_0;
			parsed_0 = tokenize(cmd_w_space[i]);
			if (execvp(parsed_0[0], parsed_0) < 0) {
				printf("Shell: Incorrect command");
				//return 0;
				exit(EXIT_SUCCESS);
			}			
		} else {
			SEQ_PID = pid;
			waitpid(pid, NULL, 0);
			if(SEQ_PS_STATUS){
				SEQ_PS_STATUS = false;
				break;
			}	
		}
	}
	return 1;
}

int exec_Para_Args(char* cmd_w_space[], int cmd_count){
	
	int pid;
    //int status;
	int i;
	cur_process.type_id = 3;
	for(i = 0; i < cmd_count; i++){

    	pid = fork();
		if (pid == -1) {
			return 0;
		} else if (pid == 0) {
			char **parsed_0;
			parsed_0 = tokenize(cmd_w_space[i]);
			if (execvp(parsed_0[0], parsed_0) < 0) {
				printf("Shell: Incorrect command");
				exit(EXIT_SUCCESS);
			}
		} 

	}

	for(i = 0; i < cmd_count; i++){
		wait(NULL);
	}

	return 1;
}


int ownCmdHandler(char** parsed)
{
	int NoOfOwnCmds = 4, i, switchOwnArg = 0;
	char* ListOfOwnCmds[NoOfOwnCmds];
	char* username;

	ListOfOwnCmds[0] = "exit";
	ListOfOwnCmds[1] = "cd";
	ListOfOwnCmds[2] = "help";
	ListOfOwnCmds[3] = "hello";

	for (i = 0; i < NoOfOwnCmds; i++) {
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
			switchOwnArg = i + 1;
			break;
		}
	}

	switch (switchOwnArg) {
	case 1:
		// printf("\nGoodbye\n");
		if(NUMBER_OF_BG_CHILD){
			killpg(PGID, SIGKILL);
			while(waitpid(-1, NULL, WNOHANG) > 0){  //
				;
			}
		}
		exit(0);
	case 2:
		if (parsed[2] != NULL){
			printf("Shell: Incorrect command");
		}else{
			// chdir(parsed[1]);
			if(chdir(parsed[1]) == -1){
				printf("Shell: Incorrect command");
			}
		}
		return 1;
	case 3:
		openHelp();
		return 1;
	case 4:
		username = getenv("USER");
		printf("\nHello %s.\nMind that this is "
			"not a place to play around."
			"\nUse help to know more..\n",
			username);
		return 1;
	default:
		break;
	}
	return 0;
}


void print_ptr_arr(char* target[]){
	printf("target 0: %s\n", target[0]);
	printf("target 1: %s\n", target[1]);
}

int specialExecutionHandler(char** tokens, char* line){
	// echo 2 && echo 1 && echo 23 
	// char* line is a char array = {'e','c','h','o',' ','1',' ','&','&',' ','e','c','h','o',' ','2'}
	//						      = "echo 2 && echo 1 && echo 23"
	// char** tokens(0) = [['echo', '2']]
	// char** tokens(1) = [['echo', '1']]
	// char** tokens(2) = [['echo', '23']]

	// char** commands_arr = {'echo 2 ',' echo 1 ',' echo 23'}
	
	char* background_token = "&";
	char* sequential_token = "&&";
	char* parallel_token = "&&&";
	int tokes_lengths = countTokens(tokens);
	int i = 0;

	if(strstr(line, parallel_token) != NULL){  //is to check if parallel_token is exsit in line
		char *commands_arr[tokes_lengths]; 
		char *token_from_str;

		token_from_str = strtok(line, parallel_token);
		while (token_from_str != NULL){
			commands_arr[i] = token_from_str;
			token_from_str = strtok(NULL, parallel_token);	
			i++;  	//i is the real commands counts exept &&
		}
		char **test;
		test = commands_arr;
		exec_Para_Args(test, i);
		return 1;

	}else if(strstr(line, sequential_token) != NULL){  //&&
		char *commands_arr[tokes_lengths];    // this is an array of pointer 
		char *token_from_str;

		token_from_str = strtok(line, sequential_token);
		while (token_from_str != NULL){
			commands_arr[i] = token_from_str;
			token_from_str = strtok(NULL, sequential_token);	
			i++;  	//i is the real commands counts exept &&
			//printf("%d ", i);
		}

		char **test;
		test = commands_arr;
		exec_seq_Args(test, i);

		return 1;

	}else if(strstr(line, background_token) != NULL){
		char *commands_arr[tokes_lengths];   //array of char *ptr 
		char *token_from_str;
		//char **tokenized_token;
		
		token_from_str = strtok(line, background_token);
		while (token_from_str != NULL){
			commands_arr[i] = token_from_str;
			token_from_str = strtok(NULL, background_token);	
			i++;  	//i is the real commands counts exept &&
		}
		
		char **test;
		test = commands_arr;
		exec_backg_Args(test, i);
		return 1;
		
	}
	return 0;

}

void printDir(){
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\nDir: %s\n", cwd);
}

int executeCommand(char **tokens, char *line){
	//printf("line len: %ld\n", strlen(line));
	if (strlen(line) == 1){   //return 0 when there is no input while hit RETURN
		return 0;
	}

	//printf("run mode =%d\n", *run_mode);
	
	if (!specialExecutionHandler(tokens, line)){
		if (!ownCmdHandler(tokens)){
			if (!execArgs(tokens)){
				return 0;
			}
			return 1;
		}
		return 1;
	}
	return 1;
}

int isForeground() {
    pid_t pid = tcgetpgrp(STDIN_FILENO);
    if(pid == -1 /* piped */ || pid == getpgrp() /* foreground */)
        return (int)pid;
    /* otherwise background */
    return 0;
}

void INThandler(int sig){
	signal(sig, SIG_IGN);
	switch (cur_process.type_id)
	{
	case 1:  //for background 
		break;

	case 2: //for sequential
		// printf("here");
		SEQ_PS_STATUS = true;
		kill(SEQ_PID, SIGKILL);
		break;

	case 3:
		killpg(getppid(), SIGTERM);
		break;
	}

	cur_process.type_id = 0;
	signal(SIGINT, INThandler);

	// signal(sig, SIG_IGN);
	// if(FORG_CHILD_PID){
	// 	printf("\npid = %d killed\n", FORG_CHILD_PID);
	// 	kill(FORG_CHILD_PID, SIGINT);
	// }else{
	// 	signal(SIGINT, INThandler);
	// }

	// int pid = isForeground();
	// if (pid && sig == SIGINT){
	// 	printf("\npid = %d killed\n", pid);
	// 	kill(pid, SIGINT);
	// }else{
	// 	signal(SIGINT, INThandler);
	// }


	//printf("\nConfirm exit? [y/n]");
	// int ch = getchar();
	// if (ch == 'y' || ch == 'Y'){
	// 	printf("\nforeground process killed.\n");
	// 	exit(0);
	// }else{
	// 	signal(SIGINT, INThandler);
	// 	getchar();
	// }
}

// #include <pthread.h>
// #include <semaphore.h>
// sem_t binary;

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp == NULL) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {
		signal(SIGTERM, SIG_IGN);

		if (signal(SIGINT, INThandler) == SIG_ERR){
			printf("\ncan't catch SIGINT\n"); 
		} 
  			
		while(waitpid(-1, NULL, WNOHANG) > 0){  //
			NUMBER_OF_BG_CHILD -= 1;
			if(!NUMBER_OF_BG_CHILD){
				PGID = 0;
			}
			printf("Shell: Background process finished");
		}	

		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}


		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		if (!executeCommand(tokens, line)){
			continue;
		}

		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
		
	}
	return 0;
}
