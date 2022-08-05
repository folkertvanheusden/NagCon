/*
 * Copyright (C) 2006 Folkert van Heusden <folkert@vanheusden.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <errno.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef __sun__
#include <sys/termio.h>
#include <sys/socket.h>
#endif

extern "C" {
#include "error.h"
}
#include "utils.h"
#include "pl.h"

#define MY_RED          1
#define MY_GREEN        2
#define MY_YELLOW       3
#define MY_BLUE         4
#define MY_MAGENTA      5
#define MY_CYAN         6
#define MY_DRAW         7

char *state_str[4] = { "OK", "WA", "CR", "??" };
int terminal_resized = 0;
int max_sort_order = 2;
int sort_order = 0;
int group_by_state = 1;

void init_curses(void)
{
        initscr();
	start_color();
        keypad(stdscr, TRUE);
        cbreak();
        intrflush(stdscr, FALSE);
        leaveok(stdscr, TRUE);
        noecho();
        nonl();
        refresh();
        nodelay(stdscr, FALSE);
        meta(stdscr, TRUE);     /* enable 8-bit input */
        idlok(stdscr, TRUE);    /* may give a little clunky screenredraw */
	raw();

	use_default_colors();

	init_pair(MY_RED, COLOR_RED, -1);
	init_pair(MY_GREEN, COLOR_GREEN, -1);
	init_pair(MY_YELLOW, COLOR_YELLOW, -1);
	init_pair(MY_BLUE, COLOR_BLUE, -1);
	init_pair(MY_MAGENTA, COLOR_MAGENTA, -1);
	init_pair(MY_CYAN, COLOR_CYAN, -1);
	init_pair(MY_DRAW, -1, -1);
}

void determine_terminal_size(int *max_y, int *max_x)
{
        struct winsize size;

        *max_x = *max_y = 0;

        /* changed from 'STDIN_FILENO' as that is incorrect: we're
         * outputting to stdout!
         */
        if (ioctl(1, TIOCGWINSZ, &size) == 0)
        {
                *max_y = size.ws_row;
                *max_x = size.ws_col;
        }

        if (!*max_x || !*max_y)
        {
                char *dummy = getenv("COLUMNS");
                if (dummy)
                        *max_x = atoi(dummy);
                else
                        *max_x = 80;

                dummy = getenv("LINES");
                if (dummy)
                        *max_x = atoi(dummy);
                else
                        *max_x = 24;
        }
}

void wrong_key(void)
{
        flash();
        flushinp();
}

char *limit_str(char *str, int max_len)
{
	char *out = (char *)mymalloc(max_len + 1, "limit_str");
	int len = str?strlen(str):0;
	int copy_len = min(max_len, len);

	memset(out, ' ', max_len);
	if (copy_len)
		memcpy(out, str, copy_len);
	out[max_len] = 0x00;

	return out;
}

void online_help(int max_x, int max_y)
{
	WINDOW *win = newwin(15, 45, (max_y / 2) - (12 / 2), (max_x / 2) - (45 / 2));
	if (!win)
		error_exit("failed to create window");

	werase(win);
	box(win, 0, 0);

	mvwprintw(win, 1, 2, "The following keys will toggle a setting:");
	mvwprintw(win, 2, 2, "a  list all problems");
	mvwprintw(win, 3, 2, "n  always notify");
	mvwprintw(win, 4, 2, "k  also acknowlegded");
	mvwprintw(win, 5, 2, "o  hide ok");
	mvwprintw(win, 6, 2, "g  turn off grouping by status");
	mvwprintw(win, 7, 2, "s  change sort order");
	mvwprintw(win, 8, 2, "h  this help");
	mvwprintw(win, 10, 2, "[]{} resize host/service fields");
	mvwprintw(win, 11, 2, "q  exit");
	mvwprintw(win, 13, 2, "Press any key to exit this help");

	wnoutrefresh(win);
	doupdate();

	(void)getch();

	delwin(win);
}

void version(void)
{
	printf("nagcon v" VERSION ", (C) 2005 by folkert@vanheusden.com\n\n");
}

void help(void)
{
	version();

	printf("-f file   what file to monitor (usuallly:\n");
	printf("-F host:port   connect to a host for retrieving the status.log information\n");
	printf("          /usr/local/nagios/var/status.log, look for status_file in\n");
	printf("          the nagios.cfg file\n");
	printf("-i x      check interval (in seconds)\n");
	printf("-a        list also the services for hosts that are down\n");
	printf("-n        also display services for which notify has been switched off\n");
	printf("-k        also display problems that already have been acknowledged\n");
	printf("-g        turn off grouping by status\n");
	printf("-s        change sort order\n");
	printf("-e/-o     suppress services with ok status\n");
	printf("-x        status.log-file is in Nagios 1.0 format\n");
	printf("-X        status.log-file is in Nagios 2.0 format\n");
	printf("-1 x      set width of hostname column\n");
	printf("-2 x      set width of service description column\n");
	printf("-h        this help\n");
	printf("-V        display version\n");
}

void do_resize(int sig)
{
	terminal_resized = 1;
}

int main(int argc, char *argv[])
{
	struct stats *pstats = NULL;
	struct stat statstruct;
	int n_stats = 0;
	int n_critical = 0, n_warning = 0, n_ok = 0, n_up = 0, n_down = 0, n_unreachable = 0, n_pending = 0;
	int max_x = 80, max_y = 25;
	WINDOW *win;
	int sw;

	char *status_log = "/usr/local/nagios/var/status.log";
	int interval = 5;
	char list_all_problems = 0;
	char always_notify = 0;
	char also_acknowledged = 0;
	char hide_ok = 0;
	int host_width = 8;
	int service_width = 6;
	char nagios_version = 2;
	char file_mode = 0;

	while ((sw = getopt (argc, argv, "xXoeankF:f:i:1:2:hVgs:")) != EOF)
	{
		switch(sw)
		{
			case 'x':
				nagios_version = 1;
				break;

			case 'X':
				nagios_version = 2;
				break;

			case 'f':
				status_log = optarg;
				file_mode = 0;
				break;

			case 'F':
				status_log = optarg;
				file_mode = 1;
				break;

			case 'i':
				interval = atoi(optarg);
				break;

			case 'a':
				list_all_problems = 1;
				break;

			case 'n':
				always_notify = 1;
				break;

			case 'k':
				also_acknowledged = 1;
				break;

			case 'o':
			case 'e':
				hide_ok = 1;
				break;

			case 's':
				sort_order = atoi(optarg);
				if(sort_order < 0 || sort_order > max_sort_order)
					error_exit("invalid sort-order");
				break;

			case 'g':
				group_by_state = 1;
				break;

			case '1':
				host_width = atoi(optarg);
				if (host_width < 1 || host_width > (max_x - (22 + service_width)))
					error_exit("invalid host-column width");
				break;

			case '2':
				service_width = atoi(optarg);
				if (service_width < 1 || service_width > (max_x - (22 + host_width)))
					error_exit("invalid service-column width");
				break;

			case 'V':
				version();
				return 0;

			default:
				help();
				return 0;
		}
	}

	if (file_mode == 0 && stat(status_log, &statstruct) == -1)
		error_exit("error accessing nagios status.log file! (%s)", status_log);

	init_curses();
	signal(SIGWINCH, do_resize);

	getmaxyx(stdscr, max_y, max_x);
	win = newwin(max_y, max_x, 0, 0);

	for(;;)
	{
		fd_set rfds;
		struct timeval tm;
		int disp_index = 0;
		time_t now = time(NULL);
		char *time_str = ctime(&now);
		char *dummy = strchr(time_str, '\n');
		if (dummy) *dummy = 0x00;
		int fd;

		werase(win);
		wattron(win, COLOR_PAIR(MY_DRAW));
		box(win, 0, 0);
		wattroff(win, COLOR_PAIR(MY_DRAW));

		if (file_mode == 0)	/* file */
			fd = open64(status_log, O_RDONLY);
		else
			fd = connect_to(status_log);

		if (fd != -1)
		{
			if (nagios_version == 2)
				parse_2_0_statuslog(fd, &pstats, &n_stats);
			else if (nagios_version == 1)
				parse_1_0_statuslog(fd, &pstats, &n_stats);
			else
				error_exit("internal error: nagios version %d is unknown\n", nagios_version);
			calc_stats_stats(pstats, n_stats, list_all_problems, always_notify, also_acknowledged, hide_ok, &n_critical, &n_warning, &n_ok, &n_up, &n_down, &n_unreachable, &n_pending);
			sort_stats_array(pstats, n_stats);

			mvwprintw(win, 1, 1, "Hosts: ");
			wattron(win, COLOR_PAIR(MY_RED));
			mvwprintw(win, 1, 8,  "down: %-4d", n_down);
			wattroff(win, COLOR_PAIR(MY_RED));
			wattron(win, COLOR_PAIR(MY_YELLOW));
			mvwprintw(win, 1, 19, "unreachable: %-4d", n_unreachable);
			wattroff(win, COLOR_PAIR(MY_YELLOW));
			wattron(win, COLOR_PAIR(MY_DRAW));
			mvwprintw(win, 1, 38, "pending: %-4d", n_pending);
			wattroff(win, COLOR_PAIR(MY_DRAW));
			wattron(win, COLOR_PAIR(MY_GREEN));
			mvwprintw(win, 1, 53, "up: %-4d", n_up);
			wattroff(win, COLOR_PAIR(MY_GREEN));

			mvwprintw(win, 2, 1, "Services: ");
			wattron(win, COLOR_PAIR(MY_RED));
			mvwprintw(win, 2, 11,  "critical: %-4d", n_critical);
			wattroff(win, COLOR_PAIR(MY_RED));
			wattron(win, COLOR_PAIR(MY_YELLOW));
			mvwprintw(win, 2, 25, "warning: %-4d", n_warning);
			wattroff(win, COLOR_PAIR(MY_YELLOW));
			wattron(win, COLOR_PAIR(MY_GREEN));
			mvwprintw(win, 2, 39, "ok: %-4d", n_ok);
			wattroff(win, COLOR_PAIR(MY_GREEN));

			wmove(win, 3, 1);
			wattron(win, COLOR_PAIR(MY_DRAW));
			whline(win, 0, max_x - 2);
			mvwprintw(win, 3, max_x - (strlen(time_str) + 6), "[ %s ]", time_str);
			mvwprintw(win, 3, 2, "[ Press 'h' for help ]");
			wattroff(win, COLOR_PAIR(MY_DRAW));

			for(int loop=0; disp_index<min(max_y - 5, n_stats) && loop<n_stats; loop++)
			{
				char *host_name, *service_d, *plugin_ou;
				int color = MY_DRAW;
				struct tm *ptm = localtime(&pstats[loop].last_state_change);

				if (!should_i_show_entry(pstats, n_stats, loop, list_all_problems, always_notify, also_acknowledged, hide_ok))
					continue;

				host_name = limit_str(pstats[loop].host_name, host_width);
				service_d = limit_str(pstats[loop].service_description, service_width);
				plugin_ou = limit_str(pstats[loop].plugin_output, max(0, max_x - (22 + host_width + service_width)));

				if (pstats[loop].current_state == 0)
					color = MY_GREEN;
				else if (pstats[loop].current_state == 1)
					color = MY_YELLOW;
				else if (pstats[loop].current_state == 2)
					color = MY_RED;
				else if (pstats[loop].current_state == 3)
					color = MY_CYAN;

				wattron(win, COLOR_PAIR(color));
				mvwprintw(win, 4 + disp_index, 1, "%04d%02d%02d %02d:%02d %s %s %-2s %s",
						ptm -> tm_year + 1900, ptm -> tm_mon + 1, ptm -> tm_mday, ptm -> tm_hour, ptm -> tm_min,
						host_name,
						service_d,
						state_str[pstats[loop].current_state],
						plugin_ou);
				wattroff(win, COLOR_PAIR(color));

				myfree(host_name, "hostname");
				myfree(service_d, "service description");
				myfree(plugin_ou, "plugin output");

				disp_index++;
			}

			wnoutrefresh(win);
			doupdate();

			free_stats_array(pstats, n_stats);

			close(fd);
		}

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		tm.tv_sec = interval;
		tm.tv_usec = 0;

		if (select(1, &rfds, NULL, NULL, &tm) == -1)
		{
			if (errno != EAGAIN && errno != EINTR)
				error_exit("select failed");
		}

		if (FD_ISSET(0, &rfds))
		{
			int c = getch();
			int cu = toupper(c);

			if (c == KEY_RESIZE || c == KEY_F(5))
				terminal_resized = 1;
			else if (c == 3 || c == 7 || cu == 'Q')
				break;
			else if (cu == 'A')
				list_all_problems = 1 - list_all_problems;
			else if (cu == 'N')
				always_notify = 1 - always_notify;
			else if (cu == 'K')
				also_acknowledged = 1 - also_acknowledged;
			else if (cu == 'S')
				sort_order = (sort_order + 1) % (max_sort_order + 1);
			else if (cu == 'G')
				group_by_state = 1 - group_by_state;
			else if (cu == 'O')
				hide_ok = 1 - hide_ok;
			else if (cu == 'H')
				online_help(max_x, max_y);
			else if (cu == '[')
			{
				if (host_width > 0)
					host_width--;
				else
					wrong_key();
			}
			else if (cu == ']')
			{
				if (host_width < (max_x - (22 + service_width + 1)))
					host_width++;
				else
					wrong_key();
			}
			else if (cu == '{')
			{
				if (service_width > 0)
					service_width--;
				else
					wrong_key();
			}
			else if (cu == '}')
			{
				if (service_width < (max_x - (22 + host_width + 1)))
					service_width++;
				else
					wrong_key();
			}
			else
			{
				wrong_key();
			}
		}
		if (terminal_resized)
		{
			determine_terminal_size(&max_y, &max_x);
			delwin(win);
			wresize(stdscr, max_y, max_x);
			win = newwin(max_y, max_x, 0, 0);
			wclear(win);

			terminal_resized = 0;
		}
	}

	endwin();

	return 0;
}
