

## Abilities:

1. Redirecting stderr output
      * Example command: ls -l nofile 2> mylog
      * Previous assumptions: None

2. Appending to an existing file
      * Example command: ls -l >> mylog
      * Previous assumptions: None

3. Changing the prompt
      * Example command: prompt = myprompt
      * Previous assumptions: None

4. Printing
      * Example command: echo someting
      * Previous assumptions: None

5. Printing the status of the last command
      * Example command: echo $?
      * Previous assumptions: None

6. Repeat the last command
      * Example command: !!
      * Previous assumptions: Can't repeat if/cd/read commands

7. Exit shell
      * Example command: quit
      * Previous assumptions: None

8. Handle control-c typed - the progrem won't stop but will print "You typed Control-C!" and Stop every proccess excapt main.
      * Example command: None
      * Previous assumptions: Pressing ctr-c is not a command so the shell will still wait for a proper command.

9. Dealing with pipes
      * Example command: date | grep Fri
      * Previous assumptions: We implemented the regular piping with the assumption that it supposed to work the same as the regular bash with the extention of multiple pipes.

10. Storing variables
      * Example command: $person = David
      * Previous assumptions: None

11. Getting a value from the user
      * Example command: read name
      * Previous assumptions: If a string with spaces entered, only the first part will be stored (before the first space)

12. Browse previous commands and run them
      * Example command: "down arrow" or "up arrow" + enter
      * Previous assumptions: Can't run a previous if command. If there are no more commands a message will be printed but the position will not change.

13. IF/ELSE
      * Example command:
      
              if date | grep Fri

              then

              echo "Shabat Shalom"

              else

              echo "Hard way to go"

              fi

              (line by line)

      * Previous assumptions: If one want to use the if command he need to follow this exact pattern line by line:

              if command/pipes

              then

              commands

              else

              commands

              fi



## How To Run:
 * run `make` on command line
 * then, `./myshell` and the shell start to work.
