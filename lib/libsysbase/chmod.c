#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/iosupport.h>
#include <errno.h>

int chmod(const char *path, mode_t mode) {
	int	ret,dev;
	struct _reent *r = _REENT;

	ret = -1;

	/* Get device from path name */
	dev = FindDevice(path);

	if (dev < 0) {
		r->_errno = ENODEV;
	} else {
		if (devoptab_list[dev]->chmod_r == NULL) {
			r->_errno=ENOSYS;
		} else {
			ret = devoptab_list[dev]->chmod_r(r,path,mode);
		}
	}

	return ret;
}

