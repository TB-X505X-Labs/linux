// SPDX-License-Identifier: GPL-2.0
/*
 * camss-csiphy-snps-10-0.c
 *
 * Qualcomm MSM Camera Subsystem - CSIPHY Module 10.0 (Synopsys D-PHY)
 *
 * Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
 */

#include "camss.h"
#include "camss-csiphy.h"

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#define SNPS_PHY_B_OFFSET		0x800

#define SNPS_RX_SYS_7_00		0x020
#define SNPS_RX_CLK_LANE_3_00		0x0c0
#define SNPS_RX_CLK_LANE_4_00		0x0c4
#define SNPS_RX_CLK_LANE_6_00		0x0c8
#define SNPS_RX_CLK_LANE_7_00		0x0cc
#define SNPS_RX_LANE_0_7_00		0x12c
#define SNPS_RX_LANE0_DDL_2_00		0x1f8
#define SNPS_RX_LANE0_DDL_3_00		0x1fc
#define SNPS_RX_LANE_1_7_00		0x220
#define SNPS_RX_LANE_1_10_00		0x22c
#define SNPS_RX_LANE_1_11_00		0x230
#define SNPS_RX_STARTUP_OBS_2_00	0x324
#define SNPS_RX_STARTUP_OVR_1_00	0x384
#define SNPS_RX_STARTUP_OVR_2_00	0x388
#define SNPS_RX_STARTUP_OVR_3_00	0x38c
#define SNPS_RX_STARTUP_OVR_4_00	0x390
#define SNPS_RX_STARTUP_OVR_5_00	0x394
#define SNPS_RX_DUAL_PHY_0_00		0x4cc
#define SNPS_RX_CB_2_00			0x6b0

#define SNPS_SYS_CTRL			0x560
#define SNPS_CTRL_1			0x564
#define SNPS_CTRL_2			0x568
#define SNPS_CTRL_3			0x56c
#define SNPS_ENABLE			0x570
#define SNPS_ENABLE_CLK			0x574
#define SNPS_BASEDIR			0x578
#define SNPS_FORCE_MODE			0x57c
#define SNPS_FIFO_CTRL			0x580
#define SNPS_GLBL_IRQ_CMD		0x588
#define SNPS_IRQ_MASK_LANE_0		0x58c
#define SNPS_IRQ_CLEAR_0		0x59c
#define SNPS_SYS_CTRL_1			0x5ac
#define SNPS_IRQ_STATUS_0		0x5b4
#define SNPS_IRQ_CLK_CLEAR_0		0x5d0
#define SNPS_IRQ_CLK_STATUS_0		0x5d8
#define SNPS_IRQ_MASK_LANE_CLK_0	0x5c8

#define SNPS_LANE_MASK_AGGR		0x1f
#define SNPS_LANE_MASK_PHY_A		0x13
#define SNPS_LANE_MASK_PHY_B		0x2c

enum snps_mode {
	SNPS_PHY_A,
	SNPS_PHY_B,
	SNPS_AGGREGATE,
};

struct snps_freq {
	u16 rate;
	u8 hs_freq;
	u16 osc_freq;
};

static const struct snps_freq snps_freqs[] = {
	{ 80, 0x00, 460 }, { 90, 0x10, 460 }, { 100, 0x20, 460 },
	{ 110, 0x30, 460 }, { 120, 0x01, 460 }, { 130, 0x11, 460 },
	{ 140, 0x21, 460 }, { 150, 0x31, 460 }, { 160, 0x02, 460 },
	{ 170, 0x12, 460 }, { 180, 0x22, 460 }, { 190, 0x32, 460 },
	{ 205, 0x03, 460 }, { 220, 0x13, 460 }, { 235, 0x23, 460 },
	{ 250, 0x33, 460 }, { 275, 0x04, 460 }, { 300, 0x14, 460 },
	{ 325, 0x25, 460 }, { 350, 0x35, 460 }, { 400, 0x05, 460 },
	{ 450, 0x16, 460 }, { 500, 0x26, 460 }, { 550, 0x37, 460 },
	{ 600, 0x07, 460 }, { 650, 0x18, 460 }, { 700, 0x28, 460 },
	{ 750, 0x39, 460 }, { 800, 0x09, 460 }, { 850, 0x19, 460 },
	{ 900, 0x29, 460 }, { 950, 0x3a, 460 }, { 1000, 0x0a, 460 },
	{ 1050, 0x1a, 460 }, { 1100, 0x2a, 460 }, { 1150, 0x3b, 460 },
	{ 1200, 0x0b, 460 }, { 1250, 0x1b, 460 }, { 1300, 0x2b, 460 },
	{ 1350, 0x3c, 460 }, { 1400, 0x0c, 460 }, { 1450, 0x1c, 460 },
	{ 1500, 0x2c, 460 }, { 1550, 0x3d, 285 }, { 1600, 0x0d, 295 },
	{ 1650, 0x1d, 304 }, { 1700, 0x2e, 313 }, { 1750, 0x3e, 322 },
	{ 1800, 0x0e, 331 }, { 1850, 0x1e, 341 }, { 1900, 0x2f, 350 },
	{ 1950, 0x3f, 359 }, { 2000, 0x0f, 368 }, { 2050, 0x40, 377 },
	{ 2100, 0x41, 387 }, { 2150, 0x42, 396 }, { 2200, 0x43, 405 },
	{ 2250, 0x44, 414 }, { 2300, 0x45, 423 }, { 2350, 0x46, 432 },
	{ 2400, 0x47, 442 }, { 2450, 0x48, 451 }, { 2500, 0x49, 460 },
};

static void snps_set_bits(struct csiphy_device *csiphy, u32 reg,
			  u32 set, u32 clear)
{
	u32 val = readl_relaxed(csiphy->base + reg);

	val &= ~clear;
	val |= set;
	writel_relaxed(val, csiphy->base + reg);
}

static u8 csiphy_get_lane_mask(struct csiphy_lanes_cfg *lane_cfg)
{
	u8 lane_mask = 0;
	int i;

	for (i = 0; i < lane_cfg->num_data; i++)
		lane_mask |= BIT(lane_cfg->data[i].pos);

	if (lane_mask & 0x3)
		lane_mask |= BIT(4);
	if (lane_mask & 0xc)
		lane_mask |= BIT(5);

	return lane_mask;
}

static void csiphy_hw_version_read(struct csiphy_device *csiphy,
				   struct device *dev)
{
	dev_dbg(dev, "CSIPHY SNPS 10.0\n");
}

static void csiphy_reset(struct csiphy_device *csiphy)
{
	writel_relaxed(0x1, csiphy->base + SNPS_GLBL_IRQ_CMD);
	usleep_range(5000, 8000);
	writel_relaxed(0x0, csiphy->base + SNPS_GLBL_IRQ_CMD);
}

static void snps_irq_config(struct csiphy_device *csiphy, bool enable)
{
	u32 val = enable ? 0xff : 0;
	int i;

	for (i = 0; i < 4; i++)
		writel_relaxed(val, csiphy->base + SNPS_IRQ_MASK_LANE_0 + 4 * i);
	for (i = 0; i < 2; i++)
		writel_relaxed(val, csiphy->base + SNPS_IRQ_MASK_LANE_CLK_0 + 4 * i);
}

static const struct snps_freq *snps_find_freq(u32 rate)
{
	u32 diff = abs((int)snps_freqs[0].rate - (int)rate);
	int i;

	for (i = 1; i < ARRAY_SIZE(snps_freqs); i++) {
		u32 d = abs((int)snps_freqs[i].rate - (int)rate);

		if (d > diff)
			return &snps_freqs[i - 1];
		diff = d;
	}

	return &snps_freqs[ARRAY_SIZE(snps_freqs) - 1];
}

static void snps_2_lane_config(struct csiphy_device *csiphy,
			       enum snps_mode phy, u32 lane_rate)
{
	const struct snps_freq *f = snps_find_freq(lane_rate);
	u32 off = phy == SNPS_PHY_B ? SNPS_PHY_B_OFFSET : 0;

	if (phy == SNPS_PHY_A) {
		writel_relaxed(0x9, csiphy->base + SNPS_SYS_CTRL);
		writel_relaxed(f->hs_freq & 0x7f, csiphy->base + SNPS_CTRL_3);
	} else {
		writel_relaxed(0x9, csiphy->base + SNPS_SYS_CTRL_1);
		writel_relaxed(f->hs_freq & 0x7f, csiphy->base + SNPS_CTRL_2);
	}

	snps_set_bits(csiphy, SNPS_RX_SYS_7_00 + off, BIT(5), 0);
	snps_set_bits(csiphy, SNPS_RX_CLK_LANE_6_00 + off, BIT(7), 0);

	writel_relaxed(0x1, csiphy->base + SNPS_RX_STARTUP_OVR_4_00 + off);
	writel_relaxed(f->osc_freq & 0xff,
		       csiphy->base + SNPS_RX_STARTUP_OVR_2_00 + off);
	writel_relaxed((f->osc_freq & 0xf00) >> 8,
		       csiphy->base + SNPS_RX_STARTUP_OVR_3_00 + off);
	writel_relaxed(0x1, csiphy->base + SNPS_RX_STARTUP_OVR_5_00 + off);

	snps_set_bits(csiphy, SNPS_RX_CB_2_00 + off, BIT(6), 0);

	if (lane_rate <= 1500) {
		writel_relaxed(0x20, csiphy->base + SNPS_RX_LANE0_DDL_2_00 + off);
		writel_relaxed(0x10, csiphy->base + SNPS_RX_LANE0_DDL_3_00 + off);
		writel_relaxed(0x80, csiphy->base + SNPS_RX_LANE_1_10_00 + off);
		writel_relaxed(0x10, csiphy->base + SNPS_RX_LANE_1_11_00 + off);
	}
}

static int snps_wait_startup(struct csiphy_device *csiphy, u32 off)
{
	u32 val;
	int i;

	for (i = 0; i < 6; i++) {
		val = readl_relaxed(csiphy->base + SNPS_RX_STARTUP_OBS_2_00 + off);
		if (val & BIT(4))
			return 0;
		usleep_range(100, 150);
	}

	dev_err(csiphy->camss->dev, "CSIPHY%d SNPS startup timeout\n",
		csiphy->id);

	return -ETIMEDOUT;
}

static void csiphy_lanes_enable(struct csiphy_device *csiphy,
				struct csiphy_config *cfg,
				s64 link_freq, u8 lane_mask)
{
	struct csiphy_lanes_cfg *c = &cfg->csi2->lane_cfg;
	enum snps_mode mode;
	u32 lane_rate, val;

	switch (lane_mask) {
	case SNPS_LANE_MASK_PHY_A:
		mode = SNPS_PHY_A;
		break;
	case SNPS_LANE_MASK_PHY_B:
		mode = SNPS_PHY_B;
		break;
	case SNPS_LANE_MASK_AGGR | BIT(5):
		mode = SNPS_AGGREGATE;
		break;
	default:
		dev_err(csiphy->camss->dev,
			"CSIPHY%d unsupported SNPS lane mask 0x%x\n",
			csiphy->id, lane_mask);
		return;
	}

	lane_rate = div_u64(link_freq * 2, 1000000);
	if (mode == SNPS_AGGREGATE && c->num_data)
		lane_rate = div_u64(link_freq * 2 * c->num_data, 4 * 1000000);

	if (mode != SNPS_PHY_B)
		snps_2_lane_config(csiphy, SNPS_PHY_A, lane_rate);
	if (mode != SNPS_PHY_A)
		snps_2_lane_config(csiphy, SNPS_PHY_B, lane_rate);

	snps_irq_config(csiphy, true);

	val = 0;
	if (mode != SNPS_PHY_B)
		val |= 0x3;
	if (mode != SNPS_PHY_A)
		val |= 0xc;
	writel_relaxed(val, csiphy->base + SNPS_FORCE_MODE);

	if (mode == SNPS_AGGREGATE) {
		writel_relaxed(0x1, csiphy->base + SNPS_RX_DUAL_PHY_0_00);
		writel_relaxed(0x0, csiphy->base + SNPS_RX_DUAL_PHY_0_00 +
			       SNPS_PHY_B_OFFSET);

		snps_set_bits(csiphy, SNPS_RX_LANE_0_7_00, BIT(5), 0);
		snps_set_bits(csiphy, SNPS_RX_LANE_0_7_00 + SNPS_PHY_B_OFFSET,
			      BIT(5), 0);
		snps_set_bits(csiphy, SNPS_RX_LANE_1_7_00, BIT(5), 0);
		snps_set_bits(csiphy, SNPS_RX_LANE_1_7_00 + SNPS_PHY_B_OFFSET,
			      BIT(5), 0);

		writel_relaxed(0x0, csiphy->base + SNPS_RX_CLK_LANE_7_00);
		writel_relaxed(BIT(3), csiphy->base + SNPS_RX_CLK_LANE_7_00 +
			       SNPS_PHY_B_OFFSET);

		snps_set_bits(csiphy, SNPS_RX_STARTUP_OVR_1_00 + SNPS_PHY_B_OFFSET,
			      BIT(1), BIT(0));
		snps_set_bits(csiphy, SNPS_RX_CLK_LANE_6_00, BIT(2), BIT(7));
		snps_set_bits(csiphy, SNPS_RX_CLK_LANE_6_00 + SNPS_PHY_B_OFFSET,
			      BIT(3), BIT(7) | BIT(2));
		snps_set_bits(csiphy, SNPS_RX_CLK_LANE_3_00 + SNPS_PHY_B_OFFSET,
			      BIT(7), 0);
		writel_relaxed(0xa, csiphy->base + SNPS_RX_CLK_LANE_4_00 +
			       SNPS_PHY_B_OFFSET);

		writel_relaxed(0x7e, csiphy->base + SNPS_FIFO_CTRL);
		writel_relaxed(0x7f, csiphy->base + SNPS_FIFO_CTRL);
	}

	val = 0;
	if (mode != SNPS_PHY_B)
		val |= 0x3;
	if (mode != SNPS_PHY_A)
		val |= 0xc;
	writel_relaxed(val, csiphy->base + SNPS_ENABLE);

	val = 0;
	if (mode != SNPS_PHY_B)
		val |= 0x1;
	if (mode != SNPS_PHY_A)
		val |= 0x2;
	writel_relaxed(val, csiphy->base + SNPS_BASEDIR);
	writel_relaxed(val, csiphy->base + SNPS_ENABLE_CLK);

	switch (mode) {
	case SNPS_PHY_A:
		writel_relaxed(0x1, csiphy->base + SNPS_CTRL_1);
		writel_relaxed(0x5, csiphy->base + SNPS_CTRL_1);
		snps_wait_startup(csiphy, 0);
		break;
	case SNPS_PHY_B:
		writel_relaxed(0x7, csiphy->base + SNPS_CTRL_1);
		writel_relaxed(0xf, csiphy->base + SNPS_CTRL_1);
		snps_wait_startup(csiphy, SNPS_PHY_B_OFFSET);
		break;
	case SNPS_AGGREGATE:
		writel_relaxed(0x3, csiphy->base + SNPS_CTRL_1);
		writel_relaxed(0x7, csiphy->base + SNPS_CTRL_1);
		snps_wait_startup(csiphy, 0);
		writel_relaxed(0xf, csiphy->base + SNPS_CTRL_1);
		snps_wait_startup(csiphy, SNPS_PHY_B_OFFSET);
		break;
	}

	writel_relaxed(0x0, csiphy->base + SNPS_FORCE_MODE);
}

static void csiphy_lanes_disable(struct csiphy_device *csiphy,
				 struct csiphy_config *cfg)
{
	snps_irq_config(csiphy, false);
	writel_relaxed(0x0, csiphy->base + SNPS_CTRL_1);
	writel_relaxed(0x0, csiphy->base + SNPS_ENABLE);
	writel_relaxed(0x0, csiphy->base + SNPS_ENABLE_CLK);
}

static irqreturn_t csiphy_isr(int irq, void *dev)
{
	struct csiphy_device *csiphy = dev;
	u32 val;
	int i;

	for (i = 0; i < 4; i++) {
		val = readl_relaxed(csiphy->base + SNPS_IRQ_STATUS_0 + 4 * i);
		writel_relaxed(val, csiphy->base + SNPS_IRQ_CLEAR_0 + 4 * i);
		if (val)
			dev_err_ratelimited(csiphy->camss->dev,
					    "CSIPHY%d IRQ status%d 0x%x\n",
					    csiphy->id, i, val);
	}

	for (i = 0; i < 2; i++) {
		val = readl_relaxed(csiphy->base + SNPS_IRQ_CLK_STATUS_0 + 4 * i);
		writel_relaxed(val, csiphy->base + SNPS_IRQ_CLK_CLEAR_0 + 4 * i);
		if (val)
			dev_err_ratelimited(csiphy->camss->dev,
					    "CSIPHY%d IRQ clk status%d 0x%x\n",
					    csiphy->id, i, val);
	}

	writel_relaxed(0x1, csiphy->base + SNPS_GLBL_IRQ_CMD);
	writel_relaxed(0x0, csiphy->base + SNPS_GLBL_IRQ_CMD);

	for (i = 0; i < 4; i++)
		writel_relaxed(0x0, csiphy->base + SNPS_IRQ_CLEAR_0 + 4 * i);
	for (i = 0; i < 2; i++)
		writel_relaxed(0x0, csiphy->base + SNPS_IRQ_CLK_CLEAR_0 + 4 * i);

	return IRQ_HANDLED;
}

static int csiphy_init(struct csiphy_device *csiphy)
{
	return 0;
}

const struct csiphy_hw_ops csiphy_ops_snps_10_0 = {
	.get_lane_mask = csiphy_get_lane_mask,
	.hw_version_read = csiphy_hw_version_read,
	.reset = csiphy_reset,
	.lanes_enable = csiphy_lanes_enable,
	.lanes_disable = csiphy_lanes_disable,
	.isr = csiphy_isr,
	.init = csiphy_init,
};
