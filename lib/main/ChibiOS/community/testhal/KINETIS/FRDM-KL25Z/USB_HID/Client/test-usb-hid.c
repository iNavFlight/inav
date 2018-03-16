/*

  Copyright (c) 2014 Guillaume Duc <guillaume@guiduc.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>

#define USB_HID_IN_REPORT_SIZE 1
#define USB_HID_OUT_REPORT_SIZE 1

struct usb_hid_in_report_s
{
  uint8_t sequence_number;
};

struct usb_hid_out_report_s
{
  uint8_t sequence_number;
};

static uint8_t usb_hid_in_report_buf[USB_HID_IN_REPORT_SIZE];
// +1 for the report index
static uint8_t usb_hid_out_report_buf[USB_HID_OUT_REPORT_SIZE + 1];

static struct usb_hid_in_report_s *usb_hid_in_report =
  (struct usb_hid_in_report_s *) usb_hid_in_report_buf;

static struct usb_hid_out_report_s *usb_hid_out_report =
  (struct usb_hid_out_report_s *) (&usb_hid_out_report_buf[1]);

static int usb_hid_fd;
static uint8_t wkup_pb_old_value = 0;

static void
read_in_report ()
{
  int res, i;

  printf ("read()\n");
  res = read (usb_hid_fd, usb_hid_in_report_buf, USB_HID_IN_REPORT_SIZE);
  if (res < 0)
    {
      perror ("read");
      exit (EXIT_FAILURE);
    }
  else
    {
      printf ("read() read %d bytes:\t", res);
      for (i = 0; i < res; i++)
	printf ("%02hhx ", usb_hid_in_report_buf[i]);
      printf ("\n");
    }
}

static void
send_out_report ()
{
  int res;

  usb_hid_out_report_buf[0] = 0;

  res =
    write (usb_hid_fd, usb_hid_out_report_buf, USB_HID_OUT_REPORT_SIZE + 1);
  if (res < 0)
    {
      perror ("write");
      exit (EXIT_FAILURE);
    }

  usb_hid_out_report->sequence_number++;
}

static void
usb_hid_init (const char *dev_name)
{
  int i, res;
  int desc_size = 0;
  char buf[256];

  struct hidraw_report_descriptor rpt_desc;
  struct hidraw_devinfo info;

  usb_hid_fd = open (dev_name, O_RDWR);

  if (usb_hid_fd < 0)
    {
      perror ("Unable to open device");
      exit (EXIT_FAILURE);
    }

  memset (&rpt_desc, 0x0, sizeof (rpt_desc));
  memset (&info, 0x0, sizeof (info));
  memset (buf, 0x0, sizeof (buf));

  // Get Report Descriptor Size
  res = ioctl (usb_hid_fd, HIDIOCGRDESCSIZE, &desc_size);
  if (res < 0)
    perror ("HIDIOCGRDESCSIZE");
  else
    printf ("Report Descriptor Size: %d\n", desc_size);

  // Get Report Descriptor
  rpt_desc.size = desc_size;
  res = ioctl (usb_hid_fd, HIDIOCGRDESC, &rpt_desc);
  if (res < 0)
    {
      perror ("HIDIOCGRDESC");
    }
  else
    {
      printf ("Report Descriptor:\n");
      for (i = 0; i < rpt_desc.size; i++)
	printf ("%02hhx ", rpt_desc.value[i]);
      puts ("\n");
    }
}

int
main (int argc, char **argv)
{
  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s /dev/hidrawX\n", argv[0]);
      return EXIT_FAILURE;
    }

  memset (usb_hid_out_report_buf, 0, sizeof (usb_hid_out_report_buf));

  usb_hid_init (argv[1]);
  usb_hid_out_report->sequence_number = 4;
  send_out_report ();

  while (1)
    {
      read_in_report ();

      if (usb_hid_in_report->sequence_number == 40)
	{
	  usb_hid_out_report->sequence_number = usb_hid_in_report->sequence_number / 2;
	  send_out_report ();
	}

    }

  close (usb_hid_fd);

  return EXIT_SUCCESS;
}
