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

Windows binaries
----------------

Download the DLL from http://openmeteo.org/downloads/, rename it to
``dickinson.dll`` and put it in ``C:\Windows\System32``. Note that we
only make 32-bit DLLs.

Compiling on Windows
--------------------

Dickinson has only been tested in 32-bit Windows.

Requirements:

* `MinGW compiler with MSYS shell`_ (go to Downloads, Installer,
  mingw-get-inst, and download the latest ``min-get-inst`` executable;
  run it and tell it to install itself, making sure to include the
  MSYS shell).

Put a copy of dickinson inside the MSYS shell user's home directory
(something like ``C:\MinGW\msys\1.0\home\user``), then start the MSYS
shell from the Windows menu, ``MinGW`` program folder. Then::

   cd dickinson
   ./configure
   make

This will leave a ``libdickinson-0.dll`` file in the
``dickinson\src\.libs`` directory; copy it to
``C:\Windows\System32\dickinson.dll``.

.. _MinGW compiler with MSYS shell: http://mingw.org/
