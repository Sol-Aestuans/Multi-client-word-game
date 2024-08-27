/* client.c - code for client program. Do not rename this file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <time.h>

#include "proj.h"
#define MAX_WORD_LEN 254

#define WAIT 0
#define TURN 1
#define ROUND_END 2
#define GAME_END 3

void printRound(uint8_t roundNum, uint8_t p1Wins, uint8_t p2Wins, char* board, uint8_t boardLen) {
    PRINT_wARG("\nRound %d...\nScore is %d-%d\n", roundNum, p1Wins, p2Wins);
    PRINT_MSG("Board: ");
    for (int i = 0; i < boardLen; i++) {
        PRINT_wARG("%c ", board[i]);
    }
    PRINT_MSG("\n");
}

void endGame(uint8_t playerWins, uint8_t opponentWins, int socket_fd, uint8_t p1Wins, uint8_t p2Wins) {
    // display winner, if there is one, game score otherwise
    if (playerWins == 3) {
        PRINT_MSG("You won!\n");
    } else if(opponentWins == 3) {
        PRINT_MSG("You lost!\n");
    } else {
        PRINT_wARG("Game ended prematurely. Final score: %d-%d", p1Wins, p2Wins);
    }

    close(socket_fd);
    exit(0);
}

int main( int argc, char **argv) {

    if (argc != 3) {
        PRINT_MSG("Usage: ./client {server address} {server port}\n");
    }
    
    int port;
    if (sscanf(argv[2], "%d", &port)<=0) {
        DEBUG_MSG("invalid argmuemnts");
        exit(EXIT_FAILURE);
    }
    int socket_fd;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))<0) {
        DEBUG_MSG("Failed to create socket_fd");
        exit(-1);
    }
    // establish server address
    struct sockaddr_in addr;

    // for timeout
    fd_set timer_fd_set;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // convert the IP to sin_addr format
    if ((inet_pton(AF_INET, argv[1], &addr.sin_addr))<0) {
        DEBUG_MSG("Address Failure");
        exit(EXIT_FAILURE);
    }

    // connect to server
    if (connect(socket_fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        DEBUG_MSG("Connection Failed");
        exit(EXIT_FAILURE);
    }

    uint8_t playerID, turnTime, boardLen, roundStatus, gameStatus = 0;
    char turnStatus;

    //Get Player ID
    read(socket_fd, &playerID, sizeof(playerID));
    //Get Word Length
    read(socket_fd, &boardLen, sizeof(boardLen));
    //Get Turn Time
    read(socket_fd, &turnTime, sizeof(turnTime));
    char* board = malloc((sizeof(char)*boardLen)+1);
    char word[MAX_WORD_LEN];

    // Get timeout val
    struct timeval timeout_s = { // setting the turn timer
        .tv_sec = turnTime
    };
    int socket_timer;


    if (playerID == 1) {
        PRINT_MSG("You are Player 1... the game will begin when Player 2 joins...\n");
    } else {
        PRINT_MSG("You are Player 2...\n");
    }

    //GameLoop
    uint8_t roundNum = 1, p1Wins = 0, p2Wins = 0; // init game variables
    while (gameStatus == 0) {  // game
        // update game variables
        read(socket_fd, &p1Wins, sizeof(uint8_t));
        read(socket_fd, &p2Wins, sizeof(uint8_t));
        read(socket_fd, &roundNum, sizeof(uint8_t));
        read(socket_fd, board, boardLen * sizeof(char));
        
        // check for game over
        if (p1Wins >= 3 || p2Wins >= 3) {
            free(board);
            if (playerID == 1) {
                endGame(p1Wins, p2Wins, socket_fd, p1Wins, p2Wins);
            } else {
                endGame(p2Wins, p1Wins, socket_fd, p1Wins, p2Wins);
            }
        }

        // print round info
        printRound(roundNum, p1Wins, p2Wins, board, boardLen);

        roundStatus = 1;
        char* word_buf; // holds word to display to non-active player
        while (roundStatus == 1) {  // round loop
            uint8_t read_buf = 0; // used for reading word validity/wordsize during turn
            
            // retrieve turn status
            read(socket_fd, &turnStatus, sizeof(char));
            if(turnStatus == 'Y'){                          // this player's turn
                PRINT_MSG("Your turn!, enter word: ");
                if(send(socket_fd, NULL, 0, MSG_NOSIGNAL) ==-1) {
                    exit(-1);
                }
                FD_ZERO(&timer_fd_set);
                FD_SET(0, &timer_fd_set);
                socket_timer = select(1, &timer_fd_set, NULL, NULL, &timeout_s); // starting turn timer
                timeout_s.tv_sec = turnTime; // resetting timer
                int more;
                if (socket_timer == -1) { // error
                    PRINT_MSG("socket select error");
                    exit(EXIT_FAILURE);

                } else if (socket_timer == 0) { // timeout occurs
                    DEBUG_MSG("\t\tTIMEOUT OCCURED");
                } else {
                    read_stdin(word, MAX_WORD_LEN, &more);
                    //PRINT_MSG("\n");

                    int word_len = strlen(word) - 1;
                    write(socket_fd, (uint8_t*)&word_len, sizeof(uint8_t)); // send word len
                    write(socket_fd, word, strlen(word)-1); // send word
                }
                read(socket_fd, &read_buf, sizeof(uint8_t));

                if (read_buf == 1) {
                    PRINT_MSG("Valid word!\n");
                } else if(read_buf == 0) {
                    PRINT_MSG("Invalid word!\n");
                    roundStatus = 0;
                }
            } else if (turnStatus == 'N') {                   // other player's turn
                PRINT_MSG("Please wait for opponent to enter word...\n");
                read(socket_fd, &read_buf, sizeof(uint8_t)); 

                if (read_buf > 0) {   // this means word length was received
                    word_buf = malloc(sizeof(char) * read_buf);
                    read(socket_fd, word_buf, sizeof(char)*read_buf);
                    word_buf[read_buf] = '\0';
                    PRINT_wARG("Opponent entered \"%s\"\n", word_buf);
                    free(word_buf);
                } else {       // this means round is over
                    PRINT_MSG("Opponent lost the round!\n");
                    roundStatus = 0;
                }
            } else {
                DEBUG_MSG("Unexpected Turn status received");
            }
        }        
    }
    free(board);
}
