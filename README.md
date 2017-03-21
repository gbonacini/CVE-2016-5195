Description:
============

dcow is a possible exploit of the vulnerability CVE-2016-5195.
Running the program as unprivileged user on a vulnerable system, it'll modify the /etc/passwd file, forcing the password "dirtyCowFun" (SHA-512, but could be modified for older standards).
In case of successful execution, doing a "su" with that password, a root shell will be available.
Using the -s option (recomended), a root shell will be automatically opened.
A backup of the original /etc/passwd will be created in the current execution directory as .ssh_bak, if dcow is used with no options or with -n (see example below).

DISCLAIMER:
===========

This code has been posted for information and educational purposes. The intrusion in systems and services without the owner's authorisation is illegal. The misuse of the information and the software in this repo can result in criminal charges brought against the perpetrator of the crime. Any actions and or activities related to the material contained within this Git Repository is solely your responsibility. The author will not be held responsible in the event any criminal charges be brought against any individuals misusing the information or the software in this website to break the law.

Prerequisites:
==============

A CVE-2016-5195 vulnerable system.

The program was successfully used with:

- RHEL7 Linux  x86_64;
- RHEL4 (4.4.7-16, with "legacy" version) 
- Debian 7 ("wheezy");
- Ubuntu 14.04.1 LTS  
- Ubuntu 14.04.5 LTS
- Ubuntu 16.04.1 LTS
- Ubuntu 16.10
- Linux Mint 17.2

and compiled with: 

- clang version 4.0.0;
- gcc version 6.2.0 20161005 (Ubuntu 6.2.0-5ubuntu12) 
- gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.1) 
- gcc version 4.8.5 20150623 (Red Hat 4.8.5-4) (GCC);
- gcc version 4.8.4 (Ubuntu 4.8.4);
- gcc version 4.8.2 (Ubuntu 4.8.2-19ubuntu1)
- gcc version 4.7.2 (Debian 4.7.2-5);
- gcc version 4.4.7 (with "legacy" version) 

Exploit DB Reference:
=====================

EDB-ID: 40847

https://www.exploit-db.com/exploits/40847/

Installation:
=============

- Compile the program:
  make

- Start the program:<BR>
  ./dcow <BR>
  or  <BR>
  ./dcow -s     # Automatically open a root shell and restore the passwd file.<BR>
  ./dcow -s -n  # Automatically open a root shell but doesn't  restore the passwd file.

- Online help:

  ./dcow -h

WIKI:
=====

In the "wiki" section of this page is present a partial list of the vulnerable kernels/distros.

"Legacy" version:
=================

In the directory "legacy" is present a raw porting of this program that permits the compilation using pre-c++11 compilers.

Golang version:
===============

A version of this exploit written in Go + CGO language is present in the "golang" directory. It permits the creation of a static executable without external dependencies.
