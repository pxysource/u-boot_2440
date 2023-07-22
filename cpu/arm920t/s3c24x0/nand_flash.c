/**
 * @file nand_flash.c
 * @author panxingyuan (panxingyuan1@163.com)
 * @brief 
 * @version 0.1
 * @date 2023-07-02 18:23:32
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <common.h>
#include <nand.h>

#define S3C2440_NFSTAT_READY (1 << 0)

typedef volatile unsigned int S3C2440_REG32;

typedef struct {
    S3C2440_REG32   NFCONF;         /** NAND flash configuration register. */
    S3C2440_REG32   NFCONT;         /** NAND flash control register. */
    S3C2440_REG32   NFCMMD;
    S3C2440_REG32   NFADDR;
    S3C2440_REG32   NFDATA;
    S3C2440_REG32   NFMECCD0;
    S3C2440_REG32   NFMECCD1;
    S3C2440_REG32   NFSECCD;
    S3C2440_REG32   NFSTAT;
    S3C2440_REG32   NFESTAT0;
    S3C2440_REG32   NFESTAT1;
    S3C2440_REG32   NFMECC0;
    S3C2440_REG32   NFMECC1;
    S3C2440_REG32   NFSECC;
    S3C2440_REG32   NFSBLK;
    S3C2440_REG32   NFEBLK;
} S3C2440_NAND;

static S3C2440_NAND *s3c2440nand = NULL;

static void s3c2440_nand_hwcontrol(struct mtd_info *mtd, int cmd)
{
    struct nand_chip *chip = mtd->priv;

    switch (cmd)
    {
        case NAND_CTL_SETNCE:
        case NAND_CTL_CLRNCE:
            break;
        case NAND_CTL_SETCLE:
            chip->IO_ADDR_W = (void *)&s3c2440nand->NFCMMD;
            break;
        case NAND_CTL_SETALE:
            chip->IO_ADDR_W = (void *)&s3c2440nand->NFADDR;
            break;
        default:
            chip->IO_ADDR_W = (void *)&s3c2440nand->NFDATA;
            break;
    }

    return;
}

static int s3c2440_nand_dev_ready (struct mtd_info *mtd)
{
    return s3c2440nand->NFSTAT & S3C2440_NFSTAT_READY;
}

static void s3c2440_nand_select_chip(struct mtd_info *mtd, int chip)
{
    if (chip == 0)
    {
        int i;

        /*  
         * bit1: NAND Flash Memory nFCE signal control.
         * Force nFCE to low (Enable chip select).
         */
        s3c2440nand->NFCONT &= ~(1 << 1); 
    }
    else
    {
        /*
         * bit1: NAND Flash Memory nFCE signal control.
         * Force nFCE to high (Disable chip select).
         */
        s3c2440nand->NFCONT |= (1 << 1);  
    }
}

void board_nand_init(struct nand_chip *nand)
{
#define TCALS   0
#define TWRPH0  4
#define TWRPH1  2

    s3c2440nand = (S3C2440_NAND *)0x4e000000;
    /* 设置时序. */
    s3c2440nand->NFCONF = (TCALS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);
    /* 
     * 使能NAND Flash controller.
     * 使能片选.
     * 初始化ECC.
     */
    s3c2440nand->NFCONT = (1 << 4) | (0 << 1) | (1 << 0);

    nand->eccmode = NAND_ECC_SOFT;
    nand->IO_ADDR_R = &s3c2440nand->NFDATA;
    nand->IO_ADDR_W = &s3c2440nand->NFDATA;
    nand->select_chip = s3c2440_nand_select_chip;
    nand->hwcontrol = s3c2440_nand_hwcontrol;
    nand->dev_ready = s3c2440_nand_dev_ready;
    nand->options = 0;
}
