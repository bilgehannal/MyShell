#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>

/* This shell is completely created by Bilgehan Nal and Yusuf Kamil Ak.
You can find details of implementation in Project Documentation
which is named P2.pdf
Bilgehan Nal - 150114038      Yusuf Kamil AK - 150116827  */

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_POSSIBLE_CHAR_SIZE 128
#define MAX_POSSIBLE_DIR_SIZE 2048
#define OUTPUT_REDIRECTION 100
#define NO_REDIRECTION -100
#define OUTPUT_APPEND 101
#define INPUT_REDIRECTION 200
#define ERROR_REDIRECTION 400

/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

// MARK: Linked List Manipulation

typedef struct Bookmark {
  int id;
  char **args;
  struct Bookmark *previous;
  struct Bookmark *next;
}Bookmark;

extern char **environ;

Bookmark* executeCommand(char *args[], int *background, Bookmark *HEAD);

int getSizeOfArgs(char *args[]) {
  int counter = 0;
  while(args[counter] != NULL) {
    counter++;
  }
  return counter;
}

Bookmark* reDetermineID(Bookmark *head) {
  if(head != NULL) {
    head -> id = 0;
    Bookmark *temp = head ->next;
    while(temp != NULL) {
      temp -> id = temp->previous->id+1;
      fprintf(stderr, "%d\n", temp -> id );
      temp = temp -> next;
    }
  }
  return head;
}

Bookmark* createNode(char *args[MAX_POSSIBLE_CHAR_SIZE]) {
  Bookmark* node = (Bookmark*)malloc(sizeof(Bookmark));
  node -> id = -1;
  node -> args = malloc(MAX_POSSIBLE_CHAR_SIZE*sizeof(char*));
  int i;
  for( i = 0; i < MAX_POSSIBLE_CHAR_SIZE; i++ ) {
    if(args[i] == NULL) break;
    node->args[i] = strdup(args[i]);
  }
  //node -> args = args;
  node -> previous = NULL;
  node -> next = NULL;
  return node;
}

Bookmark* insertNode(Bookmark *head, char* args[]) {
  Bookmark *newNode = createNode(args);
  if(head != NULL) {
      Bookmark *tempNode = head;
      while(tempNode->next != NULL) {
          tempNode = tempNode -> next;
      }
      newNode -> id = (tempNode -> id) + 1;
      newNode -> previous = tempNode;
      tempNode -> next = newNode;
  } else {
      head = newNode;
      head -> id = 0;
  }
  return head;
}

Bookmark* deleteNode(struct Bookmark *head_ref, int delId)
{
  /* base case */
  if(head_ref == NULL)
    return NULL;

  /* If node to be deleted is head node */

  if(head_ref->id == delId) {
    head_ref = head_ref->next;
    return head_ref;
  }


  Bookmark *temp = head_ref;
  while(temp->id != delId) {
    temp = temp->next;
  }

  /* Change next only if node to be deleted is NOT the last node */
  if(temp->next != NULL)
    temp->next->previous = temp->previous;

  /* Change prev only if node to be deleted is NOT the first node */
  if(temp->previous != NULL)
    temp->previous->next = temp->next;

  /* Finally, free the memory occupied by del*/
  free(temp);
  return head_ref;
}

// MARK: Input Manipulation

void sigintHandler(int sig_num) {
    signal(SIGINT, sigintHandler);
    fflush(stdout);
}

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0); /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
      perror("error reading the command");
	    exit(-1);           /* terminate with error code of -1 */
    }

    for (i=0;i<length;i++) { /* examine every character in the inputBuffer */
      switch (inputBuffer[i]) {
	    case ' ':
	    case '\t' :               /* argument separators */
		  if(start != -1){
        args[ct] = &inputBuffer[start];    /* set up pointer */
		    ct++;
		  }
      inputBuffer[i] = '\0'; /* add a null char; make a C string */
		  start = -1;
		  break;
      case '\n':                 /* should be the final char examined */
		  if (start != -1 && inputBuffer[start] != '&') {
        args[ct] = &inputBuffer[start];
		    ct++;
		  }
      inputBuffer[i] = '\0';
      args[ct] = NULL; /* no more arguments to this command */
		  break;
      default:             /* some other character */
		    if (start == -1)
		      start = i;
        if (inputBuffer[i] == '&') {
		      *background  = 1;
          inputBuffer[i-1] = '\0';
		    }
	   } /* end of switch */
  }    /* end of for */
  args[ct] = NULL; /* just in case the input line was > 80 */

} /* end of setup routine */

// MARK: Implementations & Utility Functions of Custom Commands

char **commandWithArgsWithoutRedirectionAndRest(char *args[],char *newArray[]) {
  int i = 0;
  while (args[i] != NULL) {
    if (strcmp(args[i],">") == 0 || strcmp(args[i],"<") == 0
      ||strcmp(args[i],">>") == 0 || strcmp(args[i],"2>") == 0 ) {
      return newArray;
    }else {
      newArray[i] = args[i];
    }
    i++;
  }
  return newArray;
}

void printBookmark(Bookmark *node) {
  fprintf(stderr, "%d -> ", node->id );
  int i;
  for(i=0; i<getSizeOfArgs(node->args); i++) {
    fprintf(stderr, "%s ", node->args[i]);
  }
  fprintf(stderr, "\n");
}

Bookmark* callBookmark(char *args[],Bookmark *HEAD) {

  if (strcmp(args[1],"-l") == 0) {
    Bookmark *tempNode = HEAD;
    while(tempNode != NULL) {
      printBookmark(tempNode);
      tempNode = tempNode->next;
    }
  }else if (strcmp(args[1],"-i") == 0) {
    Bookmark *temp = HEAD;
    while(temp->id != atoi(args[2])) {
      temp = temp->next;
    }
    int a = 0;
    HEAD = executeCommand(temp->args, &a, HEAD);

  }else if (strcmp(args[1],"-d") == 0) {
    HEAD = deleteNode(HEAD, atoi(args[2]));
    HEAD = reDetermineID(HEAD);
  }else {
    HEAD = insertNode(HEAD, &args[1]); // Bookmark is deleted from the args array for the struct
  }
  return HEAD;
}

// MARK: Codesearch Manipulation

void callCodesearch(char *args[]) {
  char *command = malloc(sizeof(char)*MAX_POSSIBLE_DIR_SIZE);
  strcpy(command,"grep --include=\\*.{c,h}");
  if (strcmp(args[1],"-r") == 0) {
    strcat(command," -rn");
    strcat(command," './' -e '");
    strcat(command,(strcmp(args[1],"-r") == 0 ? args[2] : args[1]));
    strcat(command,"'");
  }else {
    strcat(command," -sn");
    strcat(command," \"");
    strcat(command,args[1]);
    strcat(command,"\" *.*");
  }
  fprintf(stderr, "\n");
  system(command);
  free(command);
}

void callPrint(char *args[]) {
  char *s = *environ;
  if(args[1] == NULL) {
    int i = 1;
    for (; s; i++) {
      fprintf(stderr, "%s\n", s);
      s = *(environ+i);
    }
  } else {
    int i = 1;
    for (; s; i++) {
      char temp[1024];
      strcpy(temp, s); 
      char *token = strtok(temp, "=");
      
      if(strcmp(token, args[1]) == 0) {
        token = getenv(token);
        fprintf(stderr, "%s\n", token);
      }
    s = *(environ+i);
    }
  }
}

void callSet(char *args[]) {
  char *command;
  int size = strlen(args[1])+1;
  
  if(args[2] != NULL) size += strlen(args[2]);
  if(args[3] != NULL) size += strlen(args[3]);
  command = (char *)malloc(size);

  strcpy(command, strdup(args[1]));
  if(args[2] != NULL) strcat(command, strdup(args[2]));
  if(args[3] != NULL) strcat(command, strdup(args[3]));
  args[0] = NULL;
  args[1] = NULL;
  args[2] = NULL;

  fprintf(stderr, "Come on: %s", command);
  putenv(args[1]);
}

int checkIfCustomCommand(char *commandName) {
  if (strcmp(commandName, "bookmark") == 0 ||
      strcmp(commandName, "codesearch") == 0 ||
      strcmp(commandName, "print") == 0 ||
      strcmp(commandName, "set") == 0 ||
      strcmp(commandName, "cd") == 0)
    return 1;
  return -1;
}

void callCd(char *args[]) {
  char *h="/home";   
  if(args[1]==NULL)
          chdir(h);
  else if ((strcmp(args[1], "")==0) || (strcmp(args[1], "/")==0))
          chdir(h);
  else if(chdir(args[1])<0)
      printf("bash: cd: %s: No such file or directory\n", args[1]);
}

Bookmark* applyCustomCommands(char *args[],Bookmark *HEAD) {
  if (strcmp(args[0], "bookmark") == 0) { HEAD = callBookmark(args,HEAD); }
  else if (strcmp(args[0], "codesearch") == 0) { callCodesearch(args); }
  else if (strcmp(args[0], "print") == 0) { callPrint(args); }
  else if (strcmp(args[0], "set") == 0) { callSet(args); }
  else if (strcmp(args[0], "cd") == 0) { callCd(args); }
  return HEAD;
}

// MARK: Application Flow Manipulation

int containsRedirection(char *args[],int tolerance) {
  int i = 0,toleranceCounter = 0;
  while (args[i] != NULL) {
    if (strcmp(args[i],">") == 0) {
      if (tolerance == toleranceCounter) {
        return OUTPUT_REDIRECTION;
      }else {
        toleranceCounter++;
      }
    }
    else if (strcmp(args[i],">>") == 0) {
      if (tolerance == toleranceCounter) {
        return OUTPUT_APPEND;
      }else {
        toleranceCounter++;
      }
    }
    else if (strcmp(args[i],"<") == 0) {
      if (tolerance == toleranceCounter) {
        return INPUT_REDIRECTION;
      }else {
        toleranceCounter++;
      }
    }
    else if (strcmp(args[i],"2>") == 0) {
      if (tolerance == toleranceCounter) {
        return ERROR_REDIRECTION;
      }else {
        toleranceCounter++;
      }
    }
    i++;
  }
  return NO_REDIRECTION;
}

void callExit(char *args[]) {
  //wait(NULL);
  fprintf(stderr, "MyShell is terminated \n");
  while(1)
    wait(NULL);
    exit(0);
}

Bookmark* executeCommand(char *args[], int *background, Bookmark *HEAD) {
  
  pid_t pid = fork();
  if (pid < 0) { /* If fork operation fails */
    /* handle error */
    fprintf(stderr, "Something's wrong about creating a new process.\n");
  }
  if (pid == 0) { /* If process is child */
    /* handle child process */

    char path1[MAX_POSSIBLE_CHAR_SIZE];
    char path2[MAX_POSSIBLE_CHAR_SIZE];
    char path3[MAX_POSSIBLE_CHAR_SIZE];
    strcpy(path1,"/bin/"); strcpy(path2,"/usr/bin/"); strcpy(path3,"/usr/local/bin/");
    strcat(path1,args[0]); strcat(path2,args[0]); strcat(path3,args[0]);
    if( strcmp(args[0], "cd") == 0) {
      callCd(args);
    } else {
      if (execv(path1, args) == -1 && execv(path2, args) == -1 && execv(path3, args) == -1) {
        if (checkIfCustomCommand(args[0]) == -1) {
          fprintf(stderr, "Invalid command. Please be sure you inserted a right command.\n");
        }else {
          HEAD = applyCustomCommands(args,HEAD);
        }
      }
    }
    
  }else { /* If process is parent */
    if (*background == 0) {
      /* parent process shall wait */
      wait(NULL);
    }
  }


  return HEAD;
}

Bookmark* executeOutputRedirection(char *args[],int *background,Bookmark *HEAD) {
  int output_fd;
  int i = 1;
  while (args[i] != NULL) {
    if (strcmp(args[i-1],">") == 0) {
      output_fd = creat(args[i],0644);
      dup2(output_fd, 1);
    }
    if (strcmp(args[i-1],">>") == 0) {
      output_fd = open(args[i],O_WRONLY|O_APPEND,0);
      dup2(output_fd, 1);
    }
    if (strcmp(args[i-1],"2>") == 0) {
      output_fd=open(args[i],O_WRONLY, 0);
      dup2(output_fd, 2);
    }
    i++;
  }
  char *newArray[MAX_LINE/2 + 1];
  close(output_fd);
  HEAD = executeCommand(commandWithArgsWithoutRedirectionAndRest(args,newArray),background,HEAD);
  return HEAD;
}

Bookmark* executeInputRedirection(char *args[],int *background,Bookmark *HEAD) {
  int input_fd,length;
  int i = 1;
  char *inputBuffer = malloc(sizeof(char)*MAX_LINE);
  while (args[i] != NULL) {
    if (strcmp(args[i-1],"<") == 0) {
      input_fd=open(args[i],O_RDONLY, 0);
      if (input_fd < 0) {
        fprintf(stderr, "Failed to open %s for reading\n", args[i]);
        return HEAD;
      }
      length = read(input_fd,inputBuffer,MAX_LINE);
      args[i-1] = inputBuffer;
      // Assuming the input file will not include multiple lines.
    }
    i++;
  }
  char *newInputArray[MAX_LINE/2 + 1];
  dup2(input_fd, 0);
  close(input_fd);
  HEAD = executeCommand(commandWithArgsWithoutRedirectionAndRest(args,newInputArray),background,HEAD);
  return HEAD;
}

Bookmark *checkRedirectionOfCommands(char *args[],int *background, Bookmark *HEAD) {
  switch (containsRedirection(args,0)) {
    case NO_REDIRECTION:
      HEAD = executeCommand(args,background,HEAD);
      break;
    case OUTPUT_REDIRECTION:
      HEAD = executeOutputRedirection(args,background,HEAD);
      break;
    case INPUT_REDIRECTION:
      HEAD = executeInputRedirection(args,background,HEAD);
      break;
    case OUTPUT_APPEND:
      HEAD = executeOutputRedirection(args,background,HEAD);
    case ERROR_REDIRECTION:
      HEAD = executeOutputRedirection(args,background,HEAD);
    default: break;
  }
  return HEAD;
}

int main(void) {
  char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
  int background; /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE/2 + 1]; /*command line arguments */
  char *cwd;
  Bookmark *HEAD = NULL;
  signal(SIGINT,sigintHandler);
  while(1) {
    background = 0;
    cwd = getcwd(cwd,MAX_POSSIBLE_DIR_SIZE);
    fprintf(stderr,"myshell:%s$",cwd);
    /*setup() calls exit() when Control-D is entered */
    setup(inputBuffer, args, &background);
    if (args[0] != NULL && strcmp(args[0], "exit") == 0) {
      callExit(args); 
    } 
    HEAD = checkRedirectionOfCommands(args, &background,HEAD);
  
    
    }
}
