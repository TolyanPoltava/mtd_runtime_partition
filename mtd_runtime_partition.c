#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <string.h>
#include <stdlib.h>
#include <linux/types.h>
#include <stdint.h>

#define MTDPARTITIONSHIFT   _IOW('M', 23, int)
#define MEMGETINFO          _IOR('M', 1, struct mtd_info_user)

#define MTD_NUMBER_DIGITS   (2)
#define MTD_READWRITE       (0)

struct user_mtd_info {
    __u32 mtd_num;
    __u64 partition_offset;
};

typedef struct mtd_info_user {
    __u8 type;
    __u32 flags;
    __u32 size;
    __u32 erasesize;
    __u32 writesize;
    __u32 oobsize;
    __u64 padding;
} mtd_info_t;

static const char *target_jffs_partition = "/dev/mtd1";

/*
*  Converts string to hex.
*/
static uint64_t get_mag(const char *p8_buf)
{
    int j = 0;
    uint64_t temp = 0;
    uint64_t data = 0;
    int count = 0;

    while (1) {
        if (p8_buf[count] == (char)0xff)
        {
            count -= 2;
            break;
        }
        else if (p8_buf[count] == '\0')
        {
            count -= 1;
            break;
        }
        count++;
    }

    while (count >= 0) {
        if ((p8_buf[count] >= '0') && (p8_buf[count] <= '9')) {
            temp = p8_buf[count] - '0';
            data |= (temp << (j*4));
        } else if ((p8_buf[count] >= 'a') && (p8_buf[count] <= 'f')) {
            temp = (p8_buf[count] - 'a') + 10;
            data |= (temp << (j*4));
        }
    j++;
    count--;
    }
    return data;
}

/*
*  look for mtd3 in /proc/mtd
*/
int already_resized()
{
    FILE *fp = fopen("/proc/mtd", "r");
    char tmp[64] = {0x0};
    while (fp != NULL && fgets(tmp, sizeof(tmp), fp) != NULL)
    {
        if (strstr(tmp, "mtd3"))
        {
            fclose(fp);
            return -1;
        }
    }
    if (fp != NULL)
        fclose(fp);
    return 0;
}

/*
*  argv[1] : Number of bytes by which to to increase mtd0 and shrink mtd1
*  return : zero on success
*/
int main(int u8_argc, char *p8_argv[])
{    
    if (u8_argc < 2) {
        fprintf(stderr, "Expands mtd0 by shrinking the beginning of mtd1.\n");
        fprintf(stderr, "mtdpart 0xabcdef\n");
        return -1;
    }

    if (already_resized())
    {
        fprintf(stderr, "Can only resize partitions once. Please reboot before trying again.\n");
        return -1;
    }

    int i32_Fd = open(target_jffs_partition, O_RDWR);
    if (i32_Fd < 0) {
        fprintf(stderr, "File open error\n");
        return -1;
    }
    else
        fprintf(stdout, "Opened %s\n", target_jffs_partition);

    mtd_info_t mtd_memory;

    /*Get MTD Erase size*/
    if (ioctl(i32_Fd, MEMGETINFO, &mtd_memory) != 0) {
        fprintf(stderr, "Error in reading MTD memory data\n");
        return -1;
    }

    uint64_t u64_mtd_erase_size = mtd_memory.erasesize;
    struct user_mtd_info user_mtd_data;

    /*Fill out partition number and offset*/
    user_mtd_data.mtd_num = 1;
    user_mtd_data.partition_offset = get_mag(p8_argv[1]);
    if ((user_mtd_data.partition_offset % u64_mtd_erase_size) != 0) {
        fprintf(stderr, "'Shift' size (%llu) must be block-alligned\n", user_mtd_data.partition_offset);
        return -1;
    }

    if (user_mtd_data.partition_offset == 0) {
        fprintf(stderr, "Invalid 'shift' size requested (%llu).\n", user_mtd_data.partition_offset);
        return -1;
    }

    /*Shift MTD partitions*/
    int err = ioctl(i32_Fd, MTDPARTITIONSHIFT, &user_mtd_data);
    if (err == 0)
        fprintf(stdout, "mtd partitions shifted\n");
    else
        fprintf(stderr, "Failed to shift MTD partitions: %d\n", err);

    close(i32_Fd);
    return 0;
}
