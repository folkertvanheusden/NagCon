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

#include <errno.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

void error_exit(const char *format, ...)
{
	int e = errno;
	va_list ap;

	endwin();

	va_start(ap, format);
	(void)vfprintf(stderr, format, ap);
	va_end(ap);
	fprintf(stderr, "errno=%d (if applicable)\n", e);

	exit(EXIT_FAILURE);
}

