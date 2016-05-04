/**
 * @brief An entry in the Master Boot Record (MBR) partition table
 */
typedef struct {
	uint8_t bootable;		///< 0x80 if bootable, 0 else
	uint8_t start_chs[3];	///< start of partion in CHS-addressing
	uint8_t type;			///< type of the partition
	uint8_t end_chs[3];		///< end of partition in CHS-addressing
	uint32_t start_sector;	///< relative offset to partition in sectors (LBA)
	uint32_t size_sectors;	///< size of partition in sectors
} __attribute__((packed)) Partition;

/**
 * @brief The entire boot sector
 */
typedef struct {
	uint8_t 	jump[3];					///< Intel 80x86 jump instruction
	uint8_t		oem_name[8];				///< OEM name
	uint16_t 	sector_size;				///< Sector size in Bytes
	uint8_t 	sectors_per_cluster;		///< Number of sectors per cluster
	uint16_t	reserved_sectors;			///< Reserved sectors
	uint8_t 	num_FATs;					///< Number of FATs
	uint16_t	num_root_entries;			///< Number entries in root dir
	uint16_t 	total_sectors_short;		///< Total # of sectors; if 0 see  
											///<   later field
	uint8_t 	media_descriptor;			///< Media descriptor
	uint16_t 	sectors_per_FAT;			///< Number of sectors per FAT
	uint16_t 	sectors_per_cylinder;		///< Number of sectors per track 
											///<   (cylinder), CHS addressing
	uint16_t 	num_heads;					///< Number of heads, CHS addressing
	uint32_t 	num_hidden_sectors;			///< Number of hidden sectors (before 
											///<   boot sector)
	uint32_t 	total_sectors_long;			///< Total # of sectors; only valid if 
											///<   total_sectors_short is 0

	uint8_t 	drive_number;				///< Drive number
	uint8_t 	current_head;				///< Current head
	uint8_t 	boot_signature;				///< Boot signature; 0x29 means
											///<   next three fields valid
	uint32_t 	volume_id;					///< Volume ID (serial number)
	uint8_t		volume_label[11];			///< Volume label
	uint8_t		fs_type[8];					///< File system type
	uint8_t		boot_code[448];				///< Boot code
	uint8_t		boot_sector_signature[2];	///< Must be 0x55AA
} __attribute__((packed)) FAT16BootSector;

/**
 * @brief A directory entry
 */
typedef struct {
	uint8_t		filename[8];	///< Name of the file; 0th byte meaning:
								///< 	0x00: Unused
								///< 	0xe5: Deleted
								///< 	0x05: Actually starts with 0xe5
								///< 	0x2e: Directory
	uint8_t 	ext[3];	///< File extension; no special characters
	uint8_t 	attributes;		///< File attributes
	uint8_t 	reserved[10];	///< Special meaning - implementation-dependent
	uint16_t 	modify_time;	///< Time created/last modified; format:
								///<   	hhhhhmmmmmmsssss (seconds/2)
	uint16_t 	modify_date;	///< Data created/last modified; format:
								///<   	yyyyyyymmmmddddd (year 0 = 1980)
	uint16_t 	start_cluster;	///< Start of file in clusters
	uint32_t 	size;			///< File size in bytes
} __attribute__((packed)) FAT16_DirEntry;

int32_t fat16_get_inode(const char *name);
int32_t fat16_read_file(uint32_t inode, uint32_t offset, 
	void *buf, uint32_t count);
int32_t fat16_mount(struct superblock_t *superblock);
int32_t fat16_stat(int32_t inode, struct stat *buf);
int32_t fat16_getdents(uint32_t dir_inode,
		uint32_t *current_inode, void *buf,uint32_t size);