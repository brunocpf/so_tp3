#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fs.h>
#include "ext2.h"

#define BASE_OFFSET 1024 // location of the super-block in the first group
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*block_size)
#define CHAR_SIZE ((int)(sizeof(char) * 8))
#define TEST_BIT(ary, bitno) ( (ary[(bitno)/CHAR_SIZE] & (1 << ((bitno)%CHAR_SIZE) )) != 0 )
#define CLEAR_BIT(ary, bitno) ( ary[((bitno)/CHAR_SIZE)] &= ~(1 << ((bitno)%CHAR_SIZE)) )            


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
unsigned int group_size;
/* ----------------- */


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Error: Input file not entered.\nUsage: dcc_os_fsck <path>\n");
        exit(EXIT_FAILURE);
    }

    int sd; // input file descriptor
    
    sd = open(argv[1], O_RDWR);

    if (sd < 0) {
        printf("Error: Unable to open input file.\nUsage: dcc_os_fsck <path>\n");
        exit(EXIT_FAILURE);
    }

    printf("Scanning file system...\n");    

    lseek(sd, BASE_OFFSET, SEEK_SET); // position head above first super-block
    read(sd, &super, sizeof(super)); // read superblock

    printf("\n--> [Fun 1]: Checking super-block...\n");
    check_fun1(sd);
    
    group_count = 1 + (super.s_blocks_count-1) / super.s_blocks_per_group; // calculate number of block groups on the disk
    descr_list_size = group_count * sizeof(struct ext2_group_desc); // calculate size of the group descriptor list in bytes
    block_size = 1024 << super.s_log_block_size; // calculate block size in bytes
    group_size = block_size * super.s_blocks_per_group;
    
    printf("\n--> [Fun 2]: Checking for multiply-owned blocks...\n");    
    check_fun2(sd);

    // Fun 3
    //check_fun3(sd);

    // Fun 4
    check_fun4(sd);

    printf("fsck exiting.\nNo errors found.\n");
    exit(EXIT_SUCCESS);
}

/* ---------- Auxiliary Functions --------- */
int is_attacked(struct ext2_super_block super_block) {
    return (super_block.s_magic != EXT2_SUPER_MAGIC);
}

int is_permission_corrupted(struct ext2_inode inode) {
    return ((inode.i_mode & 0xff) == 0);
}

void copy_super(int fd, struct ext2_super_block super_block) {
    lseek(fd, BASE_OFFSET, SEEK_SET);
    write(fd, &super_block, sizeof(super_block));
    
    lseek(fd, BASE_OFFSET, SEEK_SET);
    read(fd, &super, sizeof(super));
}

static void read_inode(int fd, int inode_no, const struct ext2_group_desc *group, unsigned int group_no, struct ext2_inode *inode) {
    unsigned int offset = BLOCK_OFFSET(group->bg_inode_table) + group_size * group_no;
    lseek(fd, offset+(inode_no-1)*sizeof(struct ext2_inode), SEEK_SET);
    read(fd, inode, sizeof(struct ext2_inode));
}
/* ---------------------------------------- */

void check_fun1(int fd) {
    short i;
    char input;
    struct ext2_super_block temp_super;

    if (is_attacked(super)) {
        printf("Disk error found:\nSuper-block magic number does not match and the disk could not be read.\nThe super-block might have been attacked or the file system is not ext2.\n\nDo you wish to attempt to recover it by using a backup on the disk? (Y/<N>)\n");
        scanf(" %c", &input);
        printf("\n");
        if (input == 'Y' || input == 'y') {
            for(i = 0; i < 3; i++) {
                printf("Checking backup at addr %d...\n", super_backup_locations[i]);
                lseek(fd, super_backup_locations[i] * 1024, SEEK_SET);
                read(fd, &temp_super, sizeof(temp_super));
                printf("%d\n", super_backup_locations[i]);
                if (!is_attacked(temp_super)) {
                    printf("Backup found, now copying...\n");
                    copy_super(fd, temp_super);
                    printf("Backup complete.\n");
                    return;
                } else {
                    printf("Could not be used.\n");
                }
            }
            printf("Backup could not be found.\n");
        }
    } else {
        printf("[Fun 1] OK: Magic number matches, disk was not attacked.\n");
        return;
    }
    printf("Could not complete fsck.\nExiting with errors.\n");    
    exit(EXIT_FAILURE);
}

void check_fun2(int fd) {
    int i, j, k, l;
    char input;
    unsigned int offset;
    struct ext2_group_desc temp_group_desc;
    struct ext2_inode temp_inode;
    unsigned char owned_blocks[super.s_blocks_count];

    for(i = 0; i < super.s_blocks_count; i++)
        owned_blocks[i] = 0;

    // Checking owned blocks
    for(i = 0; i < group_count; i++) {
        offset = BASE_OFFSET + group_size * i;
        printf("Checking for inodes in group %d, offset %d...\n", i, offset);
        lseek(fd, offset + block_size, SEEK_SET);  /* position head above the group descriptor block */
        read(fd, &temp_group_desc, sizeof(temp_group_desc));

        unsigned char *d_bmap = malloc(block_size);;
        unsigned char *i_bmap = malloc(block_size);

        lseek(fd, BLOCK_OFFSET(temp_group_desc.bg_block_bitmap), SEEK_SET);
        read(fd, d_bmap, block_size);

        lseek(fd, BLOCK_OFFSET(temp_group_desc.bg_inode_bitmap), SEEK_SET);
        read(fd, i_bmap, block_size);
        
        for(j = 1; j <= super.s_inodes_per_group; j++) {
            read_inode(fd, j, &temp_group_desc, i, &temp_inode);
            if(!TEST_BIT(i_bmap, j-1) || temp_inode.i_size <= 0) // check if inode is valid
                continue;
            owned_blocks[temp_inode.i_block[0]]++;
        }
        free(d_bmap);
        free(i_bmap);
    }


    for(l = 0; l < super.s_blocks_count; l++) {
        if (owned_blocks[l] > 1) {
            printf("Multiply-owned block found: %d\n", l);
            printf("Do you wish to delete one of its owners? (Y/<N>)\n");
            scanf(" %c", &input);
            printf("\n");
            if (input == 'Y' || input == 'y') {
                for(i = 0; i < group_count; i++) {
                    offset = BASE_OFFSET + group_size * i;
                    lseek(fd, offset + block_size, SEEK_SET);  /* position head above the group descriptor block */
                    read(fd, &temp_group_desc, sizeof(temp_group_desc));
            
                    unsigned char *d_bmap = malloc(block_size);;
                    unsigned char *i_bmap = malloc(block_size);
            
                    lseek(fd, BLOCK_OFFSET(temp_group_desc.bg_block_bitmap) + group_size * i, SEEK_SET);
                    read(fd, d_bmap, block_size);
            
                    lseek(fd, BLOCK_OFFSET(temp_group_desc.bg_inode_bitmap) + group_size * i, SEEK_SET);
                    read(fd, i_bmap, block_size);
                    
                    int found = 0;
                    for(j = 1; j <= super.s_inodes_per_group; j++) {
                        read_inode(fd, j, &temp_group_desc, i, &temp_inode);
                        if(!TEST_BIT(i_bmap, j-1) || temp_inode.i_size <= 0) // check if inode is valid
                            continue;
                        if (temp_inode.i_block[0] == l) {
                            found = 1;
                            break;
                        }
                    }
                    if(found) {
                        printf("Inode deleted.\n");
                        CLEAR_BIT(i_bmap, j-1);
                        lseek(fd, BLOCK_OFFSET(temp_group_desc.bg_inode_bitmap) + group_size * i, SEEK_SET);
                        write(fd, i_bmap, block_size);
                        owned_blocks[l]--;
                        free(d_bmap);
                        free(i_bmap);
                        break;
                    }
                    free(d_bmap);
                    free(i_bmap);
                }
            } else {
                printf("Could not complete fsck.\nExiting with errors.\n");    
                exit(EXIT_FAILURE);
            }
        }
    }


    for(i = 0; i < super.s_blocks_count; i++) {
        if(owned_blocks[i] > 1) {
            printf("Multiply-owned blocks still exist, checking again...\n");
            check_fun2(fd);
        }
    }

    printf("[Fun 2] OK: No multiply-owned blocks.\n");

}

void check_fun3(int fd) {
    short i, j, k;
    char input;
    unsigned int offset;
    struct ext2_group_desc temp_group_desc;
    struct ext2_inode temp_inode;

    for(i = 0; i < group_count; i++) {
        offset = block_size * super.s_blocks_per_group * i;
        lseek(fd, BASE_OFFSET + offset + block_size, SEEK_SET);  /* position head above the group descriptor block */
        read(fd, &temp_group_desc, sizeof(temp_group_desc));

        unsigned char *d_bmap;
        unsigned char *i_bmap;

        d_bmap = malloc(block_size);
        i_bmap = malloc(block_size);

        lseek(fd, BLOCK_OFFSET(temp_group_desc.bg_block_bitmap), SEEK_SET);
        read(fd, d_bmap, block_size);

        lseek(fd, BLOCK_OFFSET(temp_group_desc.bg_inode_bitmap), SEEK_SET);
        read(fd, i_bmap, block_size);
        
        for(j = 0; j < super.s_inodes_per_group; j++) {
            if(!TEST_BIT(i_bmap, j)) // check if inode is valid
                continue;
            printf("Reading inode %d/%d from group %d...\n",
                    j, super.s_inodes_per_group, i);
            read_inode(fd, j+1, &temp_group_desc, i, &temp_inode);
            printf("inode file mode %d, inode size %d, inode num %d\n", temp_inode.i_mode, temp_inode.i_size, j+1);            
            
            
            
            if (is_permission_corrupted(temp_inode)) {
                printf("Disk error found:\n[Fun 3] File permissions corrupted.\nSet permissions? (Y/<N>)");
                scanf("%c", &input);
                printf("\n");
                if (input == 'Y' || input == 'y') {
                    continue;
                } 
                else {
                  printf("Could not complete fsck.\nExiting with errors.\n");    
                  exit(EXIT_FAILURE);
                }
            }
        }
        free(d_bmap);
        free(i_bmap);
    }
    printf("[Fun 3] OK: Consistent permissions.\n");
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