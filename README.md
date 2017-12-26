# sftagger
A Simple File Tagging tool written in C

## Using search
`sftagger search foo | xargs -o sxiv`
Replace `foo` with your searches and `sxiv` with your program of choice

## Installation
`gcc -o sftagger sftagger.c`
`sudo cp sftagger /usr/local/bin/.`

## DONE
* File creation
* Creating new categories and categories duplication checking
* Creating new tags to a category and tags duplication checking
* Adding tags to files and tags duplication checking
* File updating (Only adds new, doesn't remove deleted)
* Search files via tags

## TODO
* List out categories
* List out tags of its category
* Output the category of the tag given
* Remove lines of unfounded files (As a separate command: `sftagger remove leftover`)
* Optional: Include files inside a child directory upon file creation/update

