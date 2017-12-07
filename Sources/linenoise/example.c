#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>
#include "linenoise.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>



void completion(const char *buf, linenoiseCompletions *lc) {
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc,"hello");
        linenoiseAddCompletion(lc,"hello there");
    }
    else if (buf[0] == '/') {
        linenoiseAddCompletion(lc,"/#");
        linenoiseAddCompletion(lc,"/bash");
        linenoiseAddCompletion(lc,"/bash_cmd");
        linenoiseAddCompletion(lc,"/channel");
        linenoiseAddCompletion(lc,"/info");
        linenoiseAddCompletion(lc,"/join");
        linenoiseAddCompletion(lc,"/leave");
        linenoiseAddCompletion(lc,"/me");
        linenoiseAddCompletion(lc,"/modules");
        linenoiseAddCompletion(lc,"/nick");
        linenoiseAddCompletion(lc,"/pid");
        linenoiseAddCompletion(lc,"/tty");
    }

}

char *hints(const char *buf, int *color, int *bold) {
        if (!strcasecmp(buf,"hello")) {
            *color = 35;
            *bold = 0;
        return " World";
    }
        else if (!strcasecmp(buf,"/")) {
            *color = 35;
            *bold = 0;
        return "command name (Press TAB to cycle through command list ﴾͡๏̯͡๏﴿ O'RLY? )";
    }
        else if (!strcasecmp(buf,"/#")) {
            *color = 35;
            *bold = 0;
        return "channel (for example, typing \"#UPM hi\" will join the channel \"#UPM\" and post \"hi\", typing \"/#UPM\" will join \"#UPM\" without posting anything)";
    }
        else if (!strcasecmp(buf,"/bash")) {
            *color = 35;
            *bold = 0;
        return " colon ; seperated ; command ; list ; (executes a list of commands directly in a sub terminal with a timeout)";
    }
        else if (!strcasecmp(buf,"/bash_cmd")) {
            *color = 35;
            *bold = 0;
        return " displays previous bash commands entered via /bash";
    }
        else if (!strcasecmp(buf,"/channel")) {
            *color = 35;
            *bold = 0;
        return " displays the current channel and all currently joined channels";
    }
        else if (!strcasecmp(buf,"/info")) {
            *color = 35;
            *bold = 0;
        return " displays info such as current nick/user, channel, pid's, tty";
    }
        else if (!strcasecmp(buf,"/join")) {
            *color = 35;
            *bold = 0;
        return " channel (same as \"/#\" but only accepts \"/# channel\")";
    }
        else if (!strcasecmp(buf,"/leave")) {
            *color = 35;
            *bold = 0;
        return " leaves the current channel then changes to the last joined channel";
    }
        else if (!strcasecmp(buf,"/me")) {
            *color = 35;
            *bold = 0;
        return " text (used as an action in irc chats, for example \"*opens the door*\")";
    }
        else if (!strcasecmp(buf,"/modules")) {
            *color = 35;
            *bold = 0;
        return " displays all loaded modules";
    }
        else if (!strcasecmp(buf,"/nick")) {
            *color = 35;
            *bold = 0;
        return " new name (changes ur current nick to a new one and auto renames if desired nick is unavailable)";
    }
        else if (!strcasecmp(buf,"/pid")) {
            *color = 35;
            *bold = 0;
        return " displays the process identification numbers only (included in \"/info\")";
    }
        else if (!strcasecmp(buf,"/tty")) { 
             *color = 35;
             *bold = 0;
        return "displays the tty only (included in \"/info\")";
    }
    return NULL;
}

#include <findme.h>
    char *line;
    char *prgname = argv[0];
    const char *prompt = "";
    /* Parse options, with --multiline we enable multi line editing. */
    if(argv[1]) {
            if (!strcmp(argv[1],"--multiline")) {
                linenoiseSetMultiLine(1);
                printf("Multi-line mode enabled.\n");
            } else if (!strcmp(argv[1],"--keycodes")) {
                linenoisePrintKeyCodes();
                exit(0);
            } else if (!strcmp(argv[1],"-h") | !strcmp(argv[1],"--help")) {
                fprintf(stderr, "Usage: %s [--multiline] [--keycodes] [-p] [--prompt]\n\n", prgname);
                setenv("hi","hi",0);
                char tty = execl("/usr/bin/tty", "/usr/bin/tty", NULL);
//                 const char *name = "CHANNEL"; char *value; value = getenv(name);
                printf("%s\n", tty);
                exit(1);
            } else if (!strcmp(argv[1],"-p") | !strcmp(argv[1],"--prompt")) {
                if(argv[2]) { 
                    printf("Prompt Specified.\n");
                    prompt = argv[2];
                }
                else {
                    fprintf(stderr, "You did not specify a prompt name: defaulting to no name\n");
                    prompt = "";
                }
            } else
                fprintf(stderr, "Invalid argument...\nCotinuing with program using default paramaters\nUsage: %s [--multiline] [--keycodes] [-p] [--prompt]\n", prgname);
            
        }

    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad("history.txt"); /* Load the history at startup a*/

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */
    const char    *my_argv[64] = {"/bin/bash" , "./modules/module_CORE" , NULL , NULL};
    int rc = exec_prog(my_argv);
    while((line = linenoise(prompt)) != NULL) {
        /* Do something with the string. */
        if (line[0] != '\0') {
            FILE *fp;
            fp = fopen("input", ("w+"));
            fputs(line, fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
            fclose(fp);
            linenoiseHistoryAdd(line); /* Add to the history. */
            linenoiseHistorySave("history.txt"); /* Save the history on disk. */
        }
        free(line);
    }
    kill(0,SIGTERM);
    return 0;
}
