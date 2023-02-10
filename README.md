# Description
This project is a shell that can run most UNIX commands and can be installed into your linux environment. There are also some add-on commands such as bg, fg, l, ll that are created and included in this shell.

### Dependency
- sudo apt-get install libreadline-dev

## Installation Process
The installation process is very easy, with several simple commands:

1. ./configure
2. make
3. sudo make install

After installation, the shell can be used directly by typing "shell" anywhere in Linux. Have fun!

# Commands
### change directory
    cd [dest]

### list directory 
    ls [flags] [dest]

supported flags:
- -a   (list all files in directory)
- -l   (list files vertically)
- -d[0-3] (change default printing format, 0="ls", 1="ls -l", 2="ls-a", 3="ls -al")

### quick list of files in directory 
    l                           
(same as ls -l)

### quick list of all files in directory 
    ll                          
(same as ls -al)

### background and foreground commands
    - bg [any_UNIX_command...]    
( command run in background with stdout redirected )
    - fg [any_UNIX_command...]    
( command will run in foreground, like linux )
    - [any_UNIX_command] [&/bg]      
( if [&/bg] is included, command runs in background )

### background running process 
    jobs                        ( prints out all background running process )

### kill processes
    kill [pid] [pid2...]        ( kill process(es) )

### quit shell
    quit                        ( Ctrl C will not terminate the shell )

To be continued...

# Updates
If you would like to modify the shell yourself, just simply "make" and "sudo make install". Otherwise, you can try to reset or clean the whole file and re-install it:

[Re-Configuration]
1. make clean
2. ./configure
3. make
4. sudo make install

[Re-Generate_the_config_files]
1. make reset
2. autoreconf -i
3. ./configure
4. make
5. sudo make install

# Author
Darren <darrench3140@gmail.com>

### References
- Writing Shells, Nightmare edition: https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf

- Directory Sorting and Listing: https://stackoverflow.com/questions/9743485/natural-sort-of-directory-filenames-in-c

- Using Dup2 to redirect output: http://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html