/* linux/arch/arm/mach-msm/board-vivo-mmc.c
 *
 * Copyright (C) 2008 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/gpio.h>
#include <linux/mfd/pmic8058.h>

#include <linux/io.h>

#include <mach/vreg.h>

#include <asm/mach/mmc.h>

#include "devices.h"
#include "board-vivo.h"
#include <mach/proc_comm.h>

#define VIVO_SDMC_CD_N_TO_SYS PM8058_GPIO_PM_TO_SYS(VIVO_GPIO_SDMC_CD_N)

extern int msm_add_sdcc(unsigned int controller, struct mmc_platform_data *plat);

#define VIVO_MMC_VDD		(MMC_VDD_28_29 | MMC_VDD_29_30)

/* ---- WIFI ---- */

static uint32_t wifi_on_gpio_table[] = {
	GPIO_CFG(116, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* DAT3 */
	GPIO_CFG(117, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* DAT2 */
	GPIO_CFG(118, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* DAT1 */
	GPIO_CFG(119, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* DAT0 */
	GPIO_CFG(111, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), /* CMD */
	GPIO_CFG(110, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), /* CLK */
	GPIO_CFG(147, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_4MA), /* WLAN IRQ */
};

static uint32_t wifi_off_gpio_table[] = {
	GPIO_CFG(116, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* DAT3 */
	GPIO_CFG(117, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* DAT2 */
	GPIO_CFG(118, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* DAT1 */
	GPIO_CFG(119, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* DAT0 */
	GPIO_CFG(111, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_4MA), /* CMD */
	GPIO_CFG(110, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_4MA), /* CLK */
	GPIO_CFG(147, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_4MA), /* WLAN IRQ */
};

static void config_gpio_table(uint32_t *table, int len)
{
		int n, rc;
		for (n = 0; n < len; n++) {
				rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
				if (rc) {
						pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
								__func__, table[n], rc);
						break;
				}
		}
}

/* BCM4329 returns wrong sdio_vsn(1) when we read cccr,
 * we use predefined value (sdio_vsn=2) here to initial sdio driver well
 */
static struct embedded_sdio_data vivo_wifi_emb_data = {
	.cccr	= {
		.sdio_vsn	= 2,
		.multi_block	= 1,
		.low_speed	= 0,
		.wide_bus	= 0,
		.high_power	= 1,
		.high_speed	= 1,
	},
};

static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;

static int
vivo_wifi_status_register(void (*callback)(int card_present, void *dev_id),
				void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static int vivo_wifi_cd;	/* WiFi virtual 'card detect' status */

static unsigned int vivo_wifi_status(struct device *dev)
{
	return vivo_wifi_cd;
}

static struct mmc_platform_data vivo_wifi_data = {
	.ocr_mask		= MMC_VDD_28_29,
	.status			= vivo_wifi_status,
	.register_status_notify	= vivo_wifi_status_register,
	.embedded_sdio		= &vivo_wifi_emb_data,
		.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
		.msmsdcc_fmin   = 144000,
		.msmsdcc_fmid   = 24576000,
		.msmsdcc_fmax   = 49152000,
		.nonremovable   = 0,
};

int vivo_wifi_set_carddetect(int val)
{
	printk(KERN_INFO "%s: %d\n", __func__, val);
	vivo_wifi_cd = val;
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		printk(KERN_WARNING "%s: Nobody to notify\n", __func__);
	return 0;
}
EXPORT_SYMBOL(vivo_wifi_set_carddetect);

int vivo_wifi_power(int on)
{
	printk(KERN_INFO "%s: %d\n", __func__, on);

	if (on) {
		config_gpio_table(wifi_on_gpio_table,
				ARRAY_SIZE(wifi_on_gpio_table));
	} else {
		config_gpio_table(wifi_off_gpio_table,
				ARRAY_SIZE(wifi_off_gpio_table));
	}

	/*vivo_wifi_bt_sleep_clk_ctl(on, ID_WIFI);*/
	gpio_set_value(VIVO_GPIO_WIFI_SHUTDOWN_N, on); /* WIFI_SHUTDOWN */
	mdelay(120);
	return 0;
}
EXPORT_SYMBOL(vivo_wifi_power);

int vivo_wifi_reset(int on)
{
	printk(KERN_INFO "%s: do nothing\n", __func__);
	return 0;
}

int __init vivo_init_mmc(unsigned int sys_rev)
{
	uint32_t id;
	wifi_status_cb = NULL;
	/*sdslot_vreg_enabled = 0;*/

	printk(KERN_INFO "vivo: %s\n", __func__);

	/* initial WIFI_SHUTDOWN# */
	id = GPIO_CFG(VIVO_GPIO_WIFI_SHUTDOWN_N, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &id, 0);
	gpio_set_value(VIVO_GPIO_WIFI_SHUTDOWN_N, 0);

	msm_add_sdcc(3, &vivo_wifi_data);
	/* reset eMMC for write protection test */
	gpio_set_value(VIVO_GPIO_EMMC_RST, 0);	/* this should not work!!! */
	udelay(100);
	gpio_set_value(VIVO_GPIO_EMMC_RST, 1);

	return 0;
}
