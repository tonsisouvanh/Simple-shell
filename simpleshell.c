#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h>

#include<sys/types.h> 
#include<sys/wait.h> 

#define MAX_LINE 80 /* The maximum length command */
#define space " "
#define hist_line 1000
#define hist_strlen 25
#define cmd_lenght 256
#define filename_lenght 256

//Global variable
int cmd_count = 0;
int hist_index = 0;
int saved_stdout = 0;
int saved_stdin = 0;
char redir_symbol[2];

void remove_newlineChar(char *str);
void print_history(char history_cmd[hist_line][hist_strlen]);
int  extract_number(char** args);
void add_history(char* buf,char history_cmd[hist_line][hist_strlen]);
int  get_Input(char* cmdLine, char history_cmd[hist_line][hist_strlen]);
int  my_cmd(char** args, char hist_cmd[hist_line][hist_strlen],char* input_filename, char* output_filename);
int  process_cmdLine(char* cmdLine, char** args, char** argsPipe, char history_cmd[hist_line][hist_strlen], char* input_filename, char* output_filename);
void execArgsNoPipe(char** args);
void splitToArgs(char* cmdLine, char** args);
void get_cmd_history(char** args, char history_cmd[hist_line][hist_strlen]);
int  redirect_output(char* output_filename);
int  redirect_input(char* input_filename);
void remove_redir(char** args, int index);
int  find_redirect_cmd(char** args);
int  redirection_cmd(char** args, char* input_filename, char* output_filename);

void process_ArgsLine(char* cmdLine, char** args);
void process_PipeLine(char* cmdLine, char** args, char** argsPipe);
void execArgsPipe(char** args, char** argsPipe);

void print_help();
void init_shell();

void init_shell(){
	printf("This is implementation of shell programming(Project 1):\n - type: 'help' to see the basic command\n");
}
void print_help(){
	int n = 7;
	char* help_list[n];
	help_list[0] = "cd - Change Directory”. When typed all by itself, it returns you to your home directory.";
	help_list[1] = "history - Print history of commands user have entered";
	help_list[2] = "pwd - Print Working Directory. Shows the current location in the directory tree.";
	help_list[3] = "!! - Run latest command that user have entered";
	help_list[4] = "! - Run the command with specific line: i<number>";
	help_list[5] = "clear - clear out the terminal";
	help_list[6] = "exit - exit the program";
	int i=0;	
	while(help_list[i] != NULL)
	{
		printf("[%d]. %s\n",i+1, help_list[i]);
		i++;
	}
}
void remove_newlineChar(char *str){
	int k = 0;
	while(str[k] != '\n'){
		k++;
	}
	str[k] ='\0';
}
//list of history
void print_history(char history_cmd[hist_line][hist_strlen])
{
	int i = 0;
	while(i < cmd_count)
	{
		printf("%d. %s\n", i+1, history_cmd[i]);
		i++;
	}
}
// extract number from string when user input !<number>
int extract_number(char** args) //here
{
	char c; 
	int i,digit,number=0; 
	char *temp;

	temp = args[0];
	
	for(i=1;i<strlen(args[0]);i++) 
	{ 
		c = *(temp+i);
		if(c>='0' && c<='9') //to confirm it's a digit 
		{ 
			digit = c - '0'; 
			number = number*10 + digit; 
		}
		
	}
	return number;
}
//--------------------------------------------//

//add command line history
void add_history(char* buf,char history_cmd[hist_line][hist_strlen]){
	if(hist_index == hist_line-1 || strcmp(buf, "history") == 0 || strcmp(buf, "!!") == 0 || strcmp(buf, "!") == 0)	
		return;
	
	strcpy(history_cmd[hist_index], buf);
	hist_index++;
	cmd_count++;
}

//input string
int get_Input(char* cmdLine, char history_cmd[hist_line][hist_strlen]){
	char buf[cmd_lenght];

	fgets(buf, cmd_lenght, stdin);
	remove_newlineChar(buf);

	if (strlen(buf) != 0 || strcmp(buf, "\0") == 0){

		add_history(buf, history_cmd);
		strcpy(cmdLine, buf);

		return 1;
	}
	else
		return 0;		

}

//function to execute the command  
int my_cmd(char** args, char hist_cmd[hist_line][hist_strlen], char* output_filename, char* input_filename){
	int noOfcmd = 4;
	int cmd_flag = -1,i = 0;
	char* my_cmd_list[noOfcmd];
	my_cmd_list[0] = "cd";
	my_cmd_list[1] = "history";
	my_cmd_list[2] = "exit";
	my_cmd_list[3] = "help";

	//check for redirection output and input for history and help command
	/*int k=0;
	while(args[k] != NULL)
	{
		if(strcmp(args[k], ">") == 0)
			if(redirect_output(args[k+1]))
			{
				strcpy(redir_symbol, ">");
				break;
			}
		if(strcmp(args[k], "<") == 0)
			if(redirect_input(args[k+1]))
			{
				strcpy(redir_symbol, "<");
				break;
			}
		k++;
	}*/

	for(i; i < noOfcmd ; i++)
	{
		if(strcmp(args[0], my_cmd_list[i]) == 0)
		{
			cmd_flag = i;
			break;
		}
	}
	
	if(cmd_flag >= 0){
		switch(cmd_flag)
		{
			case 0:
				chdir(args[1]);
				return 1;
			case 1:
				print_history(hist_cmd);
				return 1;
			case 2:
				exit(0);
			case 3:
				print_help();
				return 1;
			default:
				break;
		}
	}
	else
		return 0;
}

void remove_redir(char** args, int index)
{
	int i = index;
	while(args[i] != NULL)
	{
		args[i] = NULL;
		i++;
	}
}
int find_redirect_cmd(char** args)
{
	int i = 0;
	while(args[i] != NULL)
	{
		if(strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0)
			return i;
		i++;
	}
	return -1; //no < or > found in cmd line
}
int redirection_cmd(char** args, char* input_filename, char* output_filename)
{
	int i = find_redirect_cmd(args);

	if( i <= 0 || args[i+1] == NULL)
	{
		return 0; //if there is no <, > or no file input
	}
	else //found < or > in args
	{	
		strcpy(redir_symbol, args[i]);//assign < or > to variable char
		if(strcmp(args[i], ">") == 0)
		{
			output_filename = args[i+1];
			if(!redirect_output(output_filename))
				return 0;
		}
		if(strcmp(args[i], "<") == 0)
		{
			input_filename = args[i+1];
			if(!redirect_input(input_filename))
				return 0;
		}
		
		remove_redir(args, i);
	}
	return 1;
}
//new
void process_PipeLine(char* cmdLine, char** args, char** argsPipe)
{
	int i;
	for (i = 0; i < 100; i++) { 
		args[i] = strsep(&cmdLine, " ");
		if(strcmp(args[i], "|") == 0)
		{
			args[i] = NULL;
			break;
		}
		if (args[i] == NULL || strlen(args[i]) == 0) 
			break; 
	}

	int k;
	for (k = 0; k < 100; k++) { 
		argsPipe[k] = strsep(&cmdLine, " ");
		if (argsPipe[k] == NULL || strlen(argsPipe[k]) == 0) 
			break; 
	}

}
void process_ArgsLine(char* cmdLine, char** args)
{
	int i;
	for (i = 0; i < 100; i++) { 
		args[i] = strsep(&cmdLine, " "); 
		if (args[i] == NULL || strlen(args[i]) == 0) 
			break; 
	}
}
void execArgsPipe(char** args, char** argsPipe)
{
	int fd[2];  
    pid_t p1, p2; 
  
    if (pipe(fd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
  
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(fd[0]); 
        dup2(fd[1], STDOUT_FILENO); 
        close(fd[1]); 
  
        if (execvp(args[0], args) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 
    } 
	else { 
        // Parent executing 
        p2 = fork(); 
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) { 
            close(fd[1]); 
            dup2(fd[0], STDIN_FILENO); 
            close(fd[0]); 
            if (execvp(argsPipe[0], argsPipe) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
            // parent executing, waiting for two children

			close(fd[0]);
			close(fd[1]);

            wait(NULL); 
            wait(NULL); 
        } 
    } 
}
//split string line into args
int process_cmdLine(char* cmdLine, char** args, char** argsPipe, char history_cmd[hist_line][hist_strlen], char* input_filename, char* output_filename){

	//check if there is | in command line
	if(strspn("|",cmdLine) > 0)
	{
		process_PipeLine(cmdLine, args, argsPipe);
		return 2;
	}
	/*int i; 
    for (i = 0; i < 100; i++) { 
        args[i] = strsep(&cmdLine, space); 

        if (args[i] == NULL || strlen(args[i]) == 0) 
            break; 
    }*/
	process_ArgsLine(cmdLine,args);

	get_cmd_history(args, history_cmd);
	
	if(my_cmd(args, history_cmd, input_filename, output_filename))	// run own command
		return 0;
	if(redirection_cmd(args, input_filename, output_filename)) //handle redirect command
		return 1;
	else		//fail
		return 1;
	return 1;
}

//execute process without pipe
void execArgsNoPipe(char** args){
	pid_t pid;
	pid = fork();
	int f = 1;

	if(pid == -1)
		printf("Forking child fail!\n");
	if(pid == 0){
		if(execvp(args[0], args) < 0)
			printf("bash: s: command not found..s.\n");
		exit(0);
	}
	else{
		wait(NULL);
		return;
	}
}

//split inputted command line to args
void splitToArgs(char* cmdLine, char** args)
{
	
	int i; 
    for (i = 0; i < 100; i++) { 
        args[i] = strsep(&cmdLine, space); 

        if (args[i] == NULL || strlen(args[i]) == 0) 
            break; 
    }
}

//get recent command from history
void get_cmd_history(char** args, char history_cmd[hist_line][hist_strlen]) //cant let the command get in here
{
	if(hist_index == 0)
	{
		printf("No commands in history.\n");
		return;
	}
	if(strcmp(args[0], "!!") == 0)
	{
		splitToArgs(history_cmd[hist_index-1], args); //get latest command
		//return 1;
	}
	if(strspn(args[0],"!") > 0)
	{
		int x = extract_number(args);
		if(x <= hist_index-1){
			splitToArgs(history_cmd[x-1], args); //get specific command
			//return 1;
		}
		else{
			printf("No commands in history.\n");
			return;
		}
	}
	//return 1;
}
int redirect_output(char* output_filename)
{	
	int fdout = 0; //new file

	saved_stdout = dup(1);
	
	if ((fdout = open(output_filename, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
		perror(output_filename);	/* open failed */
		return 0; //fail
	}
	
	dup2(fdout, STDOUT_FILENO);
	close(fdout);
	return 1; //success
}
int redirect_input(char* input_filename)
{
	int fdin = 0; //new file
	
	saved_stdin = dup(STDIN_FILENO);
	

	if ((fdin = open(input_filename, O_RDONLY, 0644)) < 0) {
		perror(input_filename);	/* open failed */
		return 0; //fail
	}
	
	dup2(fdin, STDIN_FILENO);
	close(fdin);
	return 1;
}

int main(void)
{
	char history_cmd[hist_line][hist_strlen]; //store list of command
	char *args[MAX_LINE/2 + 1]; 		// command line arguments
	char *argsPipe[MAX_LINE/2+1];		// command line arguments for pipe
	char cmdLine[cmd_lenght];
	int should_run = 1; 			// flag to determine when to exit program 
	int execFlag = 0;
	char input_filename[filename_lenght];
	char output_filename[filename_lenght];

	init_shell();

	while (should_run) {
		printf("osh>");
		fflush(stdout);

		if(!get_Input(cmdLine, history_cmd))
			continue;

		execFlag = process_cmdLine(cmdLine, args, argsPipe, history_cmd, input_filename, output_filename);
		if(execFlag == 1){
			execArgsNoPipe(args);
			if(strcmp(redir_symbol, ">") == 0)
			{
				dup2(saved_stdout, STDOUT_FILENO);
				close(saved_stdout);
			}
			else if(strcmp(redir_symbol, "<") == 0)
			{
				dup2(saved_stdin, STDIN_FILENO);
				close(saved_stdin);
			}
		}
		if(execFlag == 2){
			execArgsPipe(args,argsPipe);
		}
	}
	
	return 0;

}
