//      This file is part of iCP12 by BayCom GmbH.
//
//      iCP12 is free software: you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation, either version 3 of the License, or
//      (at your option) any later version.
//
//      iCP12 is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with iCP12.  If not, see <http://www.gnu.org/licenses/>.
//
//      The developer can be reached at software@baycom.tv


#include <stdio.h>		/* Standard input/output definitions */
#include <string.h>		/* String function definitions */
#include <unistd.h>		/* UNIX standard function definitions */
#include <fcntl.h>		/* File control definitions */
#include <errno.h>		/* Error number definitions */
#include <termios.h>		/* POSIX terminal control definitions */
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

static int fd=-1;
static	char receive[32];

void icp12open (char *device)
{
	int n;
	int bytes = -1;

	struct termios options;
	//puts(cmd);
	if(fd==-1) {
		fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);

		if (fd == -1) {
			fprintf (stderr, "Cannot open device: %s\n", device);
			return;
		}

		tcgetattr (fd, &options);

		/* SEt Baud Rate */
		cfsetispeed (&options, B115200);
		cfsetospeed (&options, B115200);

		// Enable Read
		options.c_cflag |= (CLOCAL | CREAD);

		// Set the Charactor size
		options.c_cflag &= ~CSIZE;	/* Mask the character size bits */
		options.c_cflag |= CS8;	/* Select 8 data bits */

		// Set parity - No Parity (8N1)
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;

		// Enable Raw Input
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

		// Disable Software Flow control
		options.c_iflag &= ~(IXON | IXOFF | IXANY);

		// Chose raw (not processed) output
		options.c_oflag &= ~OPOST;

		if (tcsetattr (fd, TCSANOW, &options) == -1) {
			fprintf (stderr, "Error with tcsetattr = %s\n", strerror (errno));
		}
	}
}
char *icp12 (char *cmd)
{	
	int n;
	int reclen=31;
	int bytes = -1;

	// Write to the port
	int len = strlen (cmd);
	n = write (fd, cmd, len);
	if (n != len) {
		fprintf (stderr, "write() of command %s failed!\n", cmd);
	}

	memset (receive, 0, reclen);
	usleep (5000);

	time_t t = time (NULL);
	while ((time (NULL) - t) < 1) {
		bytes = read (fd, receive, reclen);
		if (bytes > 0) {
			break;
		}
		usleep (100000);
	}
	if (bytes <= 0) {
		fprintf (stderr, "Error reading from device, command: %s\n", cmd);
	}
	usleep (1000);
	return receive;
}


