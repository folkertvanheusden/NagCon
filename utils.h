void * mymalloc(int size, char *what);
void * myrealloc(void *oldp, int newsize, char *what);
char * mystrdup(char *in);
char * mtstrndup(char *in, int len);
void myfree(void *p, char *what);
ssize_t WRITE(int fd, char *whereto, size_t len);
int get_filesize(char *filename);
time_t get_filechanged(char *filename);
void resolve_host(char *host, struct sockaddr_in *addr);
int connect_to(char *hoststr);

#define incopy(a)       *((struct in_addr *)a)

#define max(x, y)	((x) > (y) ? (x) : (y))
#define min(x, y)	((x) < (y) ? (x) : (y))
