#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memset.h"
#include "lib/endian.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/fat16/fat16.h"

#include "errors.h"

static int debug=1;

/* Offsets for finding things */
#define PARTITION_TABLE_OFFSET 0x1BE
static uint32_t fat_start;
static uint32_t root_start;
static uint32_t data_start;

static FAT16BootSector bootSector;

/* Just use the file entry offset as the inode.
	Note: only looking in root directory */
int32_t fat16_get_inode(const char *name) {
	int i;
	int32_t inode = 0;
	uint32_t offset = root_start;
	FAT16DirEntry entry;

	/* Scan through root dir looking for matching filename */
	for (i = 0; i < bootSector.num_root_entries; i++) {
		inode = offset;

		ramdisk_read(offset, sizeof(FAT16DirEntry), &entry);

		if (!strncmp(name, entry.filename, 8)) {
			return inode;
		}
	}

	return -1;
}


int32_t fat16_read_file(uint32_t inode, uint32_t offset, 
	void *buf, uint32_t count);


int32_t fat16_mount(struct superblock_t *superblock) {
	int i;
	uint32_t offset = 0;
	uint32_t boot_offset;

	Partition partitionTable[4];
	/* Read the partition table */
	offset = PARTITION_TABLE_OFFSET;
	ramdisk_read(offset, sizeof(Partition)*4, partitionTable);
	
	/* Check for valid FAT16 partition type */
	for (int i = 0; i < 4; i++) {
		if (partitionTable[i].type == 4 ||
			partitionTable[i].type == 6 ||
			partitionTable[i].type == 14) {
			if (debug) printk("Found FAT16 partition!\n");
			break;
		}
	}

	/* Calculate boot sector offset */
	if (i == 4) {
		if (debug) printk("No FAT16 partition found!\n");
		return -1;
	} else {
		boot_offset = 512 * partitionTable[i].start_sector;
	}

	/* Read the MBR */
	offset = boot_offset;
	ramdisk_read(offset, sizeof(FAT16BootSector), &bootSector);

	/* Calculate start locations of FAT, root dir, and data */
	fat_start = bootSector.reserved_sectors * bootSector.sector_size;
	root_start = fat_start + bootSector.sectors_per_FAT * bootSector.num_FATs * bootSector.sector_size;
	data_start = root_start + bootSector.num_root_entries * sizeof(DirEntry);

	return 0;
}


int32_t fat16_stat(int32_t inode, struct stat *buf);


int32_t fat16_getdents(uint32_t dir_inode,
		uint32_t *current_inode, void *buf,uint32_t size);