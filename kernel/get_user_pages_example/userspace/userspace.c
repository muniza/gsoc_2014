#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[])
{
	int mh, res;
       char addrstr[80];
	char *ptr, *bp;
	const char *string = "Testing page";

	printf ("%s\n", argv[0]);
	if (argc < 2)
	{
		printf ("Usage: %s <file>\n", argv[0]);
		return 1;
	}
	mh = open (argv [1], O_RDWR);
	if (mh == -1)
	{
		printf ("Unable to open '%s'\n", argv [1]);
		return 1;
	}
	/* Grab a page of memory (4096) and sprintf into it */
	
	res = posix_memalign((void **)&ptr, 4096, 4096);

	if (NULL != ptr)
	{
	    sprintf(ptr, "%s", string);
	
	    bp = ptr;
	
	    /* Do a write to the kernel at which point it should mess with it */
	    sprintf(addrstr, "%p", ptr);
	    printf("Telling kernel it is %p\n", ptr);
           res = write(mh, addrstr, strlen(addrstr));
	
	    /* See what lives there still */
	    printf ("Marker text 1: %s\n", ptr);
	    printf ("ptr = %p, bp = %p\n", ptr, bp);

    }
    close (mh);
	return 0;
}
