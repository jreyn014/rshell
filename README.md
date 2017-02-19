<snippet>
  <content><![CDATA[
# ${1:rShell}

A program that imitates the a shell terminal that takes in basic commands such as echo, mkdir, and ls.

## Installation

Enter into the terminal the following steps: 
1. git clone https://github.com/jreyn014/rshell.git

2. cd rshell

3. git checkout hw2

4. make

5. bin/rshell

## Usage

Can submit most commands and connectors from a basic shell terminal, such as || and &&.
These commands are called upon from execvp(). Perror() is used for invalid system commands, such as a forking error
or getlogin()/gethostname() errors. We were able to display the extra hostname and getlogins in the command output.
Other than the execvp() commands, we also included our own exit function called quit(), where the terminal
can be stopped if the user types quit.

##Bugs

This program does not support grouping, such as using () and []. Also, echoing does not support "", where the quotes
are output if user types in echo "hello". We also aren't able to track the previous
command using the arrow keys like in a normal terminal. 
Our template is different from assignment 1, as we thought each command needed to be implemented
but we found out about execvp(), which completely changed our mindset on needing classes.

## License
The License can be found in the LICENSE.txt. 


]]></content>
  <tabTrigger>readme</tabTrigger>
</snippet># rshell