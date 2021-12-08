#include <stdio.h>
#include <unistd.h>
#include<iostream>
#include<string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 // The maximum length command 
#define HISTORY_LINE 100 // The maximum length for history command

//global variables
bool checkAmp = false; 
int counter;

// prase function
void parseline(char cmd[], char * args[])
{
  int i = 0;
  //tokenize commands
  char *tokens = strtok(cmd, " ");

  //read tokenized commands to tokens
  while(tokens != NULL)
    {
      args[i] = tokens; //read first parsed cmd to args
      tokens = strtok(NULL, " "); //empty tokens for new token
      i++; //next
    }
  counter= i;//global counter for amount of tokens
  args[i] = NULL;//set ars[i] to null end char array

}

// !! and & check function
void checkCmd(char cmd[], char *args[], char previouscmd[])
{

  if(cmd[strlen(cmd) - 1] == '&')
    {
      checkAmp = true;//flag ampersand is here
      cmd[strlen(cmd)-1] = '\0';//remove ampersand set it to null to end string and perform actual command
    }
  else
    {
      checkAmp = false;//no ampersand exists
    }

  //command will parse into several command arguments
  if(strcmp(cmd,"!!") == 0)
    {
      if(strcmp(previouscmd,"") == 0)
        {
	  std::cerr<<"No commands in history."<<std::endl;
        }
      else
        {
          //std::cout<<"prevcmd: "<<previouscmd<<std::endl;
          parseline(previouscmd,args);//parse previous cmd and args
        }
    }
  else//enter if
    {
      strcpy(previouscmd, cmd);//copy current cmd to be previouscmd
      parseline(cmd,args);//pass cmd and args
    }
}

// exit function 
void stopShell(char cmd[],int run)
{

  //compare comd input to see if exit match
  if(strcmp(cmd, "exit") == 0)
    {
      std::cout<<"Shell stops."<<std::endl;
      run = 0;//main loop flag set to 0
      exit(1);//exit
    }
}


int main(void)
{
  std::cout << "CS 433 Programming assignment 2" << std::endl;
  std::cout << "Author: Eric Lozano and Isaac Ferrel and Ting Wei Chien(Willy)" << std::endl;
  std::cout << "Date: 10/21/2021" << std::endl;
  std::cout << "Course: CS433 (Operating Systems)" << std::endl;
  std::cout << "Description : A simple Unix shell" << std::endl;
  std::cout << "=================================" << std::endl;

  char *args[MAX_LINE/2 + 1]; /* command line arguments */
  int should_run = 1; /* flag to determine when to exit program */
  char command[MAX_LINE]; // to store user command
  char previouscmd[HISTORY_LINE];//to store the previous command
  int store; // for store execvp result

  while (should_run){
      
    printf("osh>");
    fflush(stdout);

    //take user input cin
    std::cin.getline(command,MAX_LINE);
   
    //checker founction for !! and & cmd 
    checkCmd(command,args,previouscmd);

    //stop function for exit cmd
    stopShell(command,should_run);
   
    //------------------------------------------//
    //-----------------fork part----------------//
    //------------------------------------------//


    //step(1) fork a child process using fork()
    pid_t pid = fork();

    //Error case
    if(pid < 0)
      {
	std::cerr<<"fork failed!"<<std::endl;
	should_run = 0;
        exit(0);
      }

    //step(2) the child process will invoke execvp()
    else if(pid==0)
      {

        int filein, fileout; //store file infomation

	//loop for checking args
        for(int i = 0; args[i] != NULL; i++)
          {
	    //input from file case
            //comparing 3 total characters
            if(strncmp(args[i],"<",3) == 0)
              {
		//print file name
		std::cout<<"file: "<<args[counter - 1]<<std::endl;

                filein = open(args[counter - 1],O_RDONLY);//redirtect

                if(filein == -1)// file doesn't exist
                  {
		    std::cout<<"File does not exist file or cmd"<<std::endl;;
                    args[0] = NULL;//empty command space for next command
                    exit(0);//terminate
		  }

                dup2(filein,STDIN_FILENO);
		args[i] = NULL;//we set from character < and up to null to execute command
		break;

              }

	    //output from file case
            else if(strncmp(args[i],">",3) == 0)
              {
		std::cout<<"file: "<<args[counter - 1]<<std::endl;

                fileout = open(args[counter - 1], O_CREAT | O_RDWR);//redirtect

		if(filein == -1)//file doesn't exist
                  {
		    std::cout<<"File does not exist file or cmd"<<std::endl;;
                    args[0] = NULL;//empty command space for next command
                    exit(0);//terminate
                  }

                dup2(STDOUT_FILENO,fileout);
                args[i] = NULL;//we set from character > and up to null to execute command
                break;
              }
          }
    
        store = execvp(args[0],args);//exec command

	// cmd not found case
        if(store == -1)
          {
	    std::cout<<"command not found: "<<previouscmd<<std::endl;
          }

      } 

    //step(3) parent will invoke wait() unless command included &
    else//check for ampersand
      {
	// if command doesn't include &
        if(checkAmp == false)
          {
            wait(NULL);//parent waits for child
          }
        else
          {
	    std::cout<<"keep running."<<std::endl;
          }
      }

  }//end of while loop
}//program end

