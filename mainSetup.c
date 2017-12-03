#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

/* This shell is completely created by Bilgehan Nal and Yusuf Kamil Ak.
You can find details of implementation in Project Documentation
which is named P2.pdf
Bilgehan Nal - 150114038      Yusuf Kamil AK - 150116827  */

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_POSSIBLE_CHAR_SIZE 128
#define MAX_POSSIBLE_DIRECTION_SIZE 2048

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

Bookmark* setJob(char *args[], int *background, Bookmark *HEAD);

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
    HEAD = setJob(temp->args, &a, HEAD);

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
  char *command;
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
}

void callPrint(char *args[]) {

}

void callSet(char *args[]) {

}

int checkIfCustomCommand(char *commandName) {
  if (strcmp(commandName, "bookmark") == 0 ||
      strcmp(commandName, "codesearch") == 0 ||
      strcmp(commandName, "print") == 0 ||
      strcmp(commandName, "set") == 0 ||
      strcmp(commandName, "exit") == 0)
    return 1;
  return -1;
}

Bookmark* applyCustomCommands(char *args[],Bookmark *HEAD) {
  if (strcmp(args[0], "bookmark") == 0) { HEAD = callBookmark(args,HEAD); }
  else if (strcmp(args[0], "codesearch") == 0) { callCodesearch(args); }
  else if (strcmp(args[0], "print") == 0) { callPrint(args); }
  else if (strcmp(args[0], "set") == 0) { callSet(args); }
  return HEAD;
}

// MARK: Application Flow Manipulation

Bookmark* setJob(char *args[], int *background, Bookmark *HEAD) {
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
    if (execv(path1, args) == -1 && execv(path2, args) == -1 && execv(path3, args) == -1) {
      if (checkIfCustomCommand(args[0]) == -1) {
        fprintf(stderr, "Invalid command. Please be sure you inserted a right command.\n");
      }else {
        HEAD = applyCustomCommands(args,HEAD);
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

int main(void) {

    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */
    Bookmark *HEAD = NULL;
    while(1) {
      background = 0;
      fprintf(stderr,"myshell: ");
      /*setup() calls exit() when Control-D is entered */
      setup(inputBuffer, args, &background);

      HEAD = setJob(args, &background,HEAD);
    }
}
