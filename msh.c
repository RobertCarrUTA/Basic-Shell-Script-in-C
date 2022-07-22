// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017, 2021 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 7f704d5f-9811-4b91-a918-57c1bb646b70
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

/*
    Name: Robert Carr
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

/*
    We want to split our command line up into tokens
    so we need to define what delimits our tokens.
    In this case  white space
    will separate the tokens on our command line
*/

#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255 // The maximum command-line size
#define MAX_NUM_ARGUMENTS 10 // Mav shell only supports ten arguments

int main()
{
  char * cmd_str = (char * ) malloc(MAX_COMMAND_SIZE);

  char * history[15];
  int history_index = 0;

  // This will fill out the for loop so that it can be 
  // filled with our actual command histroy later.
  for (int i = 0; i < 15; i++)
  {
    history[i] = (char * ) malloc(MAX_COMMAND_SIZE);
  }

  // These are used later after the fork to let us
  // do the listpids command
  pid_t pid_list[15];
  int pid_index = 0;

  while (1)
  {
    // Print out the msh prompt
    printf("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));

    /* Parse input */
    char * token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char * argument_ptr;

    char * working_str = strdup(cmd_str);

    // This allows for our commands to be saved to our history array
    // by saving our history to a block of memory and incrementing to
    // the next index of the array each time we go through a new command
    memset( & history[history_index], 0, MAX_COMMAND_SIZE);
    history[history_index++] = strndup(cmd_str, MAX_COMMAND_SIZE);

    // We are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char * working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while (((argument_ptr = strsep( & working_str, WHITESPACE)) != NULL) &&
      (token_count < MAX_NUM_ARGUMENTS)) {
      token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    ///////// Shell Functionality /////////

    // This allows us to hit enter and msh> will print on a new line.
    // Without it, we drown in segfaults. Segfaults, we don't enjoy those...
    if (token[0] == NULL)
    {
      continue;
    }

    // A compare between token[0] and the exit commands will allow us to execute
    // those commands.
    if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
    {
      exit(EXIT_SUCCESS);
      return 0;
    }

    int token_index = 0;
    char * arguments[7];

    // I used arrays to test token[0], but looking at it,
    // it seems not needed, could just use a compare against
    // the strings themselves in my if-else statements
    arguments[0] = (char * ) malloc(strlen("ls"));
    arguments[1] = (char * ) malloc(strlen("-l"));
    arguments[2] = (char * ) malloc(strlen("ps"));
    arguments[3] = (char * ) malloc(strlen("listpids"));
    arguments[4] = (char * ) malloc(strlen("history"));
    arguments[5] = (char * ) malloc(strlen("cd"));

    // strncpy gave truncation warnings, so I instead used strcpy.
    strcpy(arguments[0], "ls");
    strcpy(arguments[1], "-l");
    strcpy(arguments[2], "ps");
    strcpy(arguments[3], "listpids");
    strcpy(arguments[4], "history");
    strcpy(arguments[5], "cd");

    arguments[6] = NULL;

    // Here token[0] is compared against the array of arguments and if
    // it does not match, then it says command not found.
    if (strcmp(token[0], arguments[0]) != 0 && strcmp(token[0], arguments[2]) != 0 &&
      (strcmp(token[0], arguments[3]) != 0 && strcmp(token[0], arguments[4]) != 0 &&
        (strcmp(token[0], arguments[5]) != 0)))
    {
      printf("%s: Command not found.\n\n", token[0]);
    }

    // If token[0] is equal to cd, we will then execute the cd command with whatever
    // follows cd by using token[1], IF it is a supported option
    else if (strcmp(token[0], arguments[5]) == 0)
    {
      chdir(token[1]);
      continue; // There it is again? Sorry 1310 and 1320 professors...
    }

    // If token[0] is equal to history, then we show them a list of the past 15 commands.
    else if (strcmp(token[0], arguments[4]) == 0)
    {
      for (int i = 0; i < 15; i++)
      {
        // The if below blocks NULL parts of the array from being printed.
        // Then prints the parts of the array array out that are not NULL.
        // The pid_list is about the same, so I will not make this same
        // comment twice.
        if (history[i] != NULL)
	{
          printf("%d: %s", i, history[i]);
        }
      }
    }
    else
    {
      pid_t pid = fork();
      if (pid == 0)
      {
        // My listpids command is a little weird
        // it only shows one pid, then the rest are emtpy
        // The listpid code follows same reasoning as history loop.

        pid_list[pid_index++] = getpid(); // Found out that doing this before the fork with
        if (pid_index > 14) pid_index = 0; // pid_list[] = fork() would cause two programs to run
        if (strcmp(token[0], arguments[3]) == 0)
	{
          for (int i = 0; i < 15; i++)
	  {
            if (history[i] != NULL)
	    {
              printf("%d: %d\n", i, pid_list[i]);
            }
          }

          exit(EXIT_SUCCESS);
        }

        // This for loop goes through our tokens we typed in, and executes them with
        // the help of execvp(). This also handles the "invalid option, try help"
        for (token_index = 0; token_index < token_count; token_index++)
	{
          int ret = execvp(token[token_index], & token[token_index]);
          if (ret == -1)
	  {
            perror("execvp failed: ");
          }
        }
      }
      else
      {
        int status;
        wait( & status);
      }
    }
    free(working_root);
  }
  return 0;
}
