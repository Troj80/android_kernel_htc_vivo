/* Copyright (c) 2010, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>
#include <mach/irqs.h>
#include <mach/gpiomux.h>

static struct gpiomux_setting keypad_power_key_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config vivo_keypad_configs[] __initdata = {
	{
		.gpio = 46,		/* VIVO_GPIO_KEYPAD_POWER_KEY */
		.settings = {
			[GPIOMUX_SUSPENDED] = &keypad_power_key_cfg,
		},
	},
};

static struct gpiomux_setting usb_id_output_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct msm_gpiomux_config usb_id_output_configs[] __initdata = {
	{
		.gpio = 49,		/* VIVO_GPIO_USB_ID_PIN */
		.settings = {
			[GPIOMUX_SUSPENDED] = &usb_id_output_cfg,
		},
	},
};

static struct gpiomux_setting usb_id_input_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting usb_id1_input_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config usb_id_input_configs[] __initdata = {
	{
		.gpio = 49,		/* VIVO_GPIO_USB_ID_PIN */
		.settings = {
			[GPIOMUX_SUSPENDED] = &usb_id_input_cfg,
		},
	},
	{
		.gpio = 145,		/* VIVO_GPIO_USB_ID1_PIN */
		.settings = {
			[GPIOMUX_SUSPENDED] = &usb_id1_input_cfg,
		},
	},
};

static struct gpiomux_setting mdp_vsync_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config msm7x30_vsync_configs[] __initdata = {
	{
		.gpio = 30,		/* VSYNC */
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdp_vsync_cfg,
		},
	},
};

static int __init vivo_gpiomux_init(void)
{
	int rc = msm_gpiomux_init(NR_GPIO_IRQS);

	if (rc) {
		pr_err(KERN_ERR "msm_gpiomux_init failed %d\n", rc);
		return rc;
	}

	msm_gpiomux_install(msm7x30_vsync_configs,
			ARRAY_SIZE(msm7x30_vsync_configs));

	msm_gpiomux_install(vivo_keypad_configs,
			ARRAY_SIZE(vivo_keypad_configs));

	msm_gpiomux_install(usb_id_output_configs,
			ARRAY_SIZE(usb_id_output_configs));

	msm_gpiomux_install(usb_id_input_configs,
			ARRAY_SIZE(usb_id_input_configs));

	return rc;
}

postcore_initcall(vivo_gpiomux_init);
