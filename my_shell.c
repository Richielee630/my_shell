//Zhaoqin Li

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

#define clear() printf("\033[H\033[J")

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
		//"\n>pipe handling"
		//"\n>improper space handling"
		);

	return;
}

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

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
		// waiting for child to terminate
		wait(NULL);
		return 1;
	}
}

int exec_backg_Args(char* cmd_w_space[], int cmd_count){
	
	int pid; 
	int i;

	for (i = 0; i < cmd_count; i++){
		pid = fork();	
		if(pid == -1){ //fail
			printf("\nFailed forking child..");
		}else if (pid == 0){  //child
			char **parsed_0;
			parsed_0 = tokenize(cmd_w_space[i]);

			//printf("child fork returned: %d\n", (int)pid);
			if (execvp(parsed_0[0], parsed_0) < 0) {
				printf("Shell: Incorrect command");
				exit(EXIT_SUCCESS);
			}
		}
	}
	return 1;
}

int exec_seq_Args(char* cmd_w_space[], int cmd_count){
    int pid;
	int i;

	for(i = 0; i < cmd_count; i++){

    	pid = fork();
		if (pid == -1) {
			printf("\nFailed forking child..");
			return 0;
		} else if (pid == 0) {
			char **parsed_0;
			parsed_0 = tokenize(cmd_w_space[i]);
			//if (!ownCmdHandler(parsed_0)){
				if (execvp(parsed_0[0], parsed_0) < 0) {
					printf("Shell: Incorrect command");
					//return 0;
					exit(EXIT_SUCCESS);
				}			
			//}
			//exit(0);
		} else {
			// waiting for child to terminate
			waitpid(pid, NULL, 0);
		}
	}
	return 1;
}

int exec_Para_Args(char* cmd_w_space[], int cmd_count){
	
	int pid;
    //int status;
	int i;

	for(i = 0; i < cmd_count; i++){

    	pid = fork();
		if (pid == -1) {
			printf("\nFailed forking child..");
			return 0;
		} else if (pid == 0) {
			char **parsed_0;
			parsed_0 = tokenize(cmd_w_space[i]);
			//if (!ownCmdHandler(parsed_0)){
				if (execvp(parsed_0[0], parsed_0) < 0) {
					printf("Shell: Incorrect command");
					//return 0;
					exit(EXIT_SUCCESS);
				}
				//exit(0);			
			//}
			//exit(0);
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
		exit(0);
	case 2:
		if (parsed[2] != NULL){
			printf("Shell: Incorrect command");
		}else{
			chdir(parsed[1]);
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
	char* background_token = "&";
	//int background_token_int = 38;
	
	char* sequential_token = "&&";
	//int sequential_token_int = 9766;
	//const char sequential_token = '&&';

	char* parallel_token = "&&&";
	//int parallel_token_int = 2500134;
	
	int tokes_lengths = countTokens(tokens);
	//printf("tokens number %d\n", tokes_lengths);
	// char *commands_arr[tokes_lengths];
	//printf("len: %d",strlen(line));
	int i = 0;
	//int j = 0;
	//int actual_len = 0;
	// int background_mode = 1;
	// int sequential_mode = 2;
	// int parallel_mode = 3;

	if(strstr(line, parallel_token) != NULL){
		char *commands_arr[tokes_lengths]; 
		char *token_from_str;
		//char **tokenized_token;

		token_from_str = strtok(line, parallel_token);
		while (token_from_str != NULL){
			//printf("%s\n", token_from_str);
			commands_arr[i] = token_from_str;
			//printf("%s\n", commands_arr[i]);
			token_from_str = strtok(NULL, parallel_token);	
			i++;  	//i is the real commands counts exept &&
			//printf("%d ", i);
		}

		// for (j = 0; j < i; j ++){   //only [0] ~ [i-1] elements in commands_arr are useful
		// 	//printf("...%s\n", commands_arr[j]);
		// 	tokenized_token = tokenize(commands_arr[j]);
		// 	//executeCommand(tokenized_token, commands_arr[j], &parallel_mode);
		// }

		exec_Para_Args(commands_arr,i);
		return 1;

	}else if(strstr(line, sequential_token) != NULL){  //&&
		char *commands_arr[tokes_lengths];    // this is an array of pointer 
		// char *commands_arr_ptr = commands_arr;
		// char (*ptr_ptr)[tokes_lengths];
		// ptr_ptr = &commands_arr_ptr;
		//printf("here");
		char *token_from_str;
		//char **tokenized_token;
		//char ***sqe_token_group;
		//a pointer of a pointer is a pointer to an array of pointers

		token_from_str = strtok(line, sequential_token);
		while (token_from_str != NULL){
			//printf("%s\n", token_from_str);
			commands_arr[i] = token_from_str;
			//printf("%s\n", commands_arr[i]);
			token_from_str = strtok(NULL, sequential_token);	
			i++;  	//i is the real commands counts exept &&
			//printf("%d ", i);
		}
		// for (j = 0; j < i; j ++){
		// 	//printf("...%s\n", commands_arr[j]);
		// 	tokenized_token = tokenize(commands_arr[j]);

		// 	//sqe_token_group[j] = tokenized_token;

		// 	//executeCommand(tokenized_token, commands_arr[j], &sequential_mode);

		// 	//printf(" total commands: %s", commands_arr[j]);
		// }

		exec_seq_Args(commands_arr, i);

		return 1;

	}else if(strstr(line, background_token) != NULL){
		char *commands_arr[tokes_lengths];   //array of char *ptr 
		char *token_from_str;
		//char **tokenized_token;
		
		token_from_str = strtok(line, background_token);
		while (token_from_str != NULL){
			//printf("%s\n", token_from_str);
			commands_arr[i] = token_from_str;
			//printf("%s\n", commands_arr[i]);
			token_from_str = strtok(NULL, background_token);	
			//printf("...%s\n",commands_arr[i]);
			i++;  	//i is the real commands counts exept &&
			//printf("%d ", i);
		}
		
		// test[0] = commands_arr[0];
		// printf("...%s, %s",test[0],commands_arr[0]);
		
		//print_ptr_arr(commands_arr);
		// //printf("...exec, i = %d\n", i);

		// for (j = 0; j < i; j ++){   //only [0] ~ [i-1] elements in commands_arr are useful
		// 	//printf("...%s\n", commands_arr[j]);
		// 	tokenized_token = tokenize(commands_arr[j]);
		// 	//executeCommand(tokenized_token, commands_arr[j], &background_mode);
		// 	//exec_seq_Args(tokenized_token);
		// }

		exec_backg_Args(commands_arr, i);
		return 1;
		
	}
	return 0;

}

void init_shell()
{
	clear();
	printf("\n\n\n\n******************"
		"************************");
	printf("\n\n\n\t******MY_SHELL******");
	printf("\n\n\t-CMPE142 HOWEWORK 3-");
	printf("\n\n\n\n*******************"
		"***********************");
	char* username = getenv("USER");
	printf("\n\n\nUSER is: @%s", username);
	printf("\n");
	sleep(2);
	clear();
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

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	//int execFlag = 0;
	//execlp("ls", "ls", (char *)NULL);

	// init_shell();
	// clear();

	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {		

		while(waitpid(-1, NULL, WNOHANG) > 0){  //
			printf("Shell: Background process finished");
		}	
		/* BEGIN: TAKING INPUT */
		// printDir();

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

		// printf("Command entered: %s (remove this debug output later)\n", line);
		
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		if (!executeCommand(tokens, line)){
			continue;
		}

		//printf("...clear mem\n");
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
