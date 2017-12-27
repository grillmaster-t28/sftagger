# sftagger
A simple file tagging tool written in C

Current Version: 2017/12/28 (1.0 RC2)

## Installation
Enter the following command to build and install sftagger (if necessary as root):
* `make clean install`

## Usage
* `sftagger create` - Creates/overwrites the tags file
* `sftagger update` - Updates the tags file
* `sftagger add category` - Make new category/categories
* `sftagger add tags` - Make new tags append to a category, EX:
* `sftagger add tags small big size` - `small big` are the tags, `size` are the category
* `sftagger add tags-to` - Appends tags to the files given, EX:
* `sftagger add tags-to *.png` - `*.png` are all the files with `.png` extension
* `sftagger list categories` - Lists categories
* `sftagger list tags-of` - Lists tags of the category given, EX:
* `sftagger list tags-of size` - `size` are the category
* `sftagger list all` - Lists all categories and tags
* `sftagger read` - outputs entire file of tags
* `sftagger version` - outputs the current version
* `sftagger help` - outputs the usage commands

### Using search
* `sftagger search foo | xargs -o sxiv`

Replace `foo` with your searches (can be more than one keyword(s)) and `sxiv` with your program of choice

## Dependencies
* C compiler (C99)
* libc (recommend: C99 and POSIX >= 200809)

## Tested on
* Linux (Debian Testing GNU/Linux - 2017/12/28)

## TODO (For future releases)
* Output the category of the tag given
* Removing tags
* Removing categories
* Removing tags from each file
* File overwrite, without overriding the categories/tags
* Remove lines of unfounded files (As a separate command: `sftagger remove leftover`)
* Inherited subcategories (Not sure how it will be implemented yet)
* Optional: Include files inside a child directory upon file creation/update

## Changelog
### 1.0
#### 2017/12/28 (1.0 RC2)
* Bug fix: Tag adding: Numbers not being updated
* Bug fix: Tag adding: Duplication not checked outside of its category
* Added: ability to list out of all both categories and tags
#### 2017/12/27 (1.0 RC1)
* Bug fix: Fixed line not fully cleared
* Added: ability to list out categories
* Added: ability to list out tags of its category
* Portability: Used fgets over getline
* Makefile now included
* More documented README
### PRE-RELEASE
#### 2017/12/26
* Added: files searching via tags
* Added: file updating (Only adds new, doesn't remove deleted/unfounded)
#### 2017/12/25
* Added: tags adding to files and tags duplication checking
#### 2017/12/24
* Added: new tags creation to a category and tags duplication checking
#### 2017/12/23
* Added: new categories creation and categories duplication checking
* Added: "tags" file creation

## Release Candidates and pre-releases
Those need further testing and source code checking until ready for a release

