#!/bin/bash
set -e
echo "Initializing secrets for V2X Hub..."
# Save current path
directory=$(pwd)
# Make passwords for mysql
mkdir -p secrets && cd secrets || return # SC2164 - Use return in case cd fails 

# Creates password files where user inputs password
FILE1=mysql_root_password.txt
FILE2=mysql_password.txt
if test -f "$FILE1"; then
    echo "$FILE1 exists."
else
    read -r -p "enter password for the mysql_root_password: " sql_root_pass # SC2162 - read without -r will mangle backslashes 
    echo "$sql_root_pass" > sql_root_pass.txt
    # Remove endline characters from password files
    tr -d '\n' <sql_root_pass.txt> mysql_root_password.txt && rm sql_root_pass.txt
fi

if test -f "$FILE2"; then
    echo "$FILE2 exists."
else
    read -r -p "enter password for mysql_password: " sql_pass
    echo "$sql_pass" > sql_pass.txt
    # Remove endline characters from password files
    tr -d '\n' <sql_pass.txt> mysql_password.txt && rm sql_pass.txt
fi

cd "$directory" || return # return in case cd fails

echo "Secret Initialization Complete."
