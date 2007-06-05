/* Copyright (C) 2001-2006 Artifex Software, Inc.
   All Rights Reserved.
  
   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied, modified
   or distributed except as expressly authorized under the terms of that
   license.  Refer to licensing information at http://www.artifex.com/
   or contact Artifex Software, Inc.,  7 Mt. Lassen Drive - Suite A-134,
   San Rafael, CA  94903, U.S.A., +1(415)492-9861, for further information.
*/

/* $Id$ */
/* %Calendar% IODevice */
#include "time_.h"
#include "ghost.h"
#include "gxiodev.h"
#include "istack.h"
#include "iparam.h"

/* ------ %Calendar% ------ */

private iodev_proc_get_params(calendar_get_params);
const gx_io_device gs_iodev_calendar = {
    "%Calendar%", "Special",
    { iodev_no_init, iodev_no_open_device, iodev_no_open_file,
      iodev_no_fopen, iodev_no_fclose,
      iodev_no_delete_file, iodev_no_rename_file, iodev_no_file_status,
      iodev_no_enumerate_files, NULL, NULL,
      calendar_get_params, iodev_no_put_params
    }
};

/* Get the date and time. */
private int
calendar_get_params(gx_io_device * iodev, gs_param_list * plist)
{
    int code;
    time_t t;
    struct tm *pltime;
    struct tm ltime;
    static const gs_param_item_t items[] = {
	{"Year", gs_param_type_int, offset_of(struct tm, tm_year)},
	{"Month", gs_param_type_int, offset_of(struct tm, tm_mon)},
	{"Day", gs_param_type_int, offset_of(struct tm, tm_mday)},
	{"Weekday", gs_param_type_int, offset_of(struct tm, tm_wday)},
	{"Hour", gs_param_type_int, offset_of(struct tm, tm_hour)},
	{"Minute", gs_param_type_int, offset_of(struct tm, tm_min)},
	{"Second", gs_param_type_int, offset_of(struct tm, tm_sec)},
	gs_param_item_end
    };
    bool running;

    if (time(&t) == (time_t)-1 || (pltime = localtime(&t)) == 0) {
	ltime.tm_sec = ltime.tm_min = ltime.tm_hour =
	    ltime.tm_mday = ltime.tm_mon = ltime.tm_year = 0;
	running = false;
    } else {
	ltime = *pltime;
	ltime.tm_year += 1900;
	ltime.tm_mon++;		/* 1-origin */
	running = true;
    }
    if ((code = gs_param_write_items(plist, &ltime, NULL, items)) < 0)
	return code;
    return param_write_bool(plist, "Running", &running);
}
