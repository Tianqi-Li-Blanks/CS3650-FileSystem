# COW File System

File System Homework

Extend the filesystem provided as starter code to support more functionality:

Read and write from files larger than one block. For example, you should be able to support one hundred 1k files or five 100k files.
Create directories and nested directories. Directory depth should only be limited by disk space (and possibly the POSIX API).
Remove directories.
Hard links.
Symlinks
Support metadata (permissions and timestamps) for files and directories.

Don’t worry about multiple users. Assume the user mounting the filesystem is also the owner of any filesystem objects.
Anything else that’s covered in the tests.


Part 1: Maintain your HW10 functionality
Some functionality for HW10 will be graded again for CH02. Complete any missing HW10 functionality and keep it working through the new CH03 additions.

Starting from your work from HW10, you can make yourself complete CH03 starter code as follows:

Take the new test.pl and Makefile from the provided CH03 starter code.
Rename your “nufsmount.c” to “cowmount.c”
Rename your “nufstool.c” to “cowtool.c”
As usual, you can start from either partner’s HW10 code or a combination of the two.

Part 2: Add Copy on Write Features
Modify your file system to use copy on write for all operations that modify the filesystem. Each of these operations should create a new version of the filesystem, with a new root directory that has shared structure with previous versions of the filesystem.

The operations that modify the filesystem and should therefore create a new version are:

write
truncate
mknod
unlink
link
rename
chmod
set time
Your filesystem should preserve at least the 7 most recent previous versions.

In order to be able to see this copy-on-write versioning functionality, extend the functionality of the filesystem tool to include two new operations:

List versions
Rollback to a previous version
Examples:

$ ./cowtool versions disk0.cow 
Versions for disk0.cow:
16 unlink /x3.txt
15 write /x5.txt
14 truncate /x5.txt
13 mknod /x5.txt
12 write /x4.txt
11 truncate /x4.txt
10 mknod /x4.txt
9 write /x3.txt
$ ./cowtool rollback disk0.cow 15
Rollback disk0.cow to version 15
$ ./cowtool ls disk0.cow
...
/x1.txt
/x2.txt
/x3.txt
/x4.txt
/x5.txt
In your list of versions, the first version in the list must be the current version, and that must be the second line of output.

Your version numbers must be assigned in consecutive order, and you can assume that a 32-bit int won’t overflow.

The TAs will be manually testing your submission for requested functionality that isn’t fully covered by the automated tests.

Hints & Tips
There aren’t man pages for FUSE. Instead, the documentation is in the header file: /usr/include/fuse/fuse.h
The basic development / testing strategy for this assignment is to run your program in one terminal window and try FS operations on the mounted filesystem in another separate terminal window.
Read the manual pages for the system calls you’re implementing.
To return an error from a FUSE callback, you return it as a negative number (e.g. return -ENOENT). Some things don’t work if you don’t return the right error codes.
Read and write, on success, return the number of bytes they actually read or wrote.
You need to implement getattr early and make sure it’s correct. Nothing works without getaddr. The mode for the root directory and hello.txt in the starter code are good default values for directories and files respectively.
The functions “dirname” and “basename” exist, but may mutate their argument.
https://www.cs.hmc.edu/~geoff/classes/hmc.cs135.201109/homework/fuse/fuse_doc.html
