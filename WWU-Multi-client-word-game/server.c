/* server.c - code for server program. Do not rename this file */
#include <asm-generic/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "proj.h"
#include "trie.h"
#include <time.h>

#define QLEN 6 //
#define MAX_WORD_LEN 254
#define VOWEL_LEN 5


const char* g_vowels = "aeiou";

// returns true if c is a vowel
bool isVowel(char c) {
    for (int j = 0; j < VOWEL_LEN; j++) {
        if (c == g_vowels[j]) {
            return true;
        }
    }
    return false;
}

// returns true if the given set contains a vowel
bool containsVowel(char* set, int setLen) {
    DEBUG_wARG("Checking set %s\n", set);
    for (int i = 0; i < setLen; i++) {
        if(isVowel(set[i])){
            return true;
        }
    }
    return false;
}

// returns true if the given word is made of characters in the board, including frequency of the characters
bool isOnBoard(char* word, char* board) {
    char* board_copy = malloc(strlen(board) * sizeof(char)+1);
    bool valid = true;
    strcpy(board_copy, board);
    int word_len = strlen(word);
    for(int i = 0; i < word_len; i++){
        char* instance = strchr(board_copy, word[i]);
        if(instance == NULL){  // char not in array
            valid = false; 
            break;
        } else {
            ssize_t index = instance - board_copy; // fancy pointer arithmetic to find and remove the char
            board_copy[index] = '0';
        }
    }
    free(board_copy);
    return valid;
}

// checks if a word is valid by comparing it to the dictionary & usedWords tries
bool IsValid(char* word, trie_node* usedWords, trie_node* dictionary, char* board) {
    DEBUG_wARG("validating: %s\n", word);
    if ((trie_search(dictionary, word, strlen(word)))  && (isOnBoard(word, board)) && !(trie_search(usedWords, word, strlen(word)))) {
        DEBUG_wARG("Trie search: %d\n", (trie_search(dictionary, word, strlen(word))));
        DEBUG_wARG("Used Word: %d\n", (trie_search(usedWords, word, strlen(word))));
        DEBUG_wARG("onBoard: %d\n", (isOnBoard(word, board)));
        return true;
    } else {
        DEBUG_wARG("Trie search: %d\n", (trie_search(dictionary, word, strlen(word))));
        DEBUG_wARG("Used Word: %d\n", (trie_search(usedWords, word, strlen(word))));
        DEBUG_wARG("onBoard: %d\n", (isOnBoard(word, board)));
        return false;
    }
}

// generates a random lowercase letter
char genRandomChar() {
    int randInt = 97 + (26 * (rand() / (RAND_MAX + 1.0)));
    return (char) randInt;
}

// creates a new board. If there is no vowel in the generated board, the last character is set as a random vowel
void newBoard(char* board, int boardSize) {
    srand(time(NULL));

    for (int i = 0; i < boardSize; i++) {
        board[i] = genRandomChar();
    }

    if (!containsVowel(board, boardSize)) {
        char c;
        do{
            c = genRandomChar();
        } while(!isVowel(c));
        board[boardSize - 1] = c;
    }

    DEBUG_MSG("Board Generated\n");
}

// main method
int main(int argc, char **argv) {

    if (argc != 5) {
        DEBUG_MSG("Usage: ./server {port} {board size} {seconds per round} {path to word dictionary}\n");
        exit(EXIT_FAILURE);
    }

    // Player ID 1 or 2, turn time in secs, number of letters on board
    uint8_t playerID, turnTime, letterCount, p1Wins, p2Wins;
    int scan = 0;
    sscanf(argv[2],"%d", &scan);
    if (scan <= 0 || scan > MAX_WORD_LEN) { //arugment checks
        DEBUG_MSG("invalid arguments");
        exit(EXIT_FAILURE);
    }
    sscanf(argv[3],"%d", &scan);
    if (scan <= 0 || scan > MAX_WORD_LEN) {
        DEBUG_MSG("invalid arguments");
        exit(EXIT_FAILURE);
    }
    if (sscanf(argv[2], "%hhu(uint8_t)", &letterCount) <= 0) {
        DEBUG_MSG("invalid arguments");
        exit(EXIT_FAILURE);
    } 
    if (sscanf(argv[3], "%hhu(uint8_t)", &turnTime)<=0) {
        DEBUG_MSG("invalid arguments");
        exit(EXIT_FAILURE);
    }
    if (letterCount<=0 || turnTime<=0) {
        DEBUG_MSG("invalid arguments");
        exit(EXIT_FAILURE);
    }
    int port; 
    if (sscanf(argv[1], "%d", &port)<=0) {
        DEBUG_MSG("invalid arguments");
        exit(EXIT_FAILURE);
    }
    DEBUG_wARG("Server Start\nArgs:\nPort %d\nBoard %d\nTime: %d\nPath %s\n",port,letterCount,turnTime,argv[4]);

    // open dictionary file
    FILE* file = fopen(argv[4],"r");
    if (file == NULL) {
        DEBUG_MSG("Failed to open file");
        exit(EXIT_FAILURE);
    }

    DEBUG_MSG("Opened file from path\n");

    // Populate tries
    trie_node* dictionary = trie_create();
    trie_node* usedWords = trie_create();
    
    char word[MAX_WORD_LEN];

    while (fgets(word, MAX_WORD_LEN, file)) { // populating trie
        word[strlen(word)-1] = '\0';
        trie_insert(dictionary, word, (strlen(word)));
    }

    DEBUG_MSG("Populated Dictionary\n");
    
    // Socket variables
    int socket_fd, player1_fd, player2_fd, backlog = QLEN;
    struct sockaddr_in addr;
    fd_set timer_fd_set;
    socklen_t addr_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    struct timeval timeout_s = { // setting the turn timer
        .tv_sec = turnTime
    };

    // Establish Socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))<0) {
        DEBUG_MSG("Failed to create socket_fd");
        exit(EXIT_FAILURE);
    }

    // prevents 'address already in use' error

    // Bind to address
    if (bind(socket_fd, (struct sockaddr*) &addr, addr_len) == -1) {
        DEBUG_MSG("Socket bind error\n");
        exit(EXIT_FAILURE);
    }

    // Connection loop will fork and children will move on
    // while parent listens for more connections
    int child = 1;

    while(child!=0) {

        //Get First Player
        if (listen(socket_fd, backlog)) {
            DEBUG_MSG("Socket listen error.\n");
            exit(EXIT_FAILURE);
        }
        if ((player1_fd = accept(socket_fd, (struct sockaddr*) &addr, &addr_len)) < 0) {
            DEBUG_MSG("Socket accept error.\n");
            exit(EXIT_FAILURE);
        }
        DEBUG_MSG("Player One Connected\n");

        // Send player their ID
        playerID = 1;
        write(player1_fd, &playerID, sizeof(uint8_t));
        DEBUG_MSG("Waiting on Player Two\n");

        //Get Second Player
        if (listen(socket_fd, backlog)) {
            DEBUG_MSG("Socket listen error.\n");
            exit(EXIT_FAILURE);
        }
        if ((player2_fd = accept(socket_fd, (struct sockaddr*) &addr, &addr_len)) < 0) {
            DEBUG_MSG("Socket accept error.\n");
            exit(EXIT_FAILURE);
        }
        DEBUG_MSG("Player Two Connected\n");
        // Send player their ID
        playerID = 2;
        write(player2_fd, &playerID, sizeof(uint8_t));

        child = fork();
    }

    // -------- child process code --------
    DEBUG_MSG("Connection Successful\n");
    
    // broadcast turn time and letter count
    write(player1_fd, &letterCount, sizeof(letterCount));
    write(player2_fd, &letterCount, sizeof(letterCount));
    write(player1_fd, &turnTime, sizeof(turnTime));
    write(player2_fd, &turnTime, sizeof(turnTime));
    char* board = calloc(letterCount + 1, (sizeof(char)));

    //Game loop
    p1Wins = 0; p2Wins = 0;
    uint8_t gameStatus = 1, roundStatus, roundNum = 1, word_size_buf = 0, active_player_fd, inactive_player_fd;
    int socket_timer;
    char* word_buf;

    while (gameStatus == 1) { // game loop
        newBoard(board, letterCount);
        DEBUG_wARG("Board %s\n", board);
        usedWords = trie_create();

        // round write to player1
        write(player1_fd, &p1Wins, sizeof(p1Wins));
        write(player1_fd, &p2Wins, sizeof(p2Wins));
        write(player1_fd, &roundNum, sizeof(roundNum));
        write(player1_fd, board, strlen(board) * sizeof(char));

        // round write to player2
        write(player2_fd, &p1Wins, sizeof(p1Wins));
        write(player2_fd, &p2Wins, sizeof(p2Wins));
        write(player2_fd, &roundNum, sizeof(roundNum));
        write(player2_fd, board, strlen(board) * sizeof(char));
        

        // initialize player turns
        char turn = 'Y';
        char notTurn = 'N';
        if (roundNum % 2 == 0) {  // even round
            write(player2_fd, &turn, sizeof(char)); 
            write(player1_fd, &notTurn, sizeof(char));
            active_player_fd = player2_fd;
            inactive_player_fd = player1_fd;
        } else {                // odd round
            write(player1_fd, &turn, sizeof(char)); 
            write(player2_fd, &notTurn, sizeof(char));
            active_player_fd = player1_fd;
            inactive_player_fd = player2_fd;
        }
        roundStatus = 1;

        while (roundStatus == 1) {
            FD_ZERO(&timer_fd_set);
            FD_SET(active_player_fd, &timer_fd_set);
            socket_timer = select(active_player_fd + 1, &timer_fd_set, NULL, NULL, &timeout_s); // starting turn timer
            timeout_s.tv_sec = turnTime; // resetting timer

            if (socket_timer == -1) { // error
                PRINT_MSG("socket select error");
                exit(EXIT_FAILURE);

            } else if (socket_timer == 0) { // timeout occurs
                DEBUG_MSG("\t\tTIMEOUT OCCURED");

                if (write(active_player_fd, &(uint8_t){0}, sizeof(uint8_t))<=0) {
                        exit(EXIT_FAILURE);
                    }

                if (write(inactive_player_fd, &(uint8_t){0}, sizeof(uint8_t))<=0) {
                    exit(EXIT_FAILURE);
                }

                roundNum += 1;

                if (active_player_fd == player1_fd) { // player one invalid submission, p2 gets a point
                    p2Wins += 1;
                } else {                            // vice versa, p1 gets a point
                    p1Wins += 1;
                }   

                roundStatus = 0;

            } else { // data is read

                if ((read(active_player_fd, &word_size_buf, sizeof(uint8_t))== 0)) { // client disconnected
                    DEBUG_MSG("\tCLIENT DISCONNECTED");
                    DEBUG_MSG("\tClosing Connections");
                    close(player1_fd);
                    close(player2_fd);
                    close(socket_fd);
                    free(board);
                    exit(EXIT_FAILURE);
                }
                DEBUG_wARG("Word len %d\n", word_size_buf);
                word_buf = malloc(sizeof(char) * (word_size_buf)); // free me
            
                read(active_player_fd, word_buf, (sizeof(char) * word_size_buf)); // get word
            
                word_buf[word_size_buf] = '\0';

                if (IsValid(word_buf, usedWords, dictionary, board)) {    // check validity of word

                    trie_insert(usedWords, word_buf, strlen(word_buf)); // insert into used words

                    write(active_player_fd, &(uint8_t){1}, sizeof(uint8_t)); // send 1 to active player

                    uint8_t word_len = strlen(word_buf);
                    write(inactive_player_fd, &word_len, sizeof(uint8_t)); // send word to inactive player
                    write(inactive_player_fd, word_buf, sizeof(char) * word_size_buf);

                    int buf_fd = active_player_fd;          // swap players
                    active_player_fd = inactive_player_fd;
                    inactive_player_fd = buf_fd;

                    write(active_player_fd, &turn, sizeof(char));       // update turn characters
                    write(inactive_player_fd, &notTurn, sizeof(char));
                } else {
                    if (write(active_player_fd, &(uint8_t){0}, sizeof(uint8_t))<=0) {
                        exit(EXIT_FAILURE);
                    }

                    if (write(inactive_player_fd, &(uint8_t){0}, sizeof(uint8_t))<=0) {
                        exit(EXIT_FAILURE);
                    }

                    roundNum += 1;

                    if (active_player_fd == player1_fd) { // player one invalid submission, p2 gets a point
                        p2Wins += 1;
                    } else {                            // vice versa, p1 gets a point
                        p1Wins += 1;
                    }   

                    roundStatus = 0;
                }
            free(word_buf);
            }
        }
    }

    if (fclose(file)==-1) { // closing dictionary file
            perror("Bad File Close");
            exit(EXIT_FAILURE);
    }

    free(board);
    DEBUG_MSG("Closing Connections");
    close(player1_fd);
    close(player2_fd);
    close(socket_fd);
    return 0;
}
