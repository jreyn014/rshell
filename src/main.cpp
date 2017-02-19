#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>

#include <pwd.h>
#include <unistd.h> //fork libraries
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;


//found fork template/commands online 
bool forker(char* forkInput[]) {
    pid_t pid;
    int forkStat = 0;

    pid = fork();

    if ( pid < 0) {
        perror("No Fork");
        exit (EXIT_FAILURE);
    }
    
    else if (pid == 0) {
        execvp(*forkInput, forkInput);
        perror("None executed");
        exit (EXIT_FAILURE);
    }
    
    else if (pid > 0) {
        if ( (pid = wait(&forkStat)) < 0) {
            perror("waiting");
            exit (EXIT_FAILURE);
        }
    }
    if (forkStat != 0) {
        return false;
    }
    
    return true;
}

//calls forker
bool runCommand(char* forkInput[]) {
    return forker(forkInput);
}

//removes and disregards everything after the '#' character, and does not get storde
void removeComment(string & str) {
    int commentBegin = -1;
    for (unsigned i = 0; i < str.size() && commentBegin < 0; ++i) {
        if (str.at(i) == '#') {
            str = str.substr(0, i);
        }
    }
}

//used from tokenizer.hpp library, using ';#' as delimiters to know special cases '#' and ';'
void separateCommands(vector<string> & str, string command) {
    char_separator<char> delimiters(" ", ";#");
    tokenizer <char_separator<char> > tokens(command, delimiters);
    for (tokenizer<char_separator<char> >::iterator it = tokens.begin(); it != tokens.end(); it++) {
        str.push_back(*it);
    }
}

//own exit function
void ownExit( string currCon, bool curStat, bool running) {
    if (currCon == ";") {
        running = false;
    }
    else if (currCon == "||") {
        if (curStat == false) {
            running = false;
        }
    }
    else if (currCon == "&&") {
        if (curStat == true) {
            running = false;
        }
    }
    if ( running == false) {
        exit(0);
    }
}

int main() {
    
    bool running = true;
    //gets host name from user from gethostname(from unistd.h library)
    const int hostNameSize = 30;
    char hostname[30];
    if ( gethostname(hostname, hostNameSize) == -1){
        perror("host name unavailable");
    }
    else {
        gethostname(hostname, hostNameSize);
    }
    
    //gets login name using <pwd.h> library
    struct passwd *p;
    uid_t uid;
    if ((p = getpwuid(uid = getuid())) == 0) { 
        perror("no login");
    }
    string logger = p->pw_name;
    
    while (running) {              //while the program is running
        int charCount = 0;       
        string currCon = ";";      //tracks current connector type
        bool curStat = true;    // tracks special cases of connectors
        string command;          //user input
        
        cout << logger << "@" << hostname << "$ ";
        getline(cin, command);
        vector<string> str;            //container for strings

        //find the # before storing anything
        removeComment(command);

        //parse command string
        separateCommands(str, command);
        const int forkSize = 10; //hold 10 commands at once
         //another container is needed to track the commands needed to be uploaded to fork
        char* forkInput[forkSize]; 
        /* SEPARATE COMMANDS TEST */
        // for ( unsigned int i = 0; i < str.size(); i++){
        //     cout << str.at(i) << " ";
        // }
        for ( unsigned int i = 0; i < str.size(); ++i) {
            string tracker = str.at(i);
            unsigned int checkLastCommand = str.size() - 1;
            
            if ( tracker == "quit") {
                 //our own exit function to stop the program
                ownExit( currCon, curStat, running);
            }
            //checks which connector the tracker is, then runs command accordingly
            if ( tracker == ";" || tracker == "||" || tracker == "&&") {
                
                forkInput[charCount] = 0;
                if ( currCon == ";") { 
                    curStat = runCommand(forkInput);
                }
                else if ( currCon == "||") { 
                    if ( curStat == false ) { 
                        curStat = runCommand(forkInput);
                    }
                    //else doesn't do anything because of the definition of '||'
                }
                else if ( currCon == "&&") { 
                    if ( curStat == true ) { 
                        curStat = runCommand(forkInput);
                    }
                    //else doesn't do anything because of the definition of '&&'
                }
                //needed to make values after the inputs 0 to avoid junk values
                for ( int i = 0; i < forkSize; i++) {
                    forkInput[i] = 0;   
                }
                
                //resets counter and sets the tracker to the selected connector 
                charCount = 0;
                currCon = tracker;
            }
            //Looks at the last input and decides to run it or not, mostly for single commands
            else if ( i == checkLastCommand ) { 
                //change value in str into a char to fit the input, then insert into the fork array
                char* input = const_cast<char*>(str.at(i).c_str());
                forkInput[charCount] = input;
                forkInput[charCount + 1] = 0;
                
                if ( currCon == ";") {
                     running = runCommand(forkInput);
                }
                //needed to make values after the inputs 0 to avoid junk values
                for ( int k = 0; k < forkSize; k++) {
                    forkInput[k] = 0;
                }
                //resets counter and sets the tracker to the selected connector
                charCount = 0;
                currCon = tracker;
            }
            //checks if the tracker is at a command, if it is, push it into the forkInput array
            else if ( tracker != ";" || tracker != "||" || tracker != "&&") {
                char* input = const_cast<char*>(str.at(i).c_str());
                forkInput[charCount] = input;
                charCount++;
            }
        }
    }
    return 0;
}