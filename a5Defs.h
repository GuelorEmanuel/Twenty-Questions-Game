#define SERVER_PORT  60002

/* global variable for passing around sighandling values */
int sig;


/* Functions */
void becomeGuesser(char *);
void becomeOracle();
void playGuesser(int);
void playOracle(int);
void sigHandler(int);
void winGame(int);
void loseGame(int);



