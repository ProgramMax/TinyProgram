# TinyProgram

Modern software is often slow and bloated.
TinyProgram is an example program to move in the opposite direction.
It can be used as a template.

![Example screenshot on Win98](Win98%20screenshot.png)

## Stipulations

* The program must run. (The OS must be able to run the program. Being technically valid but practically invalid isn't sufficient.)
* The program must be usable. (If the program can only return 0, that's not useful.)
* Automatic rebuild tooling must exist. (Manually hex editing is not sufficient.)

These stipulations rule out several techniques.
People have managed to get a Windows program down to just over a hundred bytes. But those programs won't run, for example.