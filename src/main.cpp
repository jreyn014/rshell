#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <signal.h>

#include <pwd.h>
#include <unistd.h> //fork and test libraries
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>     
#include <errno.h> 
#include "fcntl.h" //input/output/pipe library

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

//changes string into char
char* convert(const string& str){
	char* p = new char[str.size()+1];
	strcpy(p,str.c_str());
	return p;
}

//testing
// int findCommand(vector<char*> v, string s) {
// 	vector<string> temp, holder;
	
// 	for(unsigned j = 0; j < v.size(); j++) {
// 		string separateCommand = string(v.at(j));
// 		temp.push_back(separateCommand);
// 		holder.push_back(separateCommand);
// 	}
	
// 	int index = -1;
	
// 	for(unsigned i = 0; i < temp.size(); i++) {
// 		if(temp.at(i) == s) {
// //			cout << "exists" << endl;
// 			if(index >= 0) {
// 				index = -2;
// 			}
// 			else if(index == -1) {
// 				index = i;
// 			}
// 		}
// 	}
	
// 	for(unsigned k = 0; k < temp.size(); k++) {
// 		v.at(k) = &(holder.at(k).at(0));
// 	}
	
// 	return index;
// }


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

//our piping function, which splits pipeline into two, and calls both arguements one at a time
int piping(vector<string> & pipeLine) { 
    int stat = 0;
    
    vector<string> argLeft;
	vector<string> argRight;
	vector<char*> argLeftC;
	vector<char*> argRightC;
	bool argSwitch = false;
    
    for( unsigned i = 0; i < pipeLine.size(); i++) { 
        if (pipeLine.at(i) == "|" && argSwitch) { 
            break;
        }
        else if ( pipeLine.at(i) == "|") { 
            argSwitch = true;
        }
        else if ( argSwitch ) {
            argRight.push_back(pipeLine.at(i));
        }
        else { 
            argLeft.push_back(pipeLine.at(i));
        }
        // cout << pipeLine.at(i) << endl;
    }
    
    while ( pipeLine.front() != "|" && !pipeLine.empty()) { 
        pipeLine.erase(pipeLine.begin());
    }
    if ( !pipeLine.empty()) { 
        pipeLine.erase(pipeLine.begin());
    }
    
    transform(argLeft.begin(), argLeft.end(), back_inserter(argLeftC), convert);
	argLeftC.push_back(NULL);

	transform(argRight.begin(), argRight.end(), back_inserter(argRightC), convert);
	argRightC.push_back(NULL);
    
//     for (unsigned i = 0; argLeftC.at(i) != NULL; i++) cout << argLeftC.at(i) << " ";
// 	cout << endl;
    
	const int READ_PIPE = 0;
	const int WRITE_PIPE = 1;
	int fd[2];


	if (pipe(fd) == -1){
		perror("There was an error with pipe()");
	}

	int pid = fork();
	if (pid == -1){
		perror("There was an error with fork()");
		return -1;
	}
	else if (pid == 0){

		if (-1 == dup2(fd[WRITE_PIPE], 1))
			perror("There was an error with dup2()");
		if (-1 == close(fd[READ_PIPE]))
			perror("There was an error with close()");

		if (-1 == execvp(argLeftC[0], &argLeftC[0]))
			perror("Executing execvp in pipe error");

		exit(1);
	}
	else if (pid > 0){
		sigignore(SIGINT);
		int savestdin;
		if (-1 == (savestdin = dup(0)))
			perror("There was an error with dup()");
		if (-1 == wait(0))
			perror("There was an error with wait()");
		
		int pid2 = fork();
		if (pid2 == -1){
			perror("There was an error with fork()");
			return -1;
		}	
		else if (pid2 == 0){

			if (-1 == dup2(fd[READ_PIPE],0))
				perror("There was an error with dup2");
			if (-1 == close(fd[WRITE_PIPE]))
				perror("There was an error with close()");

		
			for (unsigned k = 0; k < pipeLine.size(); k++){
				if (pipeLine.at(k) == "|"){
					stat = piping(pipeLine);
					break;
				}
			}
			if (-1 == execvp(argRightC[0], &argRightC[0]))
				perror("There was an error with execvp()");

			exit(1);
		}
		else if (pid2 > 0){
			if (-1 == close(fd[WRITE_PIPE]))
				perror("There was an error with close()");
			if (-1 == wait(0))
				perror("There was an error with wait()");
		}


		if (-1 == dup2(savestdin, 0))
			perror("There was an error with dup2()");

	}

	for (unsigned j = 0; j < argLeftC.size(); j++){
		if (argLeftC.at(j) != NULL)
			delete[] argLeftC.at(j);
	}
	for (unsigned k = 0; k < argRightC.size(); k++){
		if (argRightC.at(k) != NULL)
			delete[] argRightC.at(k);
	}

	return stat;

}

//put the input (<) as a function
int redirInput(string infile, int & savestdin) { 
    int fd = 0;
    // cout << "REDIR: Input" << endl;
	if (-1 == (fd = open(infile.c_str(), O_RDONLY))){
		perror("Error: open()");
		return -1;
	}

	if (-1 == (savestdin = dup(0))){
		perror("Error: dup()");
	}

	if (-1 == dup2(fd, 0)) { 
		perror("Error: dup2()");
	}
	
	return fd;

}

//put the output (>) as a function
int redirOutput(string outfile, int & savestdout, int replaceFD) { 
    int fd = 0;
    // cout << "REDIR: OUTPUT" << endl;
	if (-1 == (fd = open(outfile.c_str(), O_CREAT|O_WRONLY, 0666))){
		perror("there was an error with open()");
		return -1;
	}

	if (-1 == (savestdout = dup(replaceFD))){
		perror("There was an error with dup()");
	}

	if (-1 == dup2(fd, replaceFD))
		perror("There was an error with dup2()");

	return fd;

}

//put output2 ( >> ) as a function 
int redirDoubleOut(string outfile, int & savestdout, int replaceFD) { 
    int fd = 0;
	if (-1 == (fd = open(outfile.c_str(), O_CREAT|O_APPEND|O_WRONLY, 0666))){
		perror("there was an error with open()");
		return -1;
	}

	if (-1 == (savestdout = dup(replaceFD))){
		perror("There was an error with dup()");
	}

	if (-1 == dup2(fd, replaceFD))
		perror("There was an error with dup2()");

	return fd;

}

//made to handle >, >>, <, and pipe
bool forkerIO(char* forkInput[], int forkSize) { 
    std::string pipe = "|";
    int numPipe = 0;
    int forkStat = 0;
    int checker = -1;
    vector<char* > replace; 
    for ( unsigned int i = 0; i < forkSize; i++) { 
        // cout << forkSize;
        if ( forkInput[i] == pipe) { 
            numPipe++;
        }
        replace.push_back(forkInput[i]);
        // cout << replace.at(i) << " ";
    }
    // cout << numPipe << endl;
    
    bool noPipe = true;
    string input = "<";
    string output = ">";
    string doubleOut = ">>";
    int fd = -1;
    int savestdout = -1;
    int savestdin = -1;
    
    vector<string> pipeCopy;
    for ( unsigned k = 0; k < replace.size(); k++) { 
        string str = replace.at(k);
        pipeCopy.push_back(str);
        // cout << pipeCopy.at(k) << endl;
    }
    
    for ( unsigned i = 0; i < pipeCopy.size(); i++) {
         if ( pipeCopy.at(i) == input || pipeCopy.at(i) == output || pipeCopy.at(i) == doubleOut) {
             replace.resize(i);
             break;
         }
    }
    
    //INPUT
    bool alreadyInput = 0;
    
    if ( pipeCopy.back() == input || pipeCopy.back() == output || pipeCopy.back() == doubleOut || pipeCopy.back() == "|") {
        errno = 1;
        perror("Invalid redirection/pipe at back");
        return false;
    }
    
    if ( pipeCopy.at(0) == input || pipeCopy.at(0) == output || pipeCopy.at(0) == doubleOut || pipeCopy.at(0) == "|") { 
        errno = 1;
        perror("Invalid redirection at front");
        return false;
    }
    
    for ( unsigned i = 0; i < pipeCopy.size(); i++) { 
        if ( (pipeCopy.at(i) == input) && alreadyInput) { 
            perror("Error: input redirect\n");
            return false;
        }
        else if ( pipeCopy.at(i) == input) { 
            alreadyInput = 1;
            if ( (fd = redirInput(pipeCopy.at(i+1), savestdin)) == -1) { 
                // cout << "REDIR: INPUT" << endl;
                return false;
            }
            while ( (i != pipeCopy.size()) && (pipeCopy.at(i) != output) && (pipeCopy.at(i) != doubleOut) && (pipeCopy.at(i) != "|")) {
                // cout << "ERASED" << endl;
                pipeCopy.erase(pipeCopy.begin() + i);
            }
        }
    }
    
    int counter = 0;
    int pos = -1;
    
    for ( unsigned i = 0; i < pipeCopy.size(); i++) { 
            if ( pipeCopy.at(i) == output || pipeCopy.at(i) == doubleOut) { 
                counter++;
            }
    }
    
    int childStat = 0;
    
    do { 
        counter--;
        
        for ( unsigned i = 0; i < pipeCopy.size(); i++) { 
            if ( pipeCopy.at(i) == output || pipeCopy.at(i) == doubleOut) {
                pos = (int)i;
            }
        }
        
        
        //output
        if ( pos != -1) { 
            
            if ( pipeCopy.at(pos) == output) { 
                if ( (fd = redirOutput(pipeCopy.at(pos + 1), savestdout, 1)) == -1) { 
                    return false;
                }
            }
            else if (pipeCopy.at(pos) == doubleOut) { 
                if ( (fd = redirDoubleOut(pipeCopy.at(pos + 1), savestdout, 1)) == -1) { 
                    return false;
                }
            }    
            
            pipeCopy.resize(pos);
        }
        
        
        vector<string> pipeLine;
        bool noPipe = 1;
        
        for ( unsigned l = 0; l < pipeCopy.size(); l++) { 
                if ( !noPipe && (pipeCopy.at(l) == input || pipeCopy.at(l) == output || pipeCopy.at(l) == doubleOut)) { 
                    break;
                }
                
                if ( !noPipe ) { 
                    pipeLine.push_back(pipeCopy.at(l));
                }
                
                if ( pipeCopy.at(l) == "|" && noPipe) { 
                    noPipe = 0;
                    while ( l > 0 ) { 
                        if ( pipeCopy.at(l) == input || pipeCopy.at(l) == output || pipeCopy.at(l) == doubleOut) { 
                            l++;
                            break;
                        }
                        l--;
                    }
                    l--;
                }
        }
        
        if ( !pipeLine.empty() ) { 
            if ( piping(pipeLine) == -1) { 
                perror("Piping failed");
            }
        }
        else { 
            int pid = fork();
            
            replace.push_back(NULL);
            
            if ( pid == -1) { 
                perror("Error: fork()");
                exit(1);
            }
            else if ( pid == 0) { 
                if ( execvp(replace[0], &replace[0]) == -1) { 
                    perror("Error: execvp()");
                    exit(1);
                }
            }
            else { 
                sigignore(SIGINT);
                int waitPid = 0;
                
                if ( waitpid(waitPid, &childStat, 0) == -1) { 
                    perror("Error: wait()");
                }
                
                childStat = WEXITSTATUS(childStat);
            }
        }
        
        if ( fd != -1 && close(fd) == -1) { 
            perror("Error: close()");
        }
        
        if ( counter <= 0) { 
            if ( savestdin != -1 && dup2(savestdin, 0) == -1) { 
                perror("Error: dup2()");
            }
        }
        else {
            rewind(stdin);
            }
        if ( savestdout != -1 && dup2(savestdout, 1) == -1) { 
            perror("Error: dup2()");
        }
        
        } while ( counter > 0);
    if ( childStat < 0) {
         return false;
    }
    else { 
        return true;
    }
}

//calls forker or the test function
bool runCommand(char* forkInput[]) {
    std::string testString = "test";
    std::string inputString = "<";
    std::string outputString = ">";
    std::string doubleOutString = ">>";
    std::string pipe = "|";
    
    char** n = forkInput;
    int inputChecker = 0;
    int outputChecker = 0;
    int doubleOutChecker = 0;
    int pipeChecker = 0;
    double forkSize;
   // cout << "helloman" << endl;
    while ( *n != '\0') { 
    //    cout << *n << endl;
        if ( *n == inputString) {
          //  cout << "0" << endl;
            inputChecker++;
        }
        if ( *n == outputString) { 
            outputChecker++;
        }
        if ( *n == doubleOutString) { 
            doubleOutChecker++;
        }
        if ( *n == pipe) { 
            pipeChecker++;
        }
        //cout << "oh no" << endl;
        *n++;
        forkSize++;
    }
    // cout << "Fork Size: " << forkSize << endl;
    if ( inputChecker > 1 || outputChecker > 1 || doubleOutChecker > 1) { 
        errno = 1;
        perror("Found more than one input/output direction");
        exit(1);
    }
    //we need this because using string literals
    if ( forkInput[0] == testString){
        return tester(forkInput);   //run test function
    }
    else if ( inputChecker == 1 || outputChecker == 1 || doubleOutChecker == 1 || pipeChecker >= 1) { 
        // cout << "helol" << endl;
        return forkerIO(forkInput, forkSize);
    }
    else {
        // cout << "Nothing" << endl;
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
    string openPar = "(";
    string closePar = ")";
    string openBrack = "[";
    string closeBrack = "]";
    char_separator<char> delimiters(" ", "<[]();#");
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
            perror("Not all close parenthesis have an open parenthesis");
            str.clear();
            return;
        }

        //avoid doing a command with incorrect number of brackets
        //specifically, there is at least one occurrence where there are more "]" than "["
        if (closeBrackCnt > openBrackCnt) {
            perror("Not all closing brackets have an opening bracket");
            str.clear();
            return;
        }

        str.push_back(*it);
    }
    //avoid doing a command with incorrect number of parenthesis
    //specifically, has more "(" than ")"
    if (openParCnt != closeParCnt) {
        perror("Not all open parenthesis have a close parenthesis");
        str.clear();
        return;
    }

    //avoid doing a command with incorrect number of brackets
    //specifically, has more "[" than "]"
    if (openBrackCnt != closeBrackCnt) {
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
        bool piper = false; 
        int charCount = 0;       
        string currCon = ";";      //tracks current connector type
        bool curStat = true;    // tracks special cases of connectors
        string command;          //user input
        const int forkSize = 22; //hold 10 commands at once
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
            
            if ( tracker == "quit") {
                 //our own exit function to stop the program
                ownExit( currCon, curStat, running);
            }
            
            if ( tracker == "["){
                str.at(i) = "test";
            }
            if ( tracker == "|") { 
                if ( i != str.size() - 1) { 
                    if ( str.at(i+1) == "|") {
                        cout << "no pipe" << endl;
                        piper = false;
                    }
                    else {
                        piper = true;
                    }
                }
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
                // cout << tracker << endl;
                // cout << "ENTER" << endl;
                if ( (currCon == "||" && curStat == true) || (currCon == "&&" && curStat == false)) {
                    //deletes everything after the parenthesis since everything after is not needed
                    // for (unsigned int k = 0; tracker != ")"; k++) {
                    //     i++;
                    // }
                    // cout << "DELETE 2" << endl;
                    // while ( tracker != ")") {
                    //     cout << tracker << endl;
                    //     i++;
                    // }
                    for (unsigned int k = 0; str.at(i) != ")"; k++) {
                        cout << str.at(i) << endl;
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