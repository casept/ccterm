# ccterm: casept's crappy terminal emulator

This is an extremely barebones (as in "doesn't even handle backspace properly or scroll") terminal emulator I wrote for fun in about a day, mostly as an exercise in C++, SDL2 and a bit of Unix internals demystification.

## Building

I've only ever tested the code on NixOS with the build configuration described in `shell.nix`,
but it should (at least in theory) be portable to at least other Linux flavors, MacOS, and the BSD's.

The easy way to build is using a `nix-shell`:

```sh
$ nix-shell
[nix-shell:~/dev/ccterm]$ mkdir build && cd build
[nix-shell:~/dev/ccterm/build]$ cmake -G Ninja ../
[nix-shell:~/dev/ccterm/build]$ ninja
[nix-shell:~/dev/ccterm/build]$ ./ccterm
```

Alternatively, scrounge together the following dependencies by hand:

* Any reasonably C++20-compliant compiler
* `cmake` >= 3.19
* `fmt` >= 7.1.3
* `SDL2` >= 2.0.14
* `SDL2_ttf` >= 2.0.15

Earlier dependency versions may work, but are entirely untested.
