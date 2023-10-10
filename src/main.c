#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SLEEP 500000

#define KEY_RETURN 10
#define KEY_ESC 27

#define MENU_RELOAD -1
#define MENU_CONNECT 0
#define MENU_DISCONNECT 1
#define MENU_RECONNECT 2
#define MENU_RELAY 3

WINDOW *spawn_centred_window(int height, int width);
int menu_select(WINDOW *menu_win, char **menu_items, size_t menu_size);
void mullvad_status();
void mullvad_command(WINDOW *local_win, char *arg);
void mullvad_relay(WINDOW *local_win, char **relay_list, size_t relay_list_size);

int main() {
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    char *menu_items[] = {
        "Connect", "Disconnect", "Reconnect", "Relay"
    };
    size_t menu_size = sizeof(menu_items) / sizeof(char*);

    char *relay_list[] = {
        "Australia (au)", "Bulgaria (bg)", "Canada (ca)", "Denmark (dk)", "France (fr)", "Germany (de)", "Italy (it)", "Japan (jp)", "Netherlands (nl)", "Norway (no)", "Singapore (sg)", "Spain (es)", "Sweden (se)", "UK (gb)", "USA (us)"
    };
    size_t relay_list_size = sizeof(relay_list) / sizeof(char*);

    WINDOW *menu_win = spawn_centred_window(LINES - 8, COLS - 4);
    int menu_selected;

    while (true) {
        mullvad_status();
        menu_selected = menu_select(menu_win, menu_items, menu_size);
        wclear(menu_win);

        switch (menu_selected) {
            case MENU_CONNECT:
                mullvad_command(menu_win, "connect");
                break;
            case MENU_DISCONNECT:
                mullvad_command(menu_win, "disconnect");
                break;
            case MENU_RECONNECT:
                mullvad_command(menu_win, "reconnect");
                break;
            case MENU_RELAY:
                mullvad_relay(menu_win, relay_list, relay_list_size);
                break;
            case MENU_RELOAD:
                break;
        }

        wrefresh(menu_win);
        usleep(SLEEP);
    }

    endwin();
    return 0;
}

WINDOW *spawn_centred_window(int height, int width) {
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    WINDOW *local_win = newwin(height, width, starty, startx);
    return local_win;
}

int menu_select(WINDOW *menu_win, char **menu_items, size_t menu_size) {
    int ch = 0;
    int menu_selected = 0;

    do {
        wclear(menu_win);
        switch (ch) {
            case 'j':
                if (menu_selected < menu_size - 1)
                    menu_selected++;
                else
                    menu_selected = 0;
                break;
            case 'k':
                if (menu_selected > 0)
                    menu_selected--;
                else
                    menu_selected = menu_size - 1;
                break;
            case KEY_ESC:
                return MENU_RELOAD;
            case 'q':
                endwin();
                exit(0);
        }

        for (int i = 0; i < menu_size; i++) {
            if (menu_selected == i) {
                wattron(menu_win, A_STANDOUT);
                wprintw(menu_win, "%s\n", menu_items[i]);
                wattroff(menu_win, A_STANDOUT);
                continue;
            }
            wprintw(menu_win, "%s\n", menu_items[i]);
        }
        wrefresh(menu_win);
    } while ((ch = getch()) != KEY_RETURN);

    return menu_selected;
}

void mullvad_status() {
    char *buffer;
    int bufsize = 128;
    buffer = malloc(bufsize * sizeof(char));

    char *command[] = { "mullvad", "status", NULL };
    int status;

    pid_t pid;
    int fd[2], len;
    pipe(fd);

    if ((pid = fork()) == 0) {
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);
        execvp(command[0], command);
    } else {
        wait(&status);
    }

    close(fd[1]);
    len = read(fd[0], buffer, bufsize - 1);
    close(fd[0]);
    buffer[len - 1] = '\0';

    clear();
    attron(A_BOLD);
    printw("Mullvad VPN Terminal User Interface\n");
    printw("%s\n", buffer);
    attroff(A_BOLD);
    printw("UP: J   DOWN: K   REFRESH: ESC   QUIT: Q\n");
    refresh();
}

void mullvad_command(WINDOW *local_win, char *arg) {
    char *command[] = { "mullvad", arg, NULL };
    int status;
    pid_t pid;

    if ((pid = fork()) == 0) {
        execvp(command[0], command);
    } else {
        wait(&status);
    }
}

void mullvad_relay(WINDOW *local_win, char **relay_list, size_t relay_list_size) {
    char *command[] = { "mullvad", "relay", "set", "location", "", NULL };
    int status;
    pid_t pid;

    char country_code[3];
    int location;
    location = menu_select(local_win, relay_list, relay_list_size);

    sscanf(relay_list[location], "%*s (%[a-z])", country_code);
    command[4] = country_code;

    wclear(local_win);
    wrefresh(local_win);

    if ((pid = fork()) == 0) {
        execvp(command[0], command);
    } else {
        wait(&status);
    }
}
