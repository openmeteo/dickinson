Dickinson is a C library containing various utilities, mainly for time
series operations. The Python pthelma library is partly a front-end to
Dickinson.

Here's a simple example::

    #include <stdio.h>
    #include <ts.h>

    int main(int argc, char **argv) {
        struct timeseries *a_timeseries;
        FILE *fp;
        int errline;
        char *errstr;

        /* Create an empty timeseries */
        a_timeseries = ts_create();

        /* Read timeseries from a file */
        fp = fopen("/tmp/myfile.txt", "r");
        if(ts_readfile(fp, a_timeseries, &errline, &errstr)) {
            fprintf("Error in line %d of the file: %s\n", errline, errstr);
            exit(1);
        }
        fclose(fp);

        /* Free the timeseries. */
        ts_free(a_timeseries);
    }

It contains functionality to add records, remove records, merge
timeseries, find records with specific timestamps, and all that,
handling memory allocation automatically. The library also contains
some useful CSV functionality.

For more information, see the documentation in the ``doc`` subdirectory
or online_.

.. _online: http://dickinson.readthedocs.org/
