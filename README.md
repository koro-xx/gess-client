#Gess client

A client for the Gess game (a chess-like game played on a Go board) using irc for communication.
The rules of the game can be found in [this link](http://www.archim.org.uk/eureka/53/gess.html).
Work in progress.

Licensed under the GPLv3.

<p align="center">
<img src="https://github.com/koro-xx/gess-client/blob/master/extra/gess-v0.01-screen.png" /> </p>

##Dependencies:

Requires the following libraries:
Allegro >= 5.2
libircclient

Also uses SiegeLord's WidgetZ GUI library (included in source with a minor modification).

## Build instructions

A makefile is included that should work fine on a linux system.
First install Allegro 5 and libircclient, then run make from the gess-client dir. It assumes both Allegro and libircclient are on the default locations (/usr/lib), and assumes gcc is your compiler. Change the first few lines of the makefile otherwise.

There is no install rule. Just run the executable from the same dir.

Alternatively, use cmake:

	cd gess-client && mkdir build && cd build
	cmake ..
	make

If Allegro or Ircclient are not found, you can add -DVAR=path when calling cmake, where VAR can be:

	Alleg_ROOT: path to a dir containing include/allegro5 and lib/liballeg*
	IRC_CLIENT_INCLUDE_DIR: path to a dir contianing the libircclient headers.
	IRC_CLIENT_LIBRARY: path to the ircclient library (probably libircclient.so)
