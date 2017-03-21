Golang Version:
===============

This is a porting of the c++ exploit on Go + Cgo. It permits to create a static executable, without external dependecies,  that can be used everywhere without a local compiling. The only limitation is the machine word length and processor architecture (i.e. 64 bits executable won't work on a 32 bits machine and so on).  
