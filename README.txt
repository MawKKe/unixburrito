unixburrito
===========

...is funny name for a simple thing: wrap ordinary C systems programming interfaces into typechecked
C++ interfaces. This is done to prevent accidental bugs from creeping into you code base.

For example, many of the interfaces take flags in the form of 'int's. Flag values can be created
by bitwise-OR'in different symbols together. These symbols are usully just #define'd constants.
The problem with this method is that the symbols have no type, and they cannot be type checked.
You might accidentally combine together symbols that together make no semantic sense. This mistake
is easy to make, and a very real source of silent bugs. 

This wrapper library tries to prevent such semantic bugs by introducing the original values with
better types. This is accomplished via the usage of C++ enum classes. As-is they are not perfect,
or ideal, but a whole lot better than the old interfaces. 

This library does not invent new functionality. It gives you the same old interfaces, with same 
old sematics, but with better types. The power and usefulness of the old interfaces should still 
exist, even if you decide to use this wrapper library.

There are also few helper functions, that attempt to implement the "common usage" for you.

Well designed programming interfaces prevent you from making stupid mistakes. Well designed 
interfaces provide easy access to the most common usage patterns. Well designed interfaces do not
prevent you from fully utilizing all the possible features in the name of "security". 

The traditional C-style systems programming interfaces focus only on the last part, and it seems
the first two have been forgotten entirely. This is mainly due to the limitations of C, and partly
because the systems interfaces are such low-level interfaces anyway (and whoever uses them clearly
should know how to read man pages!!!!). But to be honest, I've lost count how many times I've had 
to go through the SAME manual pages over and over again, with a fine tooth comb, to find out what 
*exactly* did each interface require, which fields do I need to initialize, which values are 
suitable for which, what are the return values and .... ARGGHGGHGHHGG.

So I decided to utilize the powers of C++, to automate a lot of this stuff. Instead of YOU, 
the programmer, most of the tedious, boring tasks are left in the hands of the compiler. Like 
it should have been from the start.

I don't know if this kind of wrappers exist, or at least I could not find any that did what I
wanted them to do.

To use this library, you need fairly new compiler with C++14 support. The oldest system I 
tried this on was Ubuntu 16.04 with gcc version 5.4.0. The code uses some newer features,
such as std::optional (std::experimental::optional with C++14) which might not be properly 
supported by all compilers.

Cheers,

Markus

---

Usage
------
To compile the example program, run:

    $ mkdir build && cd build
    $ cmake ..
    $ make


The topmost namespace is called _unix (originally it was just 'unix', but I figured this is
a common name, and it may cause colisions). If you want an alias for this name, you can do 
something like this:

    namespace foo = _unix;

..put this wherever you think is the most practical place. 

There are internal namespaces to contain certain kinds of functionalities. At the moment there are:

- _unix::signals    For various functionalities revolving around *nix signals (man 7 signal)
- _unix::inet       Various functions and classes for socket programming


---

Limitation of enum classes
---------------------------

The new (C++11) enum class feature takes the ordinary enums into the direction of algrebraic types.
However, the enum classes are still bound to an underlying integral type. The enum class values will
are not implicitly converted to integers and back, but can be if required. This makes enum class
values "more symbolic" than previous enums. Danger lies with them, any symbolic type safety can be
overrided with simple static_cast<Foo>(). Also, there is no way to check wether an integral value
is an actual symbol. Although you can check wether the value is between the min and max values of the
integral range, the range may have gaps, which means you must keep manual track of all the possible
symbols...

---

License
-------

unixburrito is released under the MIT license.

A copy of the license is found in file LICENSE.txt at the root of the project tree.

