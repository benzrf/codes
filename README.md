Ben R.F.'s Final Project for Information Theory
===============================================

Usage
-----

To build, just run `make`. To use, run `./code <subcommand>`. Run
without arguments for a list of subcommands.

Example usage:

    $ cat ../../data/mobydick.txt | ./code encode | ./biterror.py 14 | ./code decode > /tmp/mobynew.txt
    $ diff ../../data/mobydick.txt /tmp/mobynew.txt

Info
----

Check the source for fairly extensive comments. I hope it's enough!

I've used Valgrind to experimentally verify that there are no overruns
or leaks in this code—or at least, none that manifest themselves on
valid input...

One warning: The LZW implementation uses fixed-size words to store
indices in memory, so it would probably segfault if used on
sufficiently enormous data. Fortunately, I doubt anybody plans on
trying to use this for compressing multi-exabyte files :)

