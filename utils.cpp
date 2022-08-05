#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
extern "C" {
#include "error.h"
}
#include "utils.h"

#ifdef _DEBUG

typedef struct
{
        void *p;
        char *descr;
        int size;
} memlist;
memlist *pm = NULL;
int n_pm = 0;

void LOG(char *s, ...)
{
        va_list ap;
        FILE *fh = fopen("log.log", "a+");
        if (!fh)
                error_exit("can't access log.log");

        va_start(ap, s);
        vfprintf(fh, s, ap);
        va_end(ap);

        fclose(fh);
}

void dump_mem(int sig)
{
        int loop;

        signal(SIGHUP, dump_mem);

        if (sig != SIGHUP)
                error_exit("unexpected signal %d for dump_mem\n", sig);

        LOG("%d elements of memory used\n", n_pm);
        for(loop=0; loop<n_pm; loop++)
        {
                LOG("%06d] %p %d (%s)\n", loop, pm[loop].p, pm[loop].size, pm[loop].descr);
        }
        LOG("--- finished memory dump\n");
}
void remove_mem_element(void *p, char *what)
{
        if (p)
        {
                int loop;
                for(loop=0; loop<n_pm; loop++)
                {
                        if (pm[loop].p == p)
                        {
                                int n_to_move;

                                n_to_move = (n_pm - loop) - 1;
                                memmove(&pm[loop], &pm[loop + 1], n_to_move * sizeof(memlist));
                                n_pm--;
                                loop=-1;

                                break;
                        }
                }

                if (loop != -1)
                {
                        LOG("myfree: pointer %p (%s) not found\n", p, what);
                }

		if (n_pm == 0)
		{
			free(pm);
			pm = NULL;
		}
		else
		{
			pm = (memlist *)realloc(pm, sizeof(memlist) * n_pm);
			if (!pm) error_exit("failed to shrink memorylist to %d elements\n", n_pm);
		}
        }
}
void add_mem_element(void *p, int size, char *what)
{
        pm = (memlist *)realloc(pm, sizeof(memlist) * (n_pm + 1));
        if (!pm) error_exit("failed to grow memorylist from %d elements\n", n_pm);
        pm[n_pm].p = p;
        pm[n_pm].size = size;
        pm[n_pm].descr = what;
        n_pm++;
}
#endif

void myfree(void *p, char *what)
{
#ifdef _DEBUG
	if (p)
		remove_mem_element(p, what);
#endif
        free(p);
}

void * myrealloc(void *oldp, int newsize, char *what)
{
#ifdef _DEBUG
        void *dummy;

        dummy = realloc(oldp, newsize);
        if (!dummy)
                error_exit("failed to reallocate to %d bytes for %s\n", newsize, what);
        remove_mem_element(oldp, what);
        add_mem_element(dummy, newsize, what);
        signal(SIGHUP, dump_mem);
#else
        /* ----------------------------------------------------
         * add code for repeatingly retry? -> then configurable
         * via configurationfile with number of retries and/or
         * sleep
         * ----------------------------------------------------
         */
        void *dummy = realloc(oldp, newsize);
        if (!dummy)
                error_exit("failed to reallocate to %d bytes for %s\n", newsize, what);
#endif

        return dummy;
}

void * mymalloc(int size, char *what)
{
	void *dummy = myrealloc(NULL, size, what);
	if (!dummy)
		error_exit("failed to allocate %d bytes for %s\n", size, what);

	return dummy;
}

char * mystrndup(char *in, int len)
{
#ifdef _DEBUG
	char *dummy = (char *)mymalloc(len + 1, in);
	if (!dummy)
		error_exit("failed to copy string '%s': out of memory?\n", in);

	memcpy(dummy, in, len + 1);

	return dummy;
#else
	char *dummy = strndup(in, len);
	if (!dummy)
		error_exit("failed to copy string '%s': out of memory?\n", in);

	return dummy;
#endif
}

char * mystrdup(char *in)
{
	return mystrndup(in, strlen(in));
}

ssize_t WRITE(int fd, char *whereto, size_t len)
{
        ssize_t cnt=0;

        while(len>0)
        {
                ssize_t rc;

                rc = write(fd, whereto, len);

                if (rc == -1)
                {
                        if (errno != EINTR && errno != EINPROGRESS && errno != EAGAIN)
                                error_exit("Problem writing to filedescriptor\n");
                }
                else if (rc == 0)
                {
                        break;
                }
                else
                {
                        whereto += rc;
                        len -= rc;
                        cnt += rc;
                }
        }

        return cnt;
}

int get_filesize(char *filename)
{
	struct stat buf;

	if (stat(filename, &buf) == -1)
	{
		if (errno != ENOENT)
			error_exit("stat failed for %s", filename);

		return -1;
	}

	return buf.st_size;
}

time_t get_filechanged(char *filename)
{
	struct stat buf;

	if (stat(filename, &buf) == -1)
	{
		if (errno != ENOENT)
			error_exit("stat failed for %s", filename);

		return -1;
	}

	return buf.st_mtime;
}

void resolve_host(char *host, struct sockaddr_in *addr)
{
        struct hostent *hostdnsentries;

        hostdnsentries = gethostbyname(host);
        if (hostdnsentries == NULL)
        {
                switch(h_errno)
                {
                case HOST_NOT_FOUND:
                        error_exit("The specified host is unknown.\n");
                        break;

                case NO_ADDRESS:
                        error_exit("The requested name is valid but does not have an IP address.\n");
                        break;

                case NO_RECOVERY:
                        error_exit("A non-recoverable name server error occurred.\n");
                        break;

                case TRY_AGAIN:
                        error_exit("A temporary error occurred on an authoritative name server. Try again later.\n");
                        break;

                default:
                        error_exit("Could not resolve %s for an unknown reason (%d)\n", host, h_errno);
                }
        }

        /* create address structure */
        addr -> sin_family = hostdnsentries -> h_addrtype;
        addr -> sin_addr = incopy(hostdnsentries -> h_addr_list[0]);
}

int connect_to(char *hoststr)
{
	int fd;
	char *colon = strchr(hoststr, ':');
	int portnr = 33333;
	struct sockaddr_in addr;

	if (colon)
	{
		*colon = 0x00;
		portnr = atoi(colon + 1);
	}

	/* resolve */
	memset(&addr, 0x00, sizeof(addr));
	resolve_host(hoststr, &addr);
        addr.sin_port   = htons(portnr);

	/* connect */
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
                error_exit("problem creating socket (%s)", strerror(errno));

        /* connect to peer */
        if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == 0)
        {
                /* connection made, return */
                return fd;
        }

	close(fd);

	return -1;
}
