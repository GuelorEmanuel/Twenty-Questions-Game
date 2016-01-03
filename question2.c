#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "a5Defs.h"
#include <signal.h>


void becomeOracle() {
  int myListenSocket, clientSocket;
  struct sockaddr_in  myAddr, clientAddr;
  int i, addrSize;

  char buffer[100];
  char currIP[100];

  /* create socket */
  myListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (myListenSocket < 0) {
    printf("eek! couldn't open socket\n");
    exit(-1);
  }
  else printf("Socket has been opened\n");


  /* setup my server address */
  memset(&myAddr, 0, sizeof(myAddr));
  myAddr.sin_family = AF_INET;
  myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myAddr.sin_port = htons((unsigned short) SERVER_PORT);

  /* bind my listen socket */
  i = bind(myListenSocket, (struct sockaddr *) &myAddr, sizeof(myAddr));
  if (i < 0) {
    printf("eek! couldn't bind socket\n");
    exit(-1);
  }
  else printf("Socket has been blinded\n");  /* ....Socket has been created and bounded...*/


  /* listen */
  i = listen(myListenSocket, 5);
  if (i < 0) {
    printf("eek! couldn't listen\n");
    exit(-1);
  }
  else printf("I am listening!\n");


  /* wait for connection request */
  addrSize = sizeof(clientAddr);
  
  printf("Waiting for connection...\n");

  clientSocket = accept(myListenSocket, (struct sockaddr *) &clientAddr, &addrSize);
  
  if (clientSocket < 0) {
    printf("eek! couldn't accept the connection\n");
    exit(-1);
  }
  printf("Connection established...!\n");

  playOracle(clientSocket);

  /* close sockets */
  close(myListenSocket);
  close(clientSocket);
  printf("hello"); //test
  return;
}

void becomeGuesser(char *server_ip) {
  int mySocket, i;
  struct sockaddr_in  addr;

  /* create socket */
  mySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (mySocket < 0) {
    printf("eek! couldn't open socket\n");
    exit(-1);
  }


  /* setup address */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(server_ip);
  addr.sin_port = htons((unsigned short) SERVER_PORT);

  /* connect to Oracle */
  i = connect(mySocket, (struct sockaddr *) &addr, sizeof(addr));
  if (i<0) {
    printf("client could not connect!\n");
    exit(-1);
  }
  
  playGuesser(mySocket);
  
  /* close the socket */ 
  close(mySocket);
  return;
}

/* Handle the evet when a player wins a Game
 *
 */
void winGame(int mySocket) {
  int winResponse;
  char decision[10];
  
  printf("\n\nCongratulations, YOU WON!\n");
  printf("  Would you like to play again?\n        1 - YES   2 - NO\n");
  scanf("%d", &winResponse);
  
  if (winResponse == 1) { 
    strcpy(decision, "yes");
    send(mySocket, decision, strlen(decision), 0);
    playOracle(mySocket); 
  } 
  else { return; }
}

/* Handle the events when the a person loses a Game.
 *
 *
 */
void loseGame(int mySocket) {
  int bytesRcv;
  char decision[10];
  
  printf("\n\nSorry! You lost :c, but better luck next round.\n");
  printf("  Waiting for winner's decision...\n");
  
  while (1) {
    bytesRcv = recv(mySocket, decision, sizeof(decision), MSG_DONTWAIT);
    if (bytesRcv != -1) {
		  decision[bytesRcv] = 0;
		  printf("\nThis is what the Winner responded:  %s\n", decision);

		  if(strcmp(decision, "yes") == 0) { 
		    printf("You are the Guesser\n");
        playGuesser(mySocket);
		  } else {
		    becomeOracle();
		  }
    }
  }
  
  return;
}


/* Let handle the SIGINT (Ctrl-C) signal indicates a yes answer to the question
 * SIGSTP (CTRL-Z) signal indicates a no answer to the question
 * SIGQUIT (Ctrl-\) signal is a win answer, meaning the guesser has corretly guessed the object
 */

void sigHandler(int x) {
  if (x == SIGINT) {
    printf("\n\nYes\n");
    sig = SIGINT;
  } else if (x == SIGTSTP) {
    printf("\n\nNo\n");
    sig = SIGTSTP;
  } else if (x == SIGQUIT) {
    sig = SIGQUIT;
    printf("\n\nWin\n");
  }

  return;
}


void playOracle(int clientSocket) {
  int     bytesRcv;
  int    count = 0;
  char  answer[10];
  char buffer[100];
  
  /* read message from Guesser */
  printf("Press CTRL+C to begin game\n");

  while(1){
  signal(SIGINT,  sigHandler);
  signal(SIGTSTP, sigHandler);
  signal(SIGQUIT, sigHandler);
  
  bytesRcv = recv(clientSocket, buffer, sizeof(buffer), MSG_DONTWAIT);
  if (bytesRcv != -1) {
    buffer[bytesRcv] = 0;
    printf("This is what the guesser asked:  %s\n", buffer);
    ++count;
    if (count >= 20) {
      winGame(clientSocket);
      break;
    }
    printf("CTRL+C for 'yes', CTRL+Z for 'no', CTRL+'backslash' for 'win'\n");
    if (strcmp(buffer,"quit") == 0 || strcmp(buffer,"") == 0) {
      break; 
    }


  }
  /* the "..." shows players that the game is running, not blocked */
  printf("|");
  fflush(stdout);
  usleep(500000);

  /* send answer to Guesser */
  if (sig == SIGINT) {
    strcpy(answer, "Yes");
    send(clientSocket, answer, strlen(answer), 0);
  } else if (sig == SIGTSTP) {
      strcpy(answer, "No");
      send(clientSocket, answer, strlen(answer), 0);
    } else if (sig == SIGQUIT) {
        strcpy(answer, "Win");
        send(clientSocket, answer, strlen(answer), 0);
        loseGame(clientSocket);
    }
    sig = 0;
  } 
  return;
}


void playGuesser(int mySocket) {
  char     buffer[100];
  char   question[100];
  int    j, bytesRcv;
  
  /* get input from Guesser and send to Oracle */
  printf("\nAt any point, enter 'quit' to forfeit the game\n\n");
  j = 0;
  
  /* read message from Oracle */
  while (1) {
    bytesRcv = recv(mySocket, buffer, sizeof(buffer), MSG_DONTWAIT);
    if (bytesRcv != -1) {
      buffer[bytesRcv] = 0;
      printf("\nThis is what the Oracle responded:  %s\n", buffer);
      ++j;

      if ((j > 20) || (strcmp(question, "quit") == 0)) { 
        loseGame(mySocket);
	break;
      }
      if(strcmp(buffer, "Win") == 0) { 
        winGame(mySocket);
        break;
      }
      printf("Question #%d (enter question to send to Oracle): ", j);
      gets(question);
      strcpy(buffer, question);
      send(mySocket, buffer, strlen(buffer), 0);
      printf("Waiting for the Oracle");
    }
    /* the "..." shows players that the game is running, not blocked */
    printf(".");
    fflush(stdout);
    usleep(500000);
  }  
  return;
}


int main(int argc, char *argv[]) {
  if (argc > 1) {
    /* read IP address from command line */
    char server_ip[100];
    strncpy(server_ip, argv[1], 100);
    becomeGuesser(server_ip);
  } else {
    /* wait for a connection */
    becomeOracle();
  }
  return;
}
