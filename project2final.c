/*
	Created By Oniel Toledo
	On 09/07/2014
	CS 370 Project 2: a simple UNIX shell

	Final Submission (Complete All Parts)
	1. Wait prompt for user input DONE
	2. Accept commands from the user DONE
	3. Accept change directory DONE
	4. Keep history of last 10 commands DONE
	5. Handle the pipe operator *****************************************
	6. Accept an exit command with comfirmation DONE
	7. Ignore L/R arrows DONE
	8. Handle deletion/backspace DONE

	Note:
	The program is capable of completely handling L/R Arrows but has been commented out.
*/

//preprocessor directive
#define _OPEN_SYS
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

typedef int bool;
#define true 1
#define false 0
void handlePipe(char** ,char**);


int main() {
	char bChar, bChar1, bChar2;			//user input variables
	char buffer[256];

	char  histFlag;		//history variables
	char* history[10];
	int iHist=0;		//track the current number of history items
	int currHist=0;		//used for tracking number of up/down presses
	bool historyOnScreen = false;
	char *test;

	char currDir[256];	//path variables

  	char* token[100];	//tokenizing variables
	int tokenIndex = 0;

	int i=0;				//general array index iteration
	int x;

	bool isPipe = false;
	int pipeLocation=0;


	//disable currPosition everywhere only used when Left/Right Arrows are working
	//int currPosition=0;
	int pid;
	int status;

	const char* exit = "exit";
	const char* pipe = "|";
	//----------------------------------------------TERMIOS INPUT CONFIGURATION
	//tcgetattr - retrieve and store current termios config
	//tcsetattr - set a new config to be used for IO
	struct termios origConfig;
	tcgetattr(0, &origConfig);
	struct termios newConfig = origConfig; 
	newConfig.c_lflag &= ~(ICANON|ECHO);  
	newConfig.c_cc[VMIN] = 10; 
	newConfig.c_cc[VTIME] = 1;
	tcsetattr(0, TCSANOW, &newConfig);//start now

	//-----------------------------------------------------------------------OUTPUT PATH
	if(getcwd(currDir, sizeof(currDir)) == NULL) perror("error at getcwd()");
	else printf("%s->", currDir);

	//TODO change for final submission
	//while(strcmp(buffer, exit) != 0){
	//loop until program is interrupted incorrectly
	while(1){
		//-------------------------------------------------------------------GET USER INPUT
		histFlag = 'n'; //disable flag set 
		while(1){
			bChar = getchar();
			if(bChar == '\n'){
				//handle line feed
				if(i!=0){
					printf("\n");
					buffer[i] = '\0';
					break;
				}
			} else if (bChar == 8 || bChar == 127){
				//if(i>0 /*&& i==currPosition*/){ //if at cursor is at the end
				if(i>0){ //if at cursor is at the end
					//handle backspace
					printf("\b \b");
					//currPosition--;
					i--;
					buffer[i] = '\0'; //deleted location to null
				} 
				// else if (currPosition>0 && currPosition<i){
				// 	//shift on the terminal screen
				// 	printf("\b");
				// 	for(x=currPosition; x < i; x++){
				// 		printf("%c", buffer[x]);
				// 	}
				// 	for(x=0; x<i; x++){
				// 		printf(" \b");	
				// 	}
				// 	printf(" \b");
				// 	for(x=i;x>currPosition; x--){
				// 		printf("\b");
				// 	}
				// 	//now shift the actual variable
				// 	for(x=currPosition; x < i; x++){
				// 		buffer[x-1] = buffer[x];
				// 	}
				// 	currPosition--;
				// 	i--;
				// 	buffer[i] = '\0'; //deleted location to null
				// }
			} else if(bChar == 27){
				bChar1 = getchar();
				if(bChar1 == 91){
					bChar2 = getchar();
					if(bChar2 == 65){ //UP
						histFlag = 'u';
						if(currHist>=0){
							
							if(historyOnScreen==true && currHist>0){
								for(x=0; x < strlen(history[currHist]); x++){
									printf("\b \b");
								}
							historyOnScreen = false;
							}
							currHist--;
						}
						break;
					} else if (bChar2 == 66){ //DOWN
						histFlag = 'd';
						if(currHist<iHist) {
							if(historyOnScreen==true && i !=0){
								for(x=0; x < strlen(history[currHist]); x++){
									printf("\b \b");
								}
								historyOnScreen = false;
							}
							currHist++;
						}
						break;
					} 
					else if(bChar2 == 68){ //LEFT
						//if(currPosition>0){
							//printf("%c%c%c", bChar, bChar1, bChar2);
							//currPosition--;						
						//}
					} else if(bChar2 == 67){ //RIGHT
						//if(currPosition<i){
							//printf("%c%c%c", bChar, bChar1, bChar2);
							//currPosition++;
						//}
					}
					//any other special characters will fall here and be ignored
				} 
			} else {
				//if new character is requested and not at the end
				// if(currPosition<i){
				// 	//output new character and shift characters in the prompt to the right
				// 	printf("%c", bChar);
				// 	for(x=currPosition; x < i; x++)
				// 		printf("%c", buffer[x]);
				// 	//backspace back to currPosition+1
				// 	for(x=i; x > currPosition; x--)
				// 		printf("\b");

				// 	//shift characters down to right by 1 & add new character
				// 	for(x=(i+1); x > currPosition; x--)
				// 		buffer[x] = buffer[x-1];
					
				// 	buffer[currPosition] = bChar;
				// 	currPosition++;
				// 	i++;
				// } else {
					printf("%c", bChar);
					buffer[i] = bChar;
					i++;
					//currPosition=i;

				//}

			}
		}

		//---------------------------------------------------------------------ARROW KEY HANDLING
		if(histFlag == 'u'){
			if(currHist>-1){ //as long as it does not go below the first position
				historyOnScreen = true;
				strcpy(buffer, history[currHist]); //make history item be the the buffer
				i = strlen(buffer);
				//currPosition = i;
				printf("%s", buffer);
			} else {
				currHist++;
			}
		} else if(histFlag == 'd'){
			if(currHist<iHist){//as long as it does not go above the highest position entered
				historyOnScreen = true;
				strcpy(buffer, history[currHist]); //make history item be the the buffer
				i = strlen(buffer);
				//currPosition = i;
				printf("%s", buffer);
			} else  {
				currHist--;
			}
		//---------------------------------------------------------------------EXECUTE COMMAND
		} else if(histFlag == 'n'){
			//store inputted buffer command into history
			if(iHist>9){ //history array is already full make room
				for(i=0; i<9; i++) {
					history[i] = (char *)malloc(strlen(history[i+1]) * sizeof(char));
					strcpy(history[i], history[i+1]);
				}
				history[9] = (char *) malloc(strlen(buffer) * sizeof(char));
				strcpy(history[9], buffer);
			} else {
				history[iHist] = (char *) malloc(256 * sizeof(char)); //allocat memory for buffer 
				strcpy(history[iHist], buffer); //save to history
				iHist++;
			}
			//non Up/Down arrow has been pressed reset currHistort
			currHist = iHist;
			historyOnScreen = false;

			//-------------------------------------------------------------PARSE INPUT BY WHITESPACE
	   		token[tokenIndex] = strtok(buffer, " \0");
	   	 	while( token[tokenIndex] != NULL ){
				tokenIndex++; 
				token[tokenIndex] = strtok(NULL, " \0");
	   		}
	   		tokenIndex++; 
	   		token[tokenIndex] = NULL; //null terminate the string

			//create new variable and allocate memory based on # of tokens
			char *command[tokenIndex]; //alternative to dynamic allocation
			for(i=0; i<tokenIndex; i++){
				command[i] = token[i];
			}

			//----------------------------------------------------------------DIRECTORY CHANGES
			if(strcmp(token[0], "cd") == 0){
				if(chdir(token[1]) != 0)
					perror("Error in directory");

			//----------------------------------------------------------------EXIT HANDLING
			} else if(strcmp(token[0], exit) == 0){
				printf("Are you sure you want to exit [y/n]? "); //exit confirmation prompt
				bChar = getchar();
				printf("%c\n", bChar);
				if(bChar == 'y'){
					//restore termios configs and terminate
					tcsetattr(0, TCSANOW, &origConfig);
					return 0;
				} else if (bChar == 'n') {
					//do nothing
				}

			//----------------------------------------------------------------EXECUTE COMMAND
			} else {
				//look for a pipe operator
				for(x=0; x < (tokenIndex-1); x++){
					if(strcmp(command[x], pipe) == 0){
						isPipe = true;
						pipeLocation = x;
						break;
					}
				}

				if(isPipe == true) {
					//handle pipe command
					//get first pipe command
					char *pipeCommand1[pipeLocation];
					for(x=0; x < pipeLocation; x++){
						pipeCommand1[x] = command[x];
					}
					
					//get second pipe command
					char *pipeCommand2[((tokenIndex-1)-pipeLocation)];

					for(x=0; x < ((tokenIndex-1)-pipeLocation); x++){
						pipeCommand2[x] = command[((pipeLocation+1)+x)];
					}
				
					//attempt to execute the pipe command...
					handlePipe(pipeCommand1, pipeCommand2);

				} else if (isPipe == false){
					//create child process and attempt to execute command
					pid = fork();
					if(pid > 0){
						//handle parent process, wait for child to finish first
						waitpid(pid, &status, WUNTRACED);
					} else if (pid == 0) {
						//attempt to execute process
						//if single command is 1 string
						if(execvp(command[0], command) < 0){
							printf("Error the command entered is not a valid command\n");
							_exit(SIGCHLD);//terminate the child process
						}
					} else {
					  printf("An error occurred while created child process\n");			
					}
				} //end of isPipe ==false

			} //end of else for command

			//-----------------------------------------------------------------------OUTPUT PATH
			if(getcwd(currDir, sizeof(currDir)) == NULL) perror("error at getcwd()");
			else printf("%s->", currDir);

			//reset variables
			i=0;
			isPipe = false;
			//currPosition=0;
		}

		
		//reset variables
		tokenIndex=0;

	}//end while

	for(i=0; i<10; i++){
		free(history[i]);
	}

	tcsetattr(0, TCSANOW, &origConfig);
	return 0;
}

void handlePipe(char** pipeCommand1,char** pipeCommand2){
    int fd[2];
    int fd2[2];
    int bytesRead =0;
    int pid,pid2;
    int status;
    //create pipe and run first child first
    pipe(fd);
    pid = fork();
    if(!pid){
        /*Close reading end of pipe*/
        close(fd[0]);
        close(1);
        dup(fd[1]);
        execvp(pipeCommand1[0],pipeCommand1);
        perror("Command does not exist");
        exit(1);
    }
    //wait til first child is done
    close(fd[1]);
    waitpid(pid, &status, WUNTRACED);
    pid2= fork();
    //run the second child
    if(!pid2){
        close(fd[1]);
        dup2(fd[0],0);
        close(fd[0]);
        execvp(pipeCommand2[0],pipeCommand2);
        perror("Command does not exist");
        exit(1);
    }
    waitpid(pid2,&status, WUNTRACED);
}

/*
    Overall shell process:

    configure input (terminos)
    while not exited
        print prompt
        read input (break on up, down, and \n)
        if broken on up or down
            clear the current input
                executre appropriate command to generate correspinding message
            print message
        else
            parse input into a command
            if cd command
                change the current working directory
            else if exit command
                break
            else
                execute the command
        restore the input configuration (terminos)
*/
