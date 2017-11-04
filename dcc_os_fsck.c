#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/ext2_fs.h>

#define BASE_OFFSET 1024 // location of the super-block in the first group
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*block_size)

typedef int __le32;

void check_fun1(int);
void check_fun2(int);
void check_fun3(int);
void check_fun4(int);
void check_ex1(int);

/* ---- GLOBALS ---- */
const unsigned int super_backup_locations[3] = { 8193, 16384, 32768 };

struct ext2_super_block super;
unsigned int group_count;
unsigned int descr_list_size;
unsigned int block_size;
/* ----------------- */


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Error: Input file not entered.\n
                Usage: dcc_os_fsck <path>");
        exit(EXIT_FAILURE);
    }
    
    int sd; // input file descriptor
    
    sd = open(argv[1], O_RDWR);

    if (sd < 0) {
        printf("Error: Unable to open input file.\n
                Usage: dcc_os_fsck <path>");
        exit(EXIT_FAILURE);
    }

    printf("Scanning file system...");    

    lseek(sd, BASE_OFFSET, SEEK_SET); // position head above first super-block
    read(sd, &super, sizeof(super)); // read superblock

    /* [Fun 1]: check if super-block was attacked */
    check_fun1(sd);

    group_count = 1 + (super.s_blocks_count-1) / super.s_blocks_per_group; // calculate number of block groups on the disk
    descr_list_size = group_count * sizeof(struct ext2_group_descr); // calculate size of the group descriptor list in bytes
    block_size = 1024 << super.s_log_block_size; // calculate block size in bytes

    /* [Fun 2]: check if magic number matches */
    check_fun2(sd);

    // Fun 3
    check_fun3(sd);

    // Fun 4
    check_fun4(sd);


    printf("fsck exiting.\nNo errors found.\n");
    exit(EXIT_SUCCESS);
}

/* ---------- Auxiliary Functions --------- */
int is_attacked(struct ext2_super_block super_block) {
    if (super_block == NULL ||
       (super.s_magic == NULL) ||        
       (super.s_magic == EXT2_SUPER_MAGIC))
        return 1;
    else return 0;
}

void copy_super(struct ext2_super_block super_block) {
    lseek(sd, BASE_OFFSET, SEEK_SET);
    write(sd, &super_block, sizeof(super_block));
}
/* ---------------------------------------- */


void check_fun1(int fd) {
    short i;
    char input;
    struct ext2_super_block temp_super;

    if (is_attacked(super)) {
        printf("Disk error found:\n
                [Fun 1] Super-block magic number does not match and the disk could not be read.\nThe super-block might have been attacked or the file system is not ext2.\n\nDo you wish to attempt to recover it by using a backup on the disk? (Y/<N>)");
        scanf("%c", &input);
        printf("\n")
        if (input == "Y") {
            for(i = 0; i < 3; i++) {
                printf("Checking backup at addr %d...\n", super_backup_location[i]);
                lseek(sd, super_backup_location[i], SEEK_SET);
                read(sd, &temp_super, sizeof(temp_super));
                if (!is_attacked(temp_super)) {
                    printf("Backup found, now copying...\n");
                    copy_super(&temp_super);
                    printf("Backup complete.\n");
                    return;
                } else {
                    printf("Could not be used.\n");
                }
            }
            printf("Backup could not be found.")
        }
    }
    printf("Could not continue fsck.\nExiting with errors.\n");    
    exit(EXIT_FAILURE);
}

void check_fun2(int fd) {
    
}

void check_fun3(int fd) {
    
}

void check_fun4(int fd) {
    
}

void check_ex1(int fd) {
    if (super.s_magic != EXT2_SUPER_MAGIC) {
        printf("Disk Error found:\n[Extra 1] File system magic number does not match.\n");
        exit(EXIT_FAILURE);
    }
    printf("[Extra 1] Magic number matches. \n");
}