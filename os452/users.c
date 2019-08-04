/*
** SCCS ID: @(#)users.c 1.1 4/2/19
**
** File:    users.c
**
** Author:  Warren R. Carithers and various CSCI-452 classes
**
** Contributor: Oscar Onyeke Josh Bicking
**
** Description:	user routines
*/

#include "common.h"
#include "cio.h"
#include "lib.h"
#include "users.h"
#include "support.h"
/*
** USER PROCESSES
**
** Each is designed to test some facility of the OS; see the user.h
** header file for a summary of which system calls are tested by 
** each user function.
**
** Output from user processes is always alphabetic.  Uppercase 
** characters are "expected" output; lowercase are "erroneous"
** output.
**
** More specific information about each user process can be found in
** the header comment for that function (below).
**
** To spawn a specific user process, uncomment its SPAWN_x
** definition in the user.h header file.
*/

/*
** Prototypes for all one-letter user main routines (even
** ones that may not exist, for completeness)
*/

int user_a( int, void *, void * ); int user_b( int, void *, void * );
int user_c( int, void *, void * ); int user_d( int, void *, void * );
int user_e( int, void *, void * ); int user_f( int, void *, void * );
int user_g( int, void *, void * ); int user_h( int, void *, void * );
int user_i( int, void *, void * ); int user_j( int, void *, void * );
int user_k( int, void *, void * ); int user_l( int, void *, void * );
int user_m( int, void *, void * ); int user_n( int, void *, void * );
int user_o( int, void *, void * ); int user_p( int, void *, void * );
int user_q( int, void *, void * ); int user_r( int, void *, void * );
int user_s( int, void *, void * ); int user_t( int, void *, void * );
int user_u( int, void *, void * ); int user_v( int, void *, void * );
int user_w( int, void *, void * ); int user_x( int, void *, void * );
int user_y( int, void *, void * ); int user_z( int, void *, void * );

int user_mq_a( int, void *, void * ); int user_mq_b( int, void *, void * );
int user_mq_c( int, void *, void * ); int user_mq_d( int, void *, void * );
int user_mq_e( int, void *, void * ); int user_mq_f( int, void *, void * );
int user_mq_g( int, void *, void * ); int user_mq_h( int, void *, void * );
int user_mq_i( int, void *, void * ); int user_mq_j( int, void *, void * );
int user_mq_k( int, void *, void * );

/*
** Users A, B, and C are identical, except for the character they
** print out via write().  Each prints its ID, then loops 30
** times delaying and printing, before exiting.  They also verify
** the status return from write().
*/

int user_a( int argc, void *arg1, void *arg2 ) {
    int n;
    char buf[128];

    char ch = argc > 0 ? (int) arg1 : 'A';

    n = swritech( ch );
    if( n != 1 ) {
        sprint( buf, "User A, write 1 returned %d\n", n );
        cwrites( buf );
    }
    for( int i = 0; i < 30; ++i ) {
        DELAY(STD);
        n = swritech( ch );
        if( n != 1 ) {
            sprint( buf, "User A, write 2 returned %d\n", n );
            cwrites( buf );
        }
    }

    exit( 0 );

    n = write( STR_SIO, "*A*", 3 );    /* shouldn't happen! */
    if( n != 3 ) {
        sprint( buf, "User A, write 3 returned %d\n", n );
        cwrites( buf );
    }

    return( 0 );  // shut the compiler up!
}

int user_b( int argc, void *arg1, void *arg2 ) {
    int n;
    char buf[128];

    char ch = argc > 0 ? (int) arg1 : 'B';

    n = swritech( ch );
    if( n != 1 ) {
        sprint( buf, "User B, write 1 returned %d\n", n );
        cwrites( buf );
    }
    for( int i = 0; i < 30; ++i ) {
        DELAY(STD);
        n = swritech( ch );
        if( n != 1 ) {
            sprint( buf, "User B, write 2 returned %d\n", n );
            cwrites( buf );
        }
    }

    exit( 0 );

    n = write( STR_SIO, "*B*", 3 );    /* shouldn't happen! */
    if( n != 3 ) {
        sprint( buf, "User B, write 3 returned %d\n", n );
        cwrites( buf );
    }

    return( 0 );  // shut the compiler up!
}

int user_c( int argc, void *arg1, void *arg2 ) {
    int n;
    char buf[128];

    char ch = argc > 0 ? (int) arg1 : 'C';

    n = swritech( ch );
    if( n != 1 ) {
        sprint( buf, "User C, write 1 returned %d\n", n );
        cwrites( buf );
    }
    for( int i = 0; i < 30; ++i ) {
        DELAY(STD);
        n = swritech( ch );
        if( n != 1 ) {
            sprint( buf, "User C, write 2 returned %d\n", n );
            cwrites( buf );
        }
    }

    exit( 0 );

    n = write( STR_SIO, "*C*", 3 );    /* shouldn't happen! */
    if( n != 3 ) {
        sprint( buf, "User C, write 3 returned %d\n", n );
        cwrites( buf );
    }

    return( 0 );  // shut the compiler up!
}

/*
** User D spawns user Z, then exits before it can terminate.
*/

int user_d( int argc, void *arg1, void *arg2 ) {
    int32_t whom;
    int ret = 0;
    char buf[128];

    char ch = argc > 0 ? (int) arg1 : 'D';

    swritech( ch );

    // reset our priority to USER_H

    uint8_t old = getprio( -1 );
    (void) setprio( -1, PRIO_USER_H );
    uint8_t new = getprio( -1 );
    sprint( buf, "user D prio %d -> %d\n", old, new );
    cwrites( buf );

    // we spawn user_z at standard priority so that we can
    // finish before it is dispatched
    whom = spawn( user_z, PRIO_USER_S, 2, (void *) 'Z', (void *) '^' );
    if( whom < 0 ) {
        sprint( buf, "User D spawn() failed, returned %d\n", whom );
        cwrites( buf );
        ret = 1;
    }

    write( STR_SIO, &ch, 1 );

    exit( ret );

    return( 0 );  // shut the compiler up!
}

/*
** Users E, F, and G test the sleep facility.
**
** User E sleeps for 5 seconds at a time.
*/

int user_e( int argc, void *arg1, void *arg2 ) {
    char ch = argc > 0 ? (int) arg1 : 'E';

    report( 'E', getpid() );
    write( STR_SIO, &ch, 1 );
    for( int i = 0; i < 10 ; ++i ) {
        sleep( SEC_TO_MS(5) );
        write( STR_SIO, &ch, 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User F sleeps for 10 seconds at a time.
*/

int user_f( int argc, void *arg1, void *arg2 ) {
    char ch = argc > 0 ? (int) arg1 : 'F';

    report( 'F', getpid() );
    write( STR_SIO, &ch, 1 );
    for( int i = 0; i < 10 ; ++i ) {
        sleep( SEC_TO_MS(10) );
        write( STR_SIO, &ch, 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User G sleeps for 15 seconds at a time.
*/

int user_g( int argc, void *arg1, void *arg2 ) {
    char ch = argc > 0 ? (int) arg1 : 'G';

    report( 'G', getpid() );
    write( STR_SIO, &ch, 1 );
    for( int i = 0; i < 10 ; ++i ) {
        sleep( SEC_TO_MS(15) );
        write( STR_SIO, &ch, 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User H is like A-C except it only loops 10 times and doesn't
** call exit().  It also returns a non-zero status.
*/

int user_h( int argc, void *arg1, void *arg2 ) {
    char ch[100];
    char buf[128];
    int n = 0;
    int len = 0;

    __cio_printf("\n");
    sleep(SEC_TO_MS(10));
    for( ; ;) {
        __cio_printf("> ");
        __cio_gets(buf,128);
        len = __strlen(buf);
        buf[len-1] = '\0';
        // creates a new directory
        if(__strcmp(buf,"mkdir")==0){
            mkdir(STR_CONS,&ch,128);
            if (0 >= n){
                __cio_printf("mkdir failed.\n");
            }
        }
            // creates a new file
        else if(__strcmp(buf,"mkf")==0){
            n = create(STR_CONS,&ch,128);
            if (0 >= n){
                __cio_printf("mkf failed\n");
            }
        }
            // reads a file
        else if(__strcmp(buf,"read")==0){
            n = fread(STR_CONS,&ch,128);
            if (0 >= n){
                __cio_printf("read failed.\n");
            }
        }
            // writes to a file
        else if(__strcmp(buf,"write")==0){
            n = fwrite(STR_CONS,&ch,128);
            if (0 >= n){
                __cio_printf("write failed\n");
            }
        }
            // lists the contents of a directory
        else if(__strcmp(buf,"ls")==0){
            n = ls(STR_CONS,&ch,128);
            if (0 >= n){
                __cio_printf("ls failed.\n");
            }
        }
            // removes a file or a directory
        else if(__strcmp(buf,"rm")==0){
            n = rm(STR_CONS,&ch,128);
            if (0 >= n){
                __cio_printf("rm failed\n");
            }
        }
            // exits the file system
        else if(__strcmp(buf,"exit")==0){
            exit(0);
            return(0);
        }
            // lists the commands of the file system
        else if(__strcmp(buf,"help")==0){
            __cio_printf("Make a directory : mkdir\nCreate a file : mkf\n");
            __cio_printf("Write to a file : write\nRead a file : read\n");
            __cio_printf("Remove a directory/file : rm\nlist the contents of a directory : ls\n");
        }
        else{
            // incorrect input
            if (__strcmp(buf,"")==0 || __strcmp(buf," ")==0)
            __cio_printf("Unrecogizable input\n");
        }
    }

    exit(0);
    return(0);
}

/*
** User J tries to spawn 2*MAX_PROCS copies of user_y.
*/

int user_j( int argc, void *arg1, void *arg2 ) {
    char ch  = argc > 0 ? (int) arg1 : 'J';
    char ch2 = argc > 1 ? (int) arg2 : 'j';

    write( STR_SIO, &ch, 1 );

    for( int i = 0; i < (MAX_PROCS * 2) ; ++i ) {
        int32_t whom = spawn( user_y, PRIO_USER_S, 2, (void *) 'Y', (void *) i );
        if( whom < 0 ) {
            write( STR_SIO, &ch2, 1 );
        } else {
            write( STR_SIO, &ch, 1 );
        }
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User K prints, goes into a loop which runs five times, and exits.
** In the loop, it does a spawn of user_x, sleeps 30 seconds, and prints.
** It gives user_x() numbers in the 0..4 range.
*/

int user_k( int argc, void *arg1, void *arg2 ) {
    char ch  = argc > 0 ? (int) arg1 : 'K';
    char ch2 = argc > 1 ? (int) arg2 : 'k';

    write( STR_SIO, &ch, 1 );

    for( int i = 0; i < 5 ; ++i ) {
        write( STR_SIO, &ch, 1 );
        int32_t whom = spawn( user_x, PRIO_USER_S, 2, (void *) 'X', (void *) i );
        if( whom < 0 ) {
            swritech( ch2 );
        }
        sleep( SEC_TO_MS(30) );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User L is like user K, except that it prints times and doesn't sleep
** each time, it just preempts itself.  It also gives user_x() numbers
** in the 100..104 range.
*/

int user_l( int argc, void *arg1, void *arg2 ) {
    uint32_t now;
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'L';
    char ch2 = argc > 1 ? (int) arg1 : 'l';

    now = time();
    sprint( buf, "User L running, initial time %d\n", now );
    cwrites( buf );

    write( STR_SIO, &ch, 1 );

    for( int i = 0; i < 5 ; ++i ) {
        write( STR_SIO, &ch, 1 );
        int32_t whom = spawn( user_x, PRIO_USER_S, 2, (void *) 'X', (void *) i + 100 );
        if( whom < 0 ) {
            swritech( ch2 );
        } else {
            // yield, but don't sleep
            sleep( 0 );
        }
    }

    now = time();
    sprint( buf, "User L exiting, time %d\n", now );
    cwrites( buf );

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User M iterates spawns five copies of user W, reporting their PIDs.
*/

int user_m( int argc, void *arg1, void *arg2 ) {
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'M';
    char ch2 = argc > 1 ? (int) arg2 : 'm';

    write( STR_SIO, &ch, 1 );

    for( int i = 0; i < 5; ++i ) {
        write( STR_SIO, &ch, 1 );
        int32_t whom = spawn( user_w, PRIO_USER_H, 2, (void *) 'W', (void *) i + 10 );
        if( whom < 0 ) {
            swritech( ch2 );
        } else {
            sprint( buf, "M spawned W, PID %d\n", whom );
            cwrites( buf );
        }
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User N is like user M, except that it spawns user W and user Z
** at lower priority
*/

int user_n( int argc, void *arg1, void *arg2 ) {
    int32_t whom;
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'N';
    char ch2 = argc > 1 ? (int) arg2 : 'n';

    write( STR_SIO, &ch, 1 );

    for( int i = 0; i < 5; ++i ) {
        write( STR_SIO, &ch, 1 );
        whom = spawn( user_w, PRIO_USER_S, 2, (void *) 'W', (void *) i + 1000 );
        if( whom < 0 ) {
            swritech( ch2 );
        } else {
            sprint( buf, "User N spawned W, PID %d\n", whom );
            cwrites( buf );
        }

        whom = spawn( user_z, PRIO_USER_S, 1, (void *) 'Z', (void *) 0 );
        if( whom < 0 ) {
            swritech( ch2 );
        } else {
            sprint( buf, "User N spawned Z, PID %d\n", whom );
            cwrites( buf );
        }
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User P iterates three times.  Each iteration sleeps for two seconds,
** then gets and prints the system time.
*/

int user_p( int argc, void *arg1, void *arg2 ) {
    uint32_t now;
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'P';

    now = time();
    sprint( buf, "User P running, start at %d\n", now );
    cwrites( buf );

    write( STR_SIO, &ch, 1 );

    for( int i = 0; i < 3; ++i ) {
        sleep( SEC_TO_MS(2) );
        now = time();
        sprint( buf, "User P reporting time %d\n", now );
        cwrites( buf );
        write( STR_SIO, &ch, 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User Q does a bogus system call
*/

int user_q( int argc, void *arg1, void *arg2 ) {
    char ch = argc > 0 ? (int) arg1 : 'Q';

    write( STR_SIO, &ch, 1 );
    bogus();

    cwrites( "User Q returned from bogus syscall!?!?!\n" );

    exit( 1 );

    return( 0 );  // shut the compiler up!
}


/*
** User R loops forever reading/writing
*/

int user_r( int argc, void *arg1, void *arg2 ) {
    char buf[128];
    char b2[8];

    char ch  = argc > 0 ? (int) arg1 : 'R';

    b2[0] = ch;
    write( STR_SIO, b2, 1 );

    sleep( SEC_TO_MS(10) );

    for(;;) {
        int32_t n = read( STR_SIO, &b2[1], 1 );
        if( n != 1 ) {
            sprint( buf, "User R, read returned %d\n", n );
            cwrites( buf );
            if( n == -1 ) {
                // wait a bit
                sleep( SEC_TO_MS(1) );
            }
        } else {
            write( STR_SIO, b2, 2 );
        }
    }

    cwrites( "User R exiting!?!?!?\n" );
    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User S sleeps for 20 seconds at a time, and loops forever.
*/

int user_s( int argc, void *arg1, void *arg2 ) {
    char ch  = argc > 0 ? (int) arg1 : 'S';
    char ch2 = argc > 1 ? (int) arg2 : 's';

    write( STR_SIO, &ch, 1 );
    for(;;) {
        sleep( SEC_TO_MS(20) );
        write( STR_SIO, &ch, 1 );
    }

    cwrites( "User S exiting!?!?!\n" );
    swritech( ch2 );
    exit( 1 );

    return( 0 );  // shut the compiler up!

}

/*
** User T iterates three times, spawning copies of user W; then it
** sleeps for eight seconds, and then waits for those processes.
*/

int user_t( int argc, void *arg1, void *arg2 ) {
    int32_t whom[3];
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'T';
    char ch2 = argc > 1 ? (int) arg2 : 't';

    write( STR_SIO, &ch, 1 );

    for( int i = 0; i < 3; ++i ) {
        whom[i] = spawn( user_w, PRIO_USER_S, 2, (void *) 'W', (void *) i+20 );
        if( whom[i] < 0 ) {
            swritech( ch2 );
        } else {
            swritech( ch );
        }
    }

    sleep( SEC_TO_MS(8) );

    // collect exit status information

    do {
        int32_t this;
        int32_t exitstatus;
        this = wait( &exitstatus );
        if( this < 0 ) {
            if( this != E_NO_CHILDREN ) {
                sprint( buf, "User T: wait() status %d\n", this );
                cwrites( buf );
            }
            break;
        }
        sprint( buf, "User T: child PID %d status %d\n", this, exitstatus );
        cwrites( buf );
    } while( 1 );

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** User U is like user T, except that doesn't collect its children
*/

int user_u( int argc, void *arg1, void *arg2 ) {
    int32_t whom[3];

    char ch  = argc > 0 ? (int) arg1 : 'U';
    char ch2 = argc > 1 ? (int) arg2 : 'u';

    write( STR_SIO, &ch, 1 );

    for( int i = 0; i < 3; ++i ) {
        whom[i] = spawn( user_w, PRIO_USER_S, 2, (void *) 'W', (void *) i+20 );
        if( whom[i] < 0 ) {
            swritech( ch2 );
        } else {
            swritech( ch );
        }
    }

    sleep( SEC_TO_MS(8) );

    exit( 0 );

    return( 0 );  // shut the compiler up!
}

/*
** Users W through Z are spawned by other user processes.  Each
** prints its arguments to the SIO on each iteration.
*/

/*
** User W prints W characters 20 times, sleeping 3 seconds between.
*/

int user_w( int argc, void *arg1, void *arg2 ) {
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'W';

    report( 'W', getpid() );

    sprint( buf, " %c[%d/%d/%d] ", ch, argc, arg1, arg2 );

    for( int i = 0; i < 20 ; ++i ) {
        swrites( buf );
        sleep( SEC_TO_MS(3) );
    }


    cwrites( buf );
    cwrites( " exiting\n" );

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User X prints X characters 20 times.  It is spawned multiple
** times, prints its PID when started and exiting, and exits
** with a non-zero status
*/

int user_x( int argc, void *arg1, void *arg2 ) {
    int32_t whom;
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'X';

    report( 'X', whom=getpid() );

    sprint( buf, " %c[%d/%d/%d] ", ch, argc, arg1, arg2 );

    for( int i = 0; i < 20 ; ++i ) {
        swrites( buf );
        DELAY(STD);
    }

    cwrites( buf );
    cwrites( " exiting\n" );

    exit( whom ? whom : -98765 );

    return( 0 );  // shut the compiler up!
}

/*
** User Y prints Y characters 10 times.
*/

int user_y( int argc, void *arg1, void *arg2 ) {
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'Y';

    sprint( buf, " %c[%d/%d/%d] ", ch, argc, arg1, arg2 );

    for( int i = 0; i < 10 ; ++i ) {
        swrites( buf );
        DELAY(STD);
        sleep( SEC_TO_MS(1) );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User Z prints Z characters 10 times.  Before it exits, its parent
** may have exited; it reports this so that we can verify reparenting.
*/

int user_z( int argc, void *arg1, void *arg2 ) {
    int32_t me, parent;
    char buf[128];

    char ch  = argc > 0 ? (int) arg1 : 'Z';

    sprint( buf, " %c[%d/%d/%d] ", ch, argc, arg1, arg2 );

    me = getpid();
    parent = getppid();

    char buf2[128];

    sprint( buf2, "user %s (%d) running, parent %d\n", buf, me, parent );
    cwrites( buf2 );

    for( int i = 0; i < 10 ; ++i ) {
        swrites( buf );
        DELAY(STD);
    }

    // get "new" parent PID
    int32_t parent2 = getppid();

    sprint( buf2, "user %s (%d) exiting, parent now %d (was %d)\n",
            buf, me, parent2, parent );
    cwrites( buf2 );

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User MQ_A tests basic message queue functionality. Open an MQ, send a
** character, receive a character, close the MQ.
*/
int user_mq_a( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'A';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_%c", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    char msg[2];
    sprint(msg, "%c", ch);
    //sprint(buf, "Writing %s to %s\n", msg, mq_name);
    //cwrites(buf);
    int status = mq_write(mqd, (unsigned char *) msg, 2, 4, false);

    if (status < 0) {
        sprint(buf, "%s write failed, got %d !?!?!?\n", mq_name, status);
        cwrites( buf );
        exit( 1 );
    }

    char mailbox[2];
    //sprint(buf, "Reading from %s.. ", mq_name);
    //cwrites(buf);
    int read = mq_read(mqd, (unsigned char *) mailbox, 2, 4, false);
    if (read <= 0) {
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }
    //sprint(buf, "Read %d bytes, got %s\n", read, mailbox);
    //cwrites(buf);

    if (strcmp(msg, mailbox) != 0) {
        sprint(buf, "Message was modified in transit! Sent %s got %s !?!?!?\n", msg, mailbox);
        cwrites( buf );
        exit( 1 );
    }
    status = mq_close(mqd);
    if (status != 0) {
        sprint(buf, "MQ_%c opening failed!?!?!?\n", ch);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User MQ_B open an MQ and sends a message for user MQ_C to read.
*/

int user_mq_b( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'B';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_B_C", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    char msg[24];
    sprint(msg, "Henlo from USER MQ_B");
    //sprint(buf, "Writing '%s' to %s\n", msg, mq_name);
    //cwrites(buf);
    int status = mq_write(mqd, (unsigned char *) msg, 24, 4, false);
    if (status < 0) {
        sprint(buf, "%s write failed, got %d !?!?!?\n", mq_name, status);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

int user_mq_c( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'C';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_B_C", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    char mailbox[24];
    //sprint(buf, "Reading from %s.. ", mq_name);
    //cwrites(buf);
    int read = mq_read(mqd, (unsigned char *) mailbox, 24, 4, false);
    if (read <= 0) {
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }
    //sprint(buf, "Read %d bytes, got %s\n", read, mailbox);
    //cwrites(buf);

    if (strcmp("Henlo from USER MQ_B", mailbox) != 0) {
        sprint(buf, "Message was modified in transit! Got %s !?!?!?\n", mailbox);
        cwrites( buf );
        exit( 1 );
    }
    int16_t status = mq_close(mqd);
    if (status != 0) {
        sprint(buf, "MQ_B_C closing failed!?!?!? %d\n", status);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User MQ_D tests opening/closing of mqueues.
*/
int user_mq_d( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'D';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqds[4];
    char mq_name1[8];
    char mq_name2[8];
    char mq_name3[8];
    char mq_name4[8];
    char * mq_names[4] = {mq_name1, mq_name2, mq_name3, mq_name4};
    for (int16_t i=0; i<4; i++) {
        sprint(mq_names[i], "MQ_%c%d", ch, i);
        //sprint(buf, "Opening %s\n", mq_names[i]);
        //cwrites(buf);
        mqds[i] = mq_open(mq_names[i]);
        //sprint(buf, "Got mqd of %d\n", mqds[i]);
        //cwrites(buf);

        if (mqds[i] <= 0) {
            sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_names[i], mqds[i]);
            cwrites( buf );
            exit( 1 );
        }
    }

    int16_t status;
    for (int16_t i=0; i<4; i++) {
        status = mq_close(mqds[i]);
        if (status != 0) {
            sprint(buf, "MQ_%c%d closing failed!?!?!?\n", ch, i);
            cwrites( buf );
            exit( 1 );
        }
    }

    // Ensure MQDs are deleted from the PCB, and we don't get an error here.
    for (int16_t i=0; i<4; i++) {
        sprint(mq_names[i], "MQ_%c%d", ch, i);
        //sprint(buf, "Opening %s\n", mq_names[i]);
        //cwrites(buf);
        mqds[i] = mq_open(mq_names[i]);
        //sprint(buf, "Got mqd of %d\n", mqds[i]);
        //cwrites(buf);

        if (mqds[i] <= 0) {
            sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_names[i], mqds[i]);
            cwrites( buf );
            exit( 1 );
        }

        status = mq_close(mqds[i]);
        if (status != 0) {
            sprint(buf, "MQ_%c%d closing failed!?!?!?\n", ch, i);
            cwrites( buf );
            exit( 1 );
        }
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User MQ_E reads before ME_F writes.
*/
int user_mq_e( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'E';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_E_F", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    char mailbox[24];
    //sprint(buf, "Reading from %s.. ", mq_name);
    //cwrites(buf);
    int read = mq_read(mqd, (unsigned char *) mailbox, 24, 4, true);
    if (read <= 0) {
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }
    //sprint(buf, "Read %d bytes, got %s\n", read, mailbox);
    //cwrites(buf);

    if (strcmp("Henlo from USER MQ_F", mailbox) != 0) {
        sprint(buf, "Message was modified in transit! Got %s !?!?!?\n", mailbox);
        cwrites( buf );
        exit( 1 );
    }
    int16_t status = mq_close(mqd);
    if (status != 0) {
        sprint(buf, "MQ_E_F closing failed!?!?!? %d\n", status);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

int user_mq_f( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'F';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_E_F", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    char msg[24];
    sprint(msg, "Henlo from USER MQ_F");
    //sprint(buf, "Writing '%s' to %s\n", msg, mq_name);
    //cwrites(buf);
    int16_t status = mq_write(mqd, (unsigned char *) msg, 24, 4, true);
    if (status < 0) {
        sprint(buf, "%s write failed, got %d !?!?!?\n", mq_name, status);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** Similar to E and F, user MQ_G writes MAX+1 messages to an mqueue, and must
** wait for MQ_H to read one.
*/
int user_mq_g( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'G';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_G_H", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    for (int i=0; i<MQ_MAX_MSGS+1; i++) {
        char msg[24];
        sprint(msg, "Henlo from USER MQ_G");
        //sprint(buf, "Writing '%s' to %s\n", msg, mq_name);
        //cwrites(buf);
        int status = mq_write(mqd, (unsigned char *) msg, 24, 4, true);
        if (status < 0) {
            sprint(buf, "%s write failed, got %d !?!?!?\n", mq_name, status);
            cwrites( buf );
            exit( 1 );
        }
    }
    int16_t status = mq_close(mqd);
    if (status != 0) {
        sprint(buf, "MQ_G_H closing failed!?!?!?\n", ch);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

int user_mq_h( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'H';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_G_H", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    char mailbox[24];
    //sprint(buf, "Reading from %s.. ", mq_name);
    //cwrites(buf);
    int read = mq_read(mqd, (unsigned char *) mailbox, 24, 4, true);
    if (read <= 0) {
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }
    //sprint(buf, "Read %d bytes, got %s\n", read, mailbox);
    //cwrites(buf);

    if (strcmp("Henlo from USER MQ_G", mailbox) != 0) {
        sprint(buf, "Message was modified in transit! Got %s !?!?!?\n", mailbox);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User MQ_I opens an mqueue, and blocks on it. While blocked, MG_J closes it.
*/
int user_mq_i( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'I';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_I_J", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    char mailbox[24];
    //sprint(buf, "Reading from %s.. ", mq_name);
    //cwrites(buf);
    int read = mq_read(mqd, (unsigned char *) mailbox, 24, 4, true);

    // MQ will close unexpectedly
    if (read != E_MQ_DNE) {
        sprint(buf, "%s: expecting return of %d, got %d !?!?!?\n",
               mq_name, E_MQ_DNE, read);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User MQ_J opens an mqueue and closes it.
*/
int user_mq_j( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'J';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_I_J", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    int16_t status = mq_close(mqd);
    if (status != 0) {
        sprint(buf, "MQ_I_J closing failed!?!?!? %d\n", status);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}

/*
** User MQ_K tests mqueue priorities.
*/
int user_mq_k( int argc, void *arg1, void *arg2 ) {

    char buf[128];
    char ch = argc > 0 ? (char)(int) arg1 : 'K';

    sprint( buf, " MQ_%c[%d/%d/%d] ", ch, argc, arg1, arg2 );
    swrites(buf);

    int16_t mqd;
    char mq_name[8];
    sprint(mq_name, "MQ_%c", ch);
    //sprint(buf, "Opening %s\n", mq_name);
    //cwrites(buf);
    mqd = mq_open(mq_name);
    //sprint(buf, "Got mqd of %d\n", mqd);
    //cwrites(buf);

    if (mqd <= 0) {
        sprint(buf, "%s opening failed, got %d !?!?!?\n", mq_name, mqd);
        cwrites( buf );
        exit( 1 );
    }

    // Write messages from lowest to highest priority
    char msg[2];
    int16_t status;
    for (int i=4; i>=1; i--) {
        sprint(msg, "%d", i);
        //sprint(buf, "Writing %s to %s\n", msg, mq_name);
        //cwrites(buf);
        status = mq_write(mqd, (unsigned char *) msg, 2, i, false);
        if (status < 0) {
            sprint(buf, "%s write failed, got %d !?!?!?\n", mq_name, status);
            cwrites( buf );
            exit( 1 );
        }
    }


    char mailbox[2];
    //sprint(buf, "Reading from %s.. ", mq_name);
    //cwrites(buf);

    /* Pull messages off in a different order */
    // - Prio 2 (get message 2)
    int16_t read;
    read = mq_read(mqd, (unsigned char *) mailbox, 2, 2, false);
    if (read <= 0) {
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }
    //sprint(buf, "Read %d bytes, got %s\n", read, mailbox);
    //cwrites(buf);
    if (strcmp("2", mailbox) != 0) {
        sprint(buf, "Wrong priority message recieved! Sent 2 got %s !?!?!?\n", mailbox);
        cwrites( buf );
        exit( 1 );
    }

    // - Prio 2 (get message 1)
    read = mq_read(mqd, (unsigned char *) mailbox, 2, 2, false);
    if (read <= 0) {
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }
    //sprint(buf, "Read %d bytes, got %s\n", read, mailbox);
    //cwrites(buf);
    if (strcmp("1", mailbox) != 0) {
        sprint(buf, "Wrong priority message recieved! Sent 1 got %s !?!?!?\n", mailbox);
        cwrites( buf );
        exit( 1 );
    }

    // - Prio 2 (get empty error)
    read = mq_read(mqd, (unsigned char *) mailbox, 2, 2, false);
    if (read != E_MQ_EMPTY) {
        sprint(buf, "%s: expecting return of %d, got %d !?!?!?\n",
               mq_name, E_MQ_EMPTY, read);
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }

    // - Prio 4 (get message 4)
    read = mq_read(mqd, (unsigned char *) mailbox, 2, 4, false);
    if (read <= 0) {
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }
    //sprint(buf, "Read %d bytes, got %s\n", read, mailbox);
    //cwrites(buf);
    if (strcmp("4", mailbox) != 0) {
        sprint(buf, "Wrong priority message recieved! Sent 4 got %s !?!?!?\n", mailbox);
        cwrites( buf );
        exit( 1 );
    }

    // - Prio 4 (get message 3)
    read = mq_read(mqd, (unsigned char *) mailbox, 2, 4, false);
    if (read <= 0) {
        sprint(buf, "%s read failed, got %d !?!?!?\n", mq_name, read);
        cwrites( buf );
        exit( 1 );
    }
    //sprint(buf, "Read %d bytes, got %s\n", read, mailbox);
    //cwrites(buf);
    if (strcmp("3", mailbox) != 0) {
        sprint(buf, "Wrong priority message recieved! Sent 4 got %s !?!?!?\n", mailbox);
        cwrites( buf );
        exit( 1 );
    }
    status = mq_close(mqd);
    if (status != 0) {
        sprint(buf, "MQ_%c opening failed!?!?!?\n", ch);
        cwrites( buf );
        exit( 1 );
    }

    exit( 0 );

    return( 0 );  // shut the compiler up!

}


/*********************
 ** SYSTEM PROCESSES **
 *********************/

/*
** Idle process
*/

int idle( int argc, void *arg1, void *arg2 ) {
    int32_t me;
    uint32_t now;
    char buf[128];

    char ch = argc > 0 ? (int) arg1 : '*';

    me = getpid();
    now = time();

    sprint( buf, "Idle [%d] started at %d\n", me, now );
    cwrites( buf );

    write( STR_SIO, &ch, 1 );

    for(;;) {
        DELAY(LONG);
        write( STR_SIO, &ch, 1 );
    }

    now = time();
    sprint( buf, "+++ Idle done at %d !?!?!\n", now );
    cwrites( buf );

    exit( 1 );

    return( 0 );  // shut the compiler up!
}

/*
** Initial process; it starts the other top-level user processes.
**
** Prints a message at startup, '+' after each user process is spawned,
** and '!' before transitioning to wait() mode to the SIO, and
** startup and transition messages to the console.  It also reports
** each child process it collects via wait() to the console along
** with that child's exit status.
*/

int init( int argc, void *arg1, void *arg2 ) {
    int32_t whom;
    char ch = '+';
    static int invoked = 0;
    char buf[128];

    if( invoked > 0 ) {
        cwrites( "Init RESTARTED???\n" );
        for(;;);
    }

    cwrites( "Init started\n" );
    ++invoked;

    // home up, clear
    swritech( '\x1a' );
    // wait a bit
    DELAY(STD);

    // a bit of Dante to set the mood
    swrites( "\n\nSpem relinquunt qui huc intrasti!\n\n\r" );

    whom = spawn( idle, PRIO_DEFERRED, 1, (void *) '.', (void *) 0 );
    if( whom < 0 ) {
        sprint( buf, "init, spawn() IDLE failed, status %d\n", whom );
        cwrites( buf );
    }

    swritech( ch );

#ifdef SPAWN_A
    whom = spawn( user_a, PRIO_USER_S, 2, (void *) 'A', (void *) 'a' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user A failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_B
    whom = spawn( user_b, PRIO_USER_S, 2, (void *) 'B', (void *) 'b' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user B failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_C
    whom = spawn( user_c, PRIO_USER_S, 2, (void *) 'C', (void *) 'c' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user C failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_D
    whom = spawn( user_d, PRIO_USER_H, 2, (void *) 'D', (void *) 'd' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user D failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_E
    whom = spawn( user_e, PRIO_USER_S, 2, (void *) 'E', (void *) 'e' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user E failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_F
    whom = spawn( user_f, PRIO_USER_S, 2, (void *) 'F', (void *) 'f' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user F failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_G
    whom = spawn( user_g, PRIO_USER_S, 2, (void *) 'G', (void *) 'g' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user G failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_H
    whom = spawn( user_h, PRIO_USER_S, 2, (void *) 'H', (void *) 'h' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user H failed\n" );
    }
    swritech( ch );
#endif

    // there is no user_i

#ifdef SPAWN_J
    whom = spawn( user_j, PRIO_USER_S, 2, (void *) 'J', (void *) 'j' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user J failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_K
    whom = spawn( user_k, PRIO_USER_S, 2, (void *) 'K', (void *) 'k' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user K failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_L
    whom = spawn( user_l, PRIO_USER_S, 2, (void *) 'L', (void *) 'l' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user L failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_M
    whom = spawn( user_m, PRIO_USER_S, 2, (void *) 'M', (void *) 'm' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user M failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_N
    whom = spawn( user_n, PRIO_USER_S, 2, (void *) 'N', (void *) 'n' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user N failed\n" );
    }
    swritech( ch );
#endif

    // there is no user_o

#ifdef SPAWN_P
    whom = spawn( user_p, PRIO_USER_S, 2, (void *) 'P', (void *) 'p' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user P failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_Q
    whom = spawn( user_q, PRIO_USER_S, 2, (void *) 'Q', (void *) 'q' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user Q failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_R
    whom = spawn( user_r, PRIO_USER_S, 2, (void *) 'R', (void *) 'r' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user R failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_S
    whom = spawn( user_s, PRIO_USER_S, 2, (void *) 'S', (void *) 's' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user S failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_T
    whom = spawn( user_t, PRIO_USER_S, 2, (void *) 'T', (void *) 't' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user T failed\n" );
    }
    swritech( ch );
#endif

#ifdef SPAWN_U
    whom = spawn( user_u, PRIO_USER_S, 2, (void *) 'U', (void *) 'u' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user U failed\n" );
    }
    swritech( ch );
#endif

    // there is no user_v
    // users w through z are spawned elsewhere

#ifdef SPAWN_MQ_A
    whom = spawn( user_mq_a, PRIO_USER_S, 2, (void *) 'A', (void *) 'a' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_A failed\n" );
    }
    swritech( ch );
#endif
#ifdef SPAWN_MQ_B_C
    whom = spawn( user_mq_b, PRIO_USER_S, 2, (void *) 'B', (void *) 'b' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_B failed\n" );
    }
    whom = spawn( user_mq_c, PRIO_USER_S, 2, (void *) 'C', (void *) 'c' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_C failed\n" );
    }
    swritech( ch );
#endif
#ifdef SPAWN_MQ_D
    whom = spawn( user_mq_d, PRIO_USER_S, 2, (void *) 'D', (void *) 'd' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_D failed\n" );
    }
    swritech( ch );
#endif
#ifdef SPAWN_MQ_E_F
    whom = spawn( user_mq_e, PRIO_USER_S, 2, (void *) 'B', (void *) 'b' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_E failed\n" );
    }
    whom = spawn( user_mq_f, PRIO_USER_S, 2, (void *) 'C', (void *) 'c' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_F failed\n" );
    }
    swritech( ch );
#endif
#ifdef SPAWN_MQ_G_H
    whom = spawn( user_mq_g, PRIO_USER_S, 2, (void *) 'G', (void *) 'g' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_G failed\n" );
    }
    whom = spawn( user_mq_h, PRIO_USER_S, 2, (void *) 'H', (void *) 'h' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_H failed\n" );
    }
    swritech( ch );
#endif
#ifdef SPAWN_MQ_I_J
    whom = spawn( user_mq_i, PRIO_USER_S, 2, (void *) 'I', (void *) 'i' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_I failed\n" );
    }
    whom = spawn( user_mq_j, PRIO_USER_S, 2, (void *) 'J', (void *) 'j' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_J failed\n" );
    }
    swritech( ch );
#endif
#ifdef SPAWN_MQ_K
    whom = spawn( user_mq_k, PRIO_USER_S, 2, (void *) 'K', (void *) 'k' );
    if( whom < 0 ) {
        cwrites( "init, spawn() user MQ_K failed\n" );
    }
    swritech( ch );
#endif

    swrites( "!\r\n\n" );

    /*
    ** At this point, we go into an infinite loop waiting
    ** for our children (direct, or inherited) to exit.
    */

    cwrites( "init() transitioning to wait() mode\n" );

    // first, we drop our priority a bit

    int32_t pid = getpid();
    uint32_t oldprio = setprio( -1, PRIO_USER_H );
    uint32_t newprio = getprio( -1 );
    sprint( buf, "init() [%d] dropped prio from %d to %d\n",
            pid, oldprio, newprio );
    cwrites( buf );

    for(;;) {
        int32_t status;
        int32_t whom = wait( &status );

        if( whom == E_NO_CHILDREN ) {
            // should never happen - idle() should always be there!
            cwrites( "INIT: wait() says 'no children'???\n" );
            continue;
        } else if( whom > 0 ) {
            sprint( buf, "INIT: pid %d exited, status %d\n", whom, status );
            cwrites( buf );
        } else {
            sprint( buf, "INIT: wait() status %d\n", whom );
            cwrites( buf );
        }
    }

    /*
    ** SHOULD NEVER REACH HERE
    */

    cwrites( "*** INIT IS EXITING???\n" );
    exit( 1 );

    return( 0 );  // shut the compiler up!
}
