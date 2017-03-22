<snippet>
  <content><![CDATA[
# ${1:rShell}

A program that imitates the a shell terminal that takes in basic commands such as echo, mkdir, and ls. Connectors are supported such as
|| , && and ;. Piping (|) and input/output redirection are also supported ( >, <, and >>).


## Installation

Enter into the terminal the following steps: 
1. git clone https://github.com/jreyn014/rshell.git

2. cd rshell

3. git checkout hw4

4. make

5. bin/rshell

## Usage

Can submit most commands and connectors from a basic shell terminal, such as || and &&.
These commands are called upon from execvp(). Perror() is used for invalid system commands, such as a forking error
or getlogin()/gethostname() errors. We were able to display the extra hostname and getlogins in the command output.
Other than the execvp() commands, we also included our own exit function called quit(), where the terminal
can be stopped if the user types quit. Able to call user pipes (|) and input/output( <, >, >>) and run them accordingly.

##Bugs

Echoing does not support "", where the quotes are output if user types in echo "hello". We also aren't able to track the previous
command using the arrow keys like in a normal terminal. There are segmentation faults if only connectors are inputted into the terminal. 
When using 3 pipes, file is sometimes not created, and only beginning and end commands are realized. 

## License
The License can be found in the LICENSE.txt. 


]]></content>
  <tabTrigger>readme</tabTrigger>
</snippet># rshell
