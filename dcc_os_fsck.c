#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/ext2_fs.h>

/* location of the super-block in the first group */
#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*block_size)

typedef int __le32;

void init_data_structures();

void cleanup_data_structures();

/* ---- GLOBALS ---- */
int sd; // input file descriptor
struct ext2_super_block super; // super-block
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
    
    char user_input;
    sd = open(argv[1], O_RDWR);

    if (sd < 0) {
        printf("Error: Unable to open input file.\n
                Usage: dcc_os_fsck <path>");
        exit(EXIT_FAILURE);
    }

    init_data_structures();

    printf("Scanning file system...");    

    // Fun 1: check if  super is zerod
    if (iszerod(super)) {
        printf("Disk error found:\n
                [Fun 1] Super-block was attacked and cannot be read.\n\n");

        printf("Do you wish to recover it using a back-up? (Y/<N>)");
        scanf("%c", &user_input);
        if (user_input == "Y") {
            printf("\nLooking for a backup...\n");
            struct ext2_super_block temp_super;
            /* Loop through each group */
            for (???) {
                lseek(sd, BASE_OFFSET + group_size * i, SEEK_SET);
                read(sd, &temp_super, sizeof(temp_super));
                if (!iszerod(temp_super)) {
                    printf("Backup found on group %d.\n", i);
                    copy_super(&temp_super);
                    printf("Backup complete.\n");
                    break;
                }
            }
        } else {
            cleanup_data_structures();
            printf("fsck exiting with errors.\n");
            exit(EXIT_FAILURE);
        }

    }

    /* calculate number of block groups on the disk */
    group_count = 1 + (super.s_blocks_count-1) / super.s_blocks_per_group;

    /* calculate size of the group descriptor list in bytes */
    descr_list_size = group_count * sizeof(struct ext2_group_descr);
        
    /* calculate block size in bytes */
    block_size = 1024 << super.s_log_block_size; 

	if (super.s_magic != EXT2_SUPER_MAGIC) {
        printf("Error: File system magic number does not match.\n");
        exit(EXIT_FAILURE);
    }


    struct ext2_group_descr group_descr;
    /* iterate over each group */
    for (i = 0; i < group_count; i++) {

        lseek(sd, BASE_OFFSET + group_size * i + block_size, SEEK_SET);
        read(sd, &group_descr, sizeof(group_descr));

        // ...

    }

    if(1) {
        printf("fsck successful.\n
                No errors found.\n");
        cleanup_data_structures();
        exit(EXIT_SUCCESS);
    }
}

void init_data_structures() {
    /* position head above first super-block */
    lseek(sd, BASE_OFFSET, SEEK_SET);

    /* read super-block */
    read(sd, &super, sizeof(super));
}

void cleanup_data_structures() {

}