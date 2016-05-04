#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memset.h"
#include "lib/endian.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/.h"

#include "errors.h"

static int debug=1;