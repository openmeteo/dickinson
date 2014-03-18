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

Download the DLL from
http://github.com/openmeteo/dickinson/releases/latest/, rename it to
``dickinson.dll`` and put it in ``C:\Windows\System32`` or another
directory in the path. Note that we only make 32-bit DLLs.

Compiling on Windows
--------------------

Dickinson has only been tested in 32-bit Windows.

Requirements:

* `MinGW compiler with MSYS shell`_ (click the "Download Installer"
  button on the right; run the downloaded installer and ask it to
  install itself; I usually don't install the UI, only the CLI; then I
  go to :file:`C:\\MinGW\\bin` and tell it ``mingw-get install
  mingw32-gcc`` and ``mingw-get install msys``; note: to uninstall
  MinGW just delete the directory :file:`C:\\MinGW`).

Put a copy of dickinson inside the MSYS shell user's home directory
(something like :file:`C:\\MinGW\\msys\\1.0\\home\\user`), then start the
MSYS shell from :file:`C:\\MinGW\\msys\\1.0\\msys.bat`. Then::

   export PATH=/c/MinGW/bin:$PATH
   cd dickinson
   CC="gcc -static-libgcc" ./configure
   make

This will leave a :file:`libdickinson-0.dll` file in the
:file:`dickinson/src/.libs` directory; rename it to
:file:`dickinson.dll`.

.. _MinGW compiler with MSYS shell: http://mingw.org/
