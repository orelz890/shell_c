

##Abilities:

1. Redirecting stderr output
example command: ls -l nofile 2> mylog
Previous assumptions: None

2. Appending to an existing file
example command: ls -l >> mylog
Previous assumptions: None

3. Changing the prompt
example command: prompt = myprompt
Previous assumptions: None

4. Printing
example command: echo someting
Previous assumptions: None

5. Printing the status of the last command
example command: echo $?
Previous assumptions: None

6. Repeat the last command
example command: !!
Previous assumptions: Can't repeat if/cd/read/pipe commands

7. Exit shell
example command: quit
Previous assumptions: None

8. Handle control-c typed - the progrem won't stop but will print "You typed Control-C
!". Stop every proccess excapt main.
example command: None
Previous assumptions: None

9. Dealing with pipes
example command: date | grep Fri
Previous assumptions: None

10. Storing variables
example command: $person = David
Previous assumptions: None

11. Getting a value from the user
example command: read name
Previous assumptions: None

12. Browse previous commands and run them
example command: "down arrow" or "up arrow" + enter
Previous assumptions: Can't run a previous if command

13. IF/ELSE
example command:
if date | grep Fri
then
echo "Shabat Shalom"
else
echo "Hard way to go"
fi
(line by line)
Previous assumptions: If one want to use the if command he need to follow this exact pattern line by line:
if command
then
commands
else
commands
fi







