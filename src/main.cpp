#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>


#include <pwd.h>
#include <unistd.h> //fork and test libraries
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>     
#include <errno.h>     //perror library 

#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

//test -e, -f, -d, if none works default to -e
//uses stat() to check tests 
bool tester(char* forkInput[]){
    // cout << "Entered" << endl;
    int index = 1;
    struct stat info;
    std::string literalE = "-e";
    std::string literalD = "-d";
    std::string literalF = "-f";
    //keeps track of the commands after "test [command]"
    if ( forkInput[1] == literalE || forkInput[1] == literalF || forkInput[1] == literalD){
        index++;
    }
    
    if ( stat( forkInput[index], &info ) == -1 ) {
        perror("Cannot Access");
        return false;
    }
    
    if ( forkInput[1] == literalF) { 
        if ( S_ISREG(info.st_mode) ) {
            cout << "(True)" << endl;
            return true;
        }
        else {
            cout << "(False)" << endl;
            return false;
        }
    }
    
    else if ( forkInput[1] == literalD) {
        if ( S_ISDIR(info.st_mode) ) {
            cout << "(True)" << endl;
            return true;
        }
        else {
            cout << "(False)" << endl;
            return false;
        }
    }

    else {
        if ( info.st_mode & S_IFMT ) {
            cout << "(True)" << endl;
            return true;
        }
        else {
            cout << "(False)" << endl;
            return false;
        }
    }

    return false;

}

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

//calls forker or the test function
bool runCommand(char* forkInput[]) {
    //we need this because using string literals
    std::string testString = "test";
    if ( forkInput[0] == testString){
        return tester(forkInput);   //run test function
    }
    else {
        return forker(forkInput);
    }
}

//removes and disregards everything after the '#' character, and does not get storde
void removeComment(string & str) {
    for (unsigned i = 0; i < str.size(); ++i) {
        if (str.at(i) == '#') {
            str = str.substr(0, i);
        }
    }
}

//used from tokenizer.hpp library, using ';#' as delimiters to know special cases '#' and ';'
void separateCommands(vector<string> & str, string command) {
    int openParCnt = 0;
    int closeParCnt = 0;
    int openBrackCnt = 0;
    int closeBrackCnt = 0;
    int commandCnt = 0;
    string openPar = "(";
    string closePar = ")";
    string openBrack = "[";
    string closeBrack = "]";
    char_separator<char> delimiters(" ", "[]();#");
    tokenizer <char_separator<char> > tokens(command, delimiters);
    for (tokenizer<char_separator<char> >::iterator it = tokens.begin(); it != tokens.end(); it++) {
        //counts number of occurrences of "(" and ")" respectively
        if (*it == openPar) {
            openParCnt++;
        }
        else if (*it == closePar) {
            closeParCnt++;
        }

        //counts number of occurrences of "[" and "]" respectively
        if (*it == openBrack) {
            openBrackCnt++;
        }
        else if (*it == closeBrack) {
            closeBrackCnt++;
        }
        
        //avoid doing a command with incorrect number of parenthesis
        //specifically, there is at least one occurrence where there are more ")" than "("
        if (closeParCnt > openParCnt) {
            errno = 1;
            perror("Not all close parenthesis have an open parenthesis");
            str.clear();
            return;
        }

        //avoid doing a command with incorrect number of brackets
        //specifically, there is at least one occurrence where there are more "]" than "["
        if (closeBrackCnt > openBrackCnt) {
            errno = 1;
            perror("Not all closing brackets have an opening bracket");
            str.clear();
            return;
        }
        commandCnt++;
        str.push_back(*it);
    }
    //checks for a connector input with no commands
    if ( (commandCnt == str.size()) && (str.at(0) == "||" || str.at(0) == "&&" || str.at(0) == ";")) { 
        errno = 1;
        perror("Single Command Inputted");
        str.clear();
        return;
    }
    //checks if user only inputs parenthesis "()" or "()()"
    if ( (openParCnt == closeParCnt) && (commandCnt == (openParCnt + closeParCnt))) {
        errno = 1;
        perror("No commands, only parenthesis");
        str.clear();
        return;
    }
    //avoid doing a command with incorrect number of parenthesis
    //specifically, has more "(" than ")"
    if (openParCnt != closeParCnt) {
        errno = 1;
        perror("Not all open parenthesis have a close parenthesis");
        str.clear();
        return;
    }

    //avoid doing a command with incorrect number of brackets
    //specifically, has more "[" than "]"
    if (openBrackCnt != closeBrackCnt) {
        errno = 1;
        perror("Not all opening brackets have a closing bracket");
        str.clear();
        return;
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
        const int forkSize = 10; //hold 10 commands at once
         //another container is needed to track the commands needed to be uploaded to fork
        char* forkInput[forkSize]; 
        
        cout << logger << "@" << hostname << "$ ";
        getline(cin, command);
        vector<string> str;            //container for strings

        //find the # before storing anything
        removeComment(command);

        //parse command string
        separateCommands(str, command);
        /* SEPARATE COMMANDS TEST */
        // for ( unsigned int i = 0; i < str.size(); i++){
        //     if ( i == 0) {
        //         cout << "START:" << endl;
        //     }
        //     cout << str.at(i) << " ";
        //     if ( i == str.size() - 1){
        //         cout << "END" << endl;
        //     }
        // }
        for ( unsigned int i = 0; i < str.size(); i++) {
            string tracker = str.at(i);
            unsigned int checkLastCommand = str.size() - 1;
            
            if ( str.at(i) == "["){
                str.at(i) = "test";
            }
            if ( tracker == "quit") {
                 //our own exit function to stop the program
                ownExit( currCon, curStat, running);
            }
            //checks which connector the tracker is, then runs command accordingly
            if ( tracker == ")" || tracker == ";" || tracker == "||" || tracker == "&&") {
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
                for ( int k = 0; k < forkSize; k++) {
                    forkInput[k] = 0;
                }

                //resets counter and sets the tracker to the selected connector
                charCount = 0;
                currCon = tracker;
            }
            //separate case for the parentheses.
            else if ( tracker == "(") {
                if ( (currCon == "||" && curStat == true) || (currCon == "&&" && curStat == false)) {
                    //deletes everything after the parenthesis since everything after is not needed
                    while ( str.at(i) != ")") {
                        i++;
                    }
                }
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
                else if ( currCon == "&&") {
                    if ( curStat == true){
                        running = runCommand(forkInput);
                    }
                }
                else if ( currCon == "||") {
                    if ( curStat == false){
                        running = runCommand(forkInput);
                    }
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
            else if ( tracker != "(" || tracker != ";" || tracker != "||" || tracker != "&&") {
                char* input = const_cast<char*>(str.at(i).c_str());
                forkInput[charCount] = input;
                charCount++;
            }
        }
    }
    return 0;
}