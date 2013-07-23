.. _install:

============
Installation
============

Unix
----

Requirements:

* GNU C compiler (gcc)
* GNU make

Compilation instructions::

   ./configure
   make

Among others, this will create a ``src/.libs`` subdirectory with the
compiled library. You may then run "sudo make install" to install it
system-wide, or set the ``LD_LIBRARY_PATH`` environment variable to
include the ``src/.libs`` subdirectory in the library search path.
For example, with ``bash``::

    export LD_LIBRARY_PATH=`pwd`/src/.libs

but you need to type this every time you run a shell.
Instead of that, what I actually do is prefix each command that
needs it; for example, to run a Django development server::

    LD_LIBRARY_PATH=../dickinson/src/.libs ./manage.py runserver

This has the added benefit that it does not get confused if I have
many instances of Dickinson on my system.

Windows
-------

It's currently broken and doesn't compile in Windows. If you are an
``autoconf`` expert, you might be able to help. We just can't
determine why `it doesn't create a DLL`_.

.. _it doesn't create a DLL: http://stackoverflow.com/questions/17813748/how-can-i-tell-autoconf-to-create-a-windows-dll
