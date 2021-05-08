/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2012-2014 Daniel Schwierzeck <daniel.schwierzeck@sphairon.com>
 */

#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/gpio_buttons.h>
#include <linux/input.h>
#include <asm/mips_machine.h>
#include <asm/ifx/machtypes.h>

#include <ifx_devices.h>
#include <ifx_led.h>
#include <ifx_ledc.h>
#include <lantiq_spi.h>
#include <pef7071.h>

static struct ltq_spi_controller_data spi_cdata_m25p80 = {
	.gpio = 10,
};

static struct ltq_spi_controller_data spi_cdata_psb21150_fxs = {
	.gpio = 39,
};

static struct ltq_spi_controller_data spi_cdata_psb21150_fxo = {
	.gpio = 22,
};

static const struct peb3086_platform_data psb21150_pdata_fxs = {
	.irq_flags = IRQF_TRIGGER_LOW,
	.iom2_chan = 0,
	.mode = MODE_LTS,
	.dps = 1,
};

static const struct peb3086_platform_data psb21150_pdata_fxo = {
	.irq_flags = IRQF_TRIGGER_LOW,
	.iom2_chan = 1,
	.mode = MODE_TE,
	.pwr_src_pin = 1,
};

static struct spi_board_info spi_boardinfo_sl550x[] __initdata = {
	{
		.modalias = "m25p80",	/* SPI flash */
		.bus_num = 0,
		.chip_select = 4,
		.mode = SPI_MODE_0,
		.max_speed_hz = 25000000,
		.controller_data = &spi_cdata_m25p80,
	},
	{
		.modalias = "psb21150",	/* IPAC-X PSB 21150 FXS */
		.bus_num = 1,
		.chip_select = 0,
		.mode = SPI_MODE_3,
		.max_speed_hz = 1000000,
		.platform_data = &psb21150_pdata_fxs,
		.controller_data = &spi_cdata_psb21150_fxs,
		.irq = IFX_EIU_IR1,	/* EXIN1 on GPIO1 */
	},
};

static struct spi_board_info spi_boardinfo_sl6501[] __initdata = {
	{
		.modalias = "m25p80",	/* SPI flash */
		.bus_num = 0,
		.chip_select = 4,
		.mode = SPI_MODE_0,
		.max_speed_hz = 25000000,
		.controller_data = &spi_cdata_m25p80,
	},
	{
		.modalias = "psb21150",	/* IPAC-X PSB 21150 FXS */
		.bus_num = 1,
		.chip_select = 0,
		.mode = SPI_MODE_3,
		.max_speed_hz = 1000000,
		.platform_data = &psb21150_pdata_fxs,
		.controller_data = &spi_cdata_psb21150_fxs,
		.irq = IFX_EIU_IR1,	/* EXIN1 on GPIO1 */
	},
	{
		.modalias = "psb21150",	/* IPAC-X PSB 21150 FXO */
		.bus_num = 1,
		.chip_select = 1,
		.mode = SPI_MODE_3,
		.max_speed_hz = 1000000,
		.platform_data = &psb21150_pdata_fxo,
		.controller_data = &spi_cdata_psb21150_fxo,
		.irq = IFX_EIU_IR1,	/* EXIN1 on GPIO1 */
	},
};

static struct spi_board_info spi_boardinfo_sl4501[] __initdata = {
	{
		.modalias = "m25p80",	/* SPI flash */
		.bus_num = 0,
		.chip_select = 4,
		.mode = SPI_MODE_0,
		.max_speed_hz = 25000000,
		.controller_data = &spi_cdata_m25p80,
	},
};

static struct gpio_button gpio_buttons[] = {
	/* Reset */
	{
		.gpio = 47,
		.active_low = 1,
		.desc = "reset-btn",
		.type = EV_KEY,
		.code = BTN_0,
		.threshold = 0,
	},
	/* WLAN */
	{
		.gpio = 45,
		.active_low = 1,
		.desc = "wlan-btn",
		.type = EV_KEY,
		.code = BTN_1,
		.threshold = 0,
	},
	/* WPS */
	{
		.gpio = 46,
		.active_low = 1,
		.desc = "wps-btn",
		.type = EV_KEY,
		.code = BTN_2,
		.threshold = 0,
	},
};

static struct gpio_buttons_platform_data gpio_button_pdata __initdata = {
	.buttons = gpio_buttons,
	.nbuttons = ARRAY_SIZE(gpio_buttons),
	.poll_interval = 101,
};

static const struct stg_gpio_config stg_gpios[] = {
	{
		.name = "wlan-pwr",
		.gpio = 37,
		.active_low = 0,
		.default_state = STG_GPIO_DEFSTATE_OFF,
	},
	{
		.name = "wlan-rst",
		.gpio = 15,
		.active_low = 1,
		.default_state = STG_GPIO_DEFSTATE_ON,
	},
	{
		.name = "slic-rst",
		.gpio = 31,
		.active_low = 1,
		.default_state = STG_GPIO_DEFSTATE_ON,
	},
};

static struct stg_platform_data stg_gpio_pdata __initdata = {
	.gpios = stg_gpios,
	.gpio_num = ARRAY_SIZE(stg_gpios),
};

static struct ifx_ledc_config_param ledc_hw_config = {
	.operation_mask = IFX_LEDC_CFG_OP_UPDATE_SOURCE |
				IFX_LEDC_CFG_OP_BLINK |
				IFX_LEDC_CFG_OP_UPDATE_CLOCK |
				IFX_LEDC_CFG_OP_STORE_MODE |
				IFX_LEDC_CFG_OP_SHIFT_CLOCK |
				IFX_LEDC_CFG_OP_DATA_OFFSET |
				IFX_LEDC_CFG_OP_NUMBER_OF_LED |
				IFX_LEDC_CFG_OP_DATA |
				IFX_LEDC_CFG_OP_MIPS0_ACCESS |
				IFX_LEDC_CFG_OP_DATA_CLOCK_EDGE,
	.source_mask = (1 << 8) - 1,
	.source = (1 << IFX_LED_EXT_SRC_GPHY1_LED0) |
			(1 << IFX_LED_EXT_SRC_GPHY1_LED1) |
			(1 << IFX_LED_EXT_SRC_GPHY0_LED0) |
			(1 << IFX_LED_EXT_SRC_GPHY0_LED1),
	.blink_mask = (1 << 8) - 1,
	.blink = 0,
	.update_clock = LED_CON1_UPDATE_SRC_GPT,
	.fpid = 8 * 16 * 4,	/* 8 pins * 16 Hz * 4 */
	.store_mode = 0,
	.fpis = 0,
	.data_offset = 0,
	.number_of_enabled_led = 8,
	.data_mask = (1 << 8) - 1,
	.data = 0,
	.mips0_access_mask = (1 << 8) - 1,
	.mips0_access = (1 << 8) - 1,
	.f_data_clock_on_rising = 0,
};

static struct ifx_led_device led_hw_config[] = {
	{
		.name			= "green:phone",
		.phys_id		= 7,
		.value_on		= 1,
		.value_off		= 0,
		.flags			= IFX_LED_DEVICE_FLAG_PHYS_LEDC,
	},
	{
		.name			= "red:phone",
		.phys_id		= 4,
		.value_on		= 1,
		.value_off		= 0,
		.flags			= IFX_LED_DEVICE_FLAG_PHYS_LEDC,
	},
#if 0
	{
		.name			= "green:lan3",
		.phys_id		= 6,
		.value_on		= 1,
		.value_off		= 0,
		.flags			= IFX_LED_DEVICE_FLAG_PHYS_LEDC,
	},
	{
		.name			= "green:lan1",
		.phys_id		= 3,
		.value_on		= 1,
		.value_off		= 0,
		.flags			= IFX_LED_DEVICE_FLAG_PHYS_LEDC,
	},
	{
		.name			= "green:lan2",
		.phys_id		= 2,
		.value_on		= 1,
		.value_off		= 0,
		.flags			= IFX_LED_DEVICE_FLAG_PHYS_LEDC,
	},
#endif
	{
		.name			= "green:usb1",
		.phys_id		= 1,
		.value_on		= 1,
		.value_off		= 0,
		.flags			= IFX_LED_DEVICE_FLAG_PHYS_LEDC,
	},
	{
		.name			= "green:dsl-led1",
		.phys_id		= 0,
		.value_on		= 1,
		.value_off		= 0,
		.flags			= IFX_LED_DEVICE_FLAG_PHYS_LEDC,
	},
	{
		.flags			= IFX_LED_DEVICE_FLAG_INVALID,
	}
};

static struct gpio_led gpio_leds_sl550x[] = {
	{
		.name = "green:power",
		.gpio = 3,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:power",
		.gpio = 19,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name = "green:info",
		.gpio = 20,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:info",
		.gpio = 21,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:wlan",
		.gpio = 29,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:wps",
		.gpio = 30,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:web",
		.gpio = 27,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:web",
		.gpio = 28,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:usb2",
		.gpio = 32,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
};

static struct gpio_led gpio_leds_sl6501[] = {
	{
		.name = "green:power",
		.gpio = 3,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:power",
		.gpio = 19,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name = "green:info",
		.gpio = 20,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:info",
		.gpio = 21,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:wlan",
		.gpio = 29,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:wps",
		.gpio = 30,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:web",
		.gpio = 27,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:web",
		.gpio = 28,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:usb2",
		.gpio = 32,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:fxo",
		.gpio = 14,
		.active_low = 0,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
};

static struct gpio_led gpio_leds_sl4501[] = {
	{
		.name = "green:power",
		.gpio = 3,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:power",
		.gpio = 19,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name = "green:info",
		.gpio = 20,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:info",
		.gpio = 21,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:wlan",
		.gpio = 29,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:wps",
		.gpio = 30,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "green:web",
		.gpio = 27,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = "red:web",
		.gpio = 28,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
};

static struct gpio_led_platform_data gpio_led_pdata_sl550x __initdata = {
	.leds = gpio_leds_sl550x,
	.num_leds = ARRAY_SIZE(gpio_leds_sl550x),
};

static struct gpio_led_platform_data gpio_led_pdata_sl6501 __initdata = {
	.leds = gpio_leds_sl6501,
	.num_leds = ARRAY_SIZE(gpio_leds_sl6501),
};

static struct gpio_led_platform_data gpio_led_pdata_sl4501 __initdata = {
	.leds = gpio_leds_sl4501,
	.num_leds = ARRAY_SIZE(gpio_leds_sl4501),
};

static struct xway_switch_phy xmii0_phy_ge = {
	.interface = PHY_INTERFACE_MODE_RGMII,
	.irq = IFX_EIU_IR2,
	.irq_flags = IRQF_TRIGGER_LOW,
	.addr = 0x0,
};

static struct xway_switch_phy xmii1_phy_ge = {
	.interface = PHY_INTERFACE_MODE_RGMII,
	.irq = IFX_EIU_IR2,
	.irq_flags = IRQF_TRIGGER_LOW,
	.addr = 0x1,
};

static struct xway_switch_phy gphy0_fe0 = {
	.interface = PHY_INTERFACE_MODE_MII,
	.irq = INT_NUM_IM3_IRL18,
	.addr = 0x11,
};

static struct xway_switch_phy gphy0_fe1 = {
	.interface = PHY_INTERFACE_MODE_MII,
	.irq = INT_NUM_IM3_IRL18,
	.addr = 0x12,
};

static struct xway_switch_phy gphy0_ge = {
	.interface = PHY_INTERFACE_MODE_GMII,
	.irq = INT_NUM_IM3_IRL18,
	.addr = 0x11,
};

static struct xway_switch_phy gphy1_ge = {
	.interface = PHY_INTERFACE_MODE_GMII,
	.irq = INT_NUM_IM3_IRL17,
	.addr = 0x13,
};

static struct xway_switch_mac xmii2_mac_ge = {
	.interface = PHY_INTERFACE_MODE_RGMII,
	.speed = SPEED_1000,
	.duplex = DUPLEX_FULL,
};

static struct xway_switch_xmii switch_xmii_sl550x[] = {
	{
		.dev_name = "sw-p0-wanoe",
		.ppid = 0,
		.phy = &xmii0_phy_ge,
	},
	{
		.dev_name = "sw-p1-lan4",
		.ppid = 1,
		.phy = &xmii1_phy_ge,
	},
	{
		.dev_name = "sw-p5-wifi",
		.ppid = 5,
		.rgmii_tx_delay = 3,
		.mac = &xmii2_mac_ge,
	},
};

static struct xway_switch_xmii switch_xmii_sl6501[] = {
	{
		.dev_name = "sw-p1-lan4",
		.ppid = 1,
		.phy = &xmii1_phy_ge,
	},
	{
		.dev_name = "sw-p5-wifi",
		.ppid = 5,
		.rgmii_tx_delay = 3,
		.mac = &xmii2_mac_ge,
	},
};

static struct xway_switch_xmii switch_xmii_sl4501[] = {
	{
		.dev_name = "sw-p5-wifi",
		.ppid = 5,
		.rgmii_tx_delay = 3,
		.mac = &xmii2_mac_ge,
	},
};

static struct xway_switch_gphy switch_gphy_sl550x[] = {
	{
		.dev_name = "sw-p2-lan1",
		.ppid = 2,
		.phy = &gphy0_fe0,
	},
	{
		.dev_name = "sw-p3-lan2",
		.ppid = 3,
		.phy = &gphy0_fe1,
	},
	{
		.dev_name = "sw-p4-lan3",
		.ppid = 4,
		.phy = &gphy1_ge,
	},
};

static struct xway_switch_gphy switch_gphy_sl4501[] = {
	{
		.dev_name = "sw-p2-lan1",
		.ppid = 2,
		.phy = &gphy0_ge,
	},
	{
		.dev_name = "sw-p4-lan2",
		.ppid = 4,
		.phy = &gphy1_ge,
	},
};

static struct xway_switch_gphy_core switch_gphy_core_sl550x[] = {
	{
		.mode = GPHY_22F,
		.id = 0,
	},
	{
		.mode = GPHY_11G,
		.id = 1,
	},
};

static struct xway_switch_gphy_core switch_gphy_core_sl4501[] = {
	{
		.mode = GPHY_11G,
		.id = 0,
	},
	{
		.mode = GPHY_11G,
		.id = 1,
	},
};

static struct xway_switch_data switch_data_sl550x = {
	.xmii = switch_xmii_sl550x,
	.num_xmii = ARRAY_SIZE(switch_xmii_sl550x),
	.gphy = switch_gphy_sl550x,
	.num_gphy = ARRAY_SIZE(switch_gphy_sl550x),
	.gphy_core = switch_gphy_core_sl550x,
	.num_gphy_core = ARRAY_SIZE(switch_gphy_core_sl550x),
};

static struct xway_switch_data switch_data_sl6501 = {
	.xmii = switch_xmii_sl6501,
	.num_xmii = ARRAY_SIZE(switch_xmii_sl6501),
	.gphy = switch_gphy_sl550x,
	.num_gphy = ARRAY_SIZE(switch_gphy_sl550x),
	.gphy_core = switch_gphy_core_sl550x,
	.num_gphy_core = ARRAY_SIZE(switch_gphy_core_sl550x),
};

static struct xway_switch_data switch_data_sl4501 = {
	.xmii = switch_xmii_sl4501,
	.num_xmii = ARRAY_SIZE(switch_xmii_sl4501),
	.gphy = switch_gphy_sl4501,
	.num_gphy = ARRAY_SIZE(switch_gphy_sl4501),
	.gphy_core = switch_gphy_core_sl4501,
	.num_gphy_core = ARRAY_SIZE(switch_gphy_core_sl4501),
};

static struct sph_usb_oc_data usb0_oc_data = {
	.oc_irq = IFX_USB0_OCIR,
	.port_power_gpio = {
		.name = "usb0-pwr",
		.gpio = 41,
		.active_low = 0,
		.default_state = STG_GPIO_DEFSTATE_OFF,
	},
};

static struct sph_usb_oc_data usb1_oc_data = {
	.oc_irq = IFX_USB1_OCIR,
	.port_power_gpio = {
		.name = "usb1-pwr",
		.gpio = 33,
		.active_low = 0,
		.default_state = STG_GPIO_DEFSTATE_OFF,
	},
};

static int sl550x_phy_fixup(struct phy_device *phydev)
{
	unsigned int led0h = 0, led0l = 0, led1h = 0, led1l = 0;

	pr_info("%s: phydev->addr %0x\n", __func__, phydev->addr);

	switch (phydev->addr) {
	case 0x00:
	case 0x01:
		led0h = 0x70;
		led0l = 0x03;
		break;
	case 0x11:
	/*
	 *  case 0x12: both FE PHYs share the same LED interface thus only one
	 *      	setup is necessary
	 */
		/* FE PHY0 works on LED0 */
		led0h = 0x30;
		led0l = 0x03;
		/* FE PHY1 works on LED1 */
		led1h = 0x30;
		led1l = 0x03;
		break;
	case 0x13:
		led0h = 0x70;
		led0l = 0x03;
		break;
	default:
		return 0;
	}

	/* LED0 */
	pef7071_mmd_write(phydev, 0x1e2, led0h);
	pef7071_mmd_write(phydev, 0x1e3, led0l);

	/* LED1 */
	pef7071_mmd_write(phydev, 0x1e4, led1h);
	pef7071_mmd_write(phydev, 0x1e5, led1l);

	/* LED2 */
	pef7071_mmd_write(phydev, 0x1e6, 0x00);
	pef7071_mmd_write(phydev, 0x1e7, 0x00);

	return 0;
}

static int sl4501_phy_fixup(struct phy_device *phydev)
{
	pr_info("%s: phydev->addr %0x\n", __func__, phydev->addr);

	/* LED0 */
	pef7071_mmd_write(phydev, 0x1e2, 0x70);
	pef7071_mmd_write(phydev, 0x1e3, 0x03);

	/* LED1 */
	pef7071_mmd_write(phydev, 0x1e4, 0x00);
	pef7071_mmd_write(phydev, 0x1e5, 0x00);

	/* LED2 */
	pef7071_mmd_write(phydev, 0x1e6, 0x00);
	pef7071_mmd_write(phydev, 0x1e7, 0x00);

	return 0;
}

static void __init sl550x_phy_init(void)
{
	phy_register_fixup_for_uid(0xd565a400, 0xffffff00, sl550x_phy_fixup);
}

static void __init sl4501_phy_init(void)
{
	phy_register_fixup_for_uid(0xd565a408, 0xfffffff8, sl4501_phy_fixup);
}

static void __init sl4501_gpio_init(void)
{
	/* LEDC/LED_ST for LED shift register */
	gpio_set_altfunc(4, 1, 0, 1);
	gpio_set_opendrain(4, LTQ_GPIO_OD_NORMAL);
	/* LEDC/LED_D for LED shift register */
	gpio_set_altfunc(5, 1, 0, 1);
	gpio_set_opendrain(5, LTQ_GPIO_OD_NORMAL);
	/* LEDC/LED_SH for LED shift register */
	gpio_set_altfunc(6, 1, 0, 1);
	gpio_set_opendrain(6, LTQ_GPIO_OD_NORMAL);

	/* TDM/FSC as input, internal pull-up */
	gpio_set_altfunc(0, 0, 0, 0);
	gpio_set_pull(0, LTQ_GPIO_PULL_UP);
	/* TDM/DCL as input, internal pull-up */
	gpio_set_altfunc(40, 0, 0, 0);
	gpio_set_pull(40, LTQ_GPIO_PULL_UP);
	/* TDM/DI as input */
	gpio_set_altfunc(26, 0, 0, 0);
	/* TDM/DO as input */
	gpio_set_altfunc(25, 0, 0, 0);

	/* SLIC/SSIO_CLK as input */
	gpio_set_altfunc(36, 0, 1, 1);
	/* SLIC/SSIO_TX as output */
	gpio_set_altfunc(34, 0, 1, 1);
	/* SLIC/SSIO_RX as input */
	gpio_set_altfunc(35, 0, 1, 1);
	/* SLIC/CLKOUT0 as output */
	gpio_set_altfunc(8, 1, 0, 1);
}

static void __init sl550x_gpio_init(void)
{
	/* EXIN1 on GPIO1 for Lantiq IPAC-X PSB 21150 */
	gpio_set_altfunc(1, 1, 0, 0);

	/* EXIN2 on GPIO2 for Lantiq PEF7071 PHYs */
	gpio_set_altfunc(2, 0, 1, 0);
}

static void __init sl550x_setup(void)
{
	sl4501_gpio_init();
	sl550x_gpio_init();
	sl550x_phy_init();
	ifx_register_spi(NULL, spi_boardinfo_sl550x,
		ARRAY_SIZE(spi_boardinfo_sl550x));
	ifx_register_usif_spi(NULL);
	ifx_register_ledc(&ledc_hw_config);
	ifx_register_led(led_hw_config, sizeof(led_hw_config));
	ifx_register_gpio_buttons(&gpio_button_pdata);
	ifx_register_gpio_leds(&gpio_led_pdata_sl550x);
	ifx_register_stg_platform(&stg_gpio_pdata);
	ifx_register_dma();
	ifx_register_wdt();
	ifx_register_vr9_switch(&switch_data_sl550x);
	ifx_register_vmmc();
	ifx_register_usb_oc(0, &usb0_oc_data);
	ifx_register_usb_oc(1, &usb1_oc_data);
}

static void __init sl6501_setup(void)
{
	sl4501_gpio_init();
	sl550x_gpio_init();
	sl550x_phy_init();
	ifx_register_spi(NULL, spi_boardinfo_sl6501,
		ARRAY_SIZE(spi_boardinfo_sl6501));
	ifx_register_usif_spi(NULL);
	ifx_register_ledc(&ledc_hw_config);
	ifx_register_led(led_hw_config, sizeof(led_hw_config));
	ifx_register_gpio_buttons(&gpio_button_pdata);
	ifx_register_gpio_leds(&gpio_led_pdata_sl6501);
	ifx_register_stg_platform(&stg_gpio_pdata);
	ifx_register_dma();
	ifx_register_wdt();
	ifx_register_vr9_switch(&switch_data_sl6501);
	ifx_register_vmmc();
	ifx_register_usb_oc(0, &usb0_oc_data);
	ifx_register_usb_oc(1, &usb1_oc_data);
}

static void __init sl4501_setup(void)
{
	sl4501_gpio_init();
	sl4501_phy_init();
	ifx_register_spi(NULL, spi_boardinfo_sl4501,
		ARRAY_SIZE(spi_boardinfo_sl4501));
	ifx_register_ledc(&ledc_hw_config);
	ifx_register_led(led_hw_config, sizeof(led_hw_config));
	ifx_register_gpio_buttons(&gpio_button_pdata);
	ifx_register_gpio_leds(&gpio_led_pdata_sl4501);
	ifx_register_stg_platform(&stg_gpio_pdata);
	ifx_register_dma();
	ifx_register_wdt();
	ifx_register_vr9_switch(&switch_data_sl4501);
	ifx_register_vmmc();
}

MIPS_MACHINE(LTQ_MACH_SPHSL550X,
		"SPHSL550X", "Speedlink 5501", sl550x_setup);

MIPS_MACHINE(LTQ_MACH_SPHSL6501,
		"SPHSL6501", "Speedlink 6501", sl6501_setup);

MIPS_MACHINE(LTQ_MACH_SPHSL4501,
		"SPHSL4501", "Speedlink 4501", sl4501_setup);
