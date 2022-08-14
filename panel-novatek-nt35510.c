#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>

#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

#include <video/mipi_display.h>

struct nt35510_desc {
	const size_t init_length;
	const struct drm_display_mode *mode;
	const unsigned flags;
};

struct nt35510 {
	struct drm_panel	panel;
	struct mipi_dsi_device	*dsi;
	const struct nt35510_desc	*desc;

	struct regulator	*power;
	struct gpio_desc	*reset;
};

static int nt35510_dsi_write(struct nt35510 *ctx, const void* seq, size_t len)
{
	int ret;
	ret = mipi_dsi_dcs_write_buffer(ctx->dsi, seq, len);
	if (ret < 0)
		return ret;
	return 0;
}

#define NT35510_DSI(ctx, seq...)                \
   {                                                     \
      const u8 d[] = {seq};                              \
      nt35510_dsi_write(ctx, d, ARRAY_SIZE(d)); \
   }

static void nt35510_init_sequence(struct nt35510* ctx) { //NT35510 initilize 
   // NT35510_DSI(ctx, MIPI_DCS_SOFT_RESET, 0x00);
   // msleep(ctx->sleep_delay);
   //    NT35510_DSI(ctx, 0x38, 0x00);
   //    msleep(ctx->sleep_delay);
   NT35510_DSI(ctx, 0xf0, 0x55, 0xAA, 0x52, 0x08, 0x01);
   NT35510_DSI(ctx, 0xB0, 0x0D, 0x0D, 0x0D);  // 0A
   NT35510_DSI(ctx, 0xB6, 0x44, 0x44, 0x44);  // 44
   NT35510_DSI(ctx, 0xB1, 0x0D, 0x0D, 0x0D);  // 0A
   NT35510_DSI(ctx, 0xB7, 0x34, 0x34, 0x34);  // 34
   NT35510_DSI(ctx, 0xB2, 0x00, 0x00, 0x00);
   NT35510_DSI(ctx, 0xB8, 0x34, 0x34, 0x34);  // 34
   NT35510_DSI(ctx, 0xBF, 0x01);
   NT35510_DSI(ctx, 0xB3, 0x0F, 0x0F, 0x0F);  // 08
   NT35510_DSI(ctx, 0xB9, 0x34, 0x34, 0x34);
   NT35510_DSI(ctx, 0xB5, 0x08, 0x08, 0x08);
   NT35510_DSI(ctx, 0xC2, 0x03);
   NT35510_DSI(ctx, 0xBA, 0x34, 0x34, 0x34);
   NT35510_DSI(ctx, 0xBC, 0x00, 0x78, 0x00);
   NT35510_DSI(ctx, 0xBD, 0x00, 0x78, 0x00);

   NT35510_DSI(ctx, 0xBE, 0x00, 0x54);  // 54  vcom---------
   NT35510_DSI(ctx, 0xd1, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(ctx, 0xd2, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(ctx, 0xd3, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(ctx, 0xd4, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(ctx, 0xd5, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(ctx, 0xd6, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   // page 0
   NT35510_DSI(ctx, 0xf0, 0x55, 0xAA, 0x52, 0x08, 0x00);
   NT35510_DSI(ctx, 0xB5, 0x6B);
   NT35510_DSI(ctx, 0xB1, 0xF8, 0x00);
   NT35510_DSI(ctx, 0xB6, 0x01);
   NT35510_DSI(ctx, 0xB7, 0x70, 0x70);
   NT35510_DSI(ctx, 0xB8, 0x01, 0x02, 0x02, 0x02);
   NT35510_DSI(ctx, 0xBC, 0x00, 0x00, 0x00);
   NT35510_DSI(ctx, 0xBD, 0x01, 0x84, 0x07, 0x32, 0x00);
   NT35510_DSI(ctx, 0xC9, 0xD0, 0x02, 0x50, 0x50, 0x50);
   NT35510_DSI(ctx, 0x34, 0x00);
   NT35510_DSI(ctx, 0x11, 0x00);
   msleep(120);
   NT35510_DSI(ctx, 0x29,0x00);  // Display on
}

static int nt35510_read_id(struct nt35510* ctx) {
   uint8_t id1, id2, id3;
   int ret;
   ret = mipi_dsi_dcs_read(ctx->dsi, 0xDA, &id1, 1);
   if (ret < 0) {
      dev_err(&ctx->dsi->dev, "Could not read MTP ID1\n");
      return ret;
   }
   ret = mipi_dsi_dcs_read(ctx->dsi, 0xDB, &id2, 1);
   if (ret < 0) {
      dev_err(&ctx->dsi->dev, "Could not read MTP ID2\n");
      return ret;
   }
   ret = mipi_dsi_dcs_read(ctx->dsi, 0xDC, &id3, 1);
   if (ret < 0) {
      dev_err(&ctx->dsi->dev, "Could not read MTP ID3\n");
      return ret;
   }
   dev_info(&ctx->dsi->dev, "MTP manufacture id: %02x %02x %02x\n", id1, id2, id3);

   return 0;
}

static inline struct nt35510 *panel_to_nt35510(struct drm_panel *panel)
{
	return container_of(panel, struct nt35510, panel);
}

static int nt35510_prepare(struct drm_panel *panel)
{
	struct nt35510 *ctx = panel_to_nt35510(panel);
	int ret;

	/* Power the panel */
	ret = regulator_enable(ctx->power);
	if (ret)
		return ret;
	msleep(5);

	/* And reset it */
	gpiod_set_value(ctx->reset, 1);
	msleep(20);

	gpiod_set_value(ctx->reset, 0);
	msleep(20);

	nt35510_init_sequence(ctx);
   dev_info(&ctx->dsi->dev, "nt35510 initilize sequence finish\n");
   nt35510_read_id(ctx);

	ret = mipi_dsi_dcs_set_tear_on(ctx->dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	if (ret)
		return ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(ctx->dsi);
	if (ret)
		return ret;

	return 0;
}

static int nt35510_enable(struct drm_panel *panel)
{
	struct nt35510 *ctx = panel_to_nt35510(panel);

	msleep(120);

	mipi_dsi_dcs_set_display_on(ctx->dsi);

	return 0;
}

static int nt35510_disable(struct drm_panel *panel)
{
	struct nt35510 *ctx = panel_to_nt35510(panel);

	return mipi_dsi_dcs_set_display_off(ctx->dsi);
}

static int nt35510_unprepare(struct drm_panel *panel)
{
	struct nt35510 *ctx = panel_to_nt35510(panel);

	mipi_dsi_dcs_enter_sleep_mode(ctx->dsi);
	regulator_disable(ctx->power);
	gpiod_set_value(ctx->reset, 1);

	return 0;
}

static const struct drm_display_mode m5hsd35510s_default_mode = {
	.clock = 26000,

   .hdisplay = 480,
   .hsync_start = 480 + 30,
   .hsync_end = 480 + 30 + 30,
   .htotal = 480 + 30 + 30 + 10,

   .vdisplay = 854,
   .vsync_start = 854 + 20,
   .vsync_end = 854 + 20 + 20,
   .vtotal = 854 + 20 + 20 + 4,

   .width_mm = 65,
   .height_mm = 117,
};

static int nt35510_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	struct nt35510 *ctx = panel_to_nt35510(panel);
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, ctx->desc->mode);
	if (!mode) {
		dev_err(&ctx->dsi->dev, "failed to add mode %ux%ux@%u\n",
			ctx->desc->mode->hdisplay,
			ctx->desc->mode->vdisplay,
			drm_mode_vrefresh(ctx->desc->mode));
		return -ENOMEM;
	}

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);

	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;

	return 1;
}

static const struct drm_panel_funcs nt35510_funcs = {
	.prepare	= nt35510_prepare,
	.unprepare	= nt35510_unprepare,
	.enable		= nt35510_enable,
	.disable	= nt35510_disable,
	.get_modes	= nt35510_get_modes,
};

static int nt35510_dsi_probe(struct mipi_dsi_device *dsi)
{
	struct nt35510 *ctx;
	int ret;

	ctx = devm_kzalloc(&dsi->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;
	mipi_dsi_set_drvdata(dsi, ctx);
	ctx->dsi = dsi;
	ctx->desc = of_device_get_match_data(&dsi->dev);

	ctx->panel.prepare_upstream_first = true;
	drm_panel_init(&ctx->panel, &dsi->dev, &nt35510_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ctx->power = devm_regulator_get(&dsi->dev, "power");
	if (IS_ERR(ctx->power)) {
		dev_err(&dsi->dev, "Couldn't get our power regulator\n");
		return PTR_ERR(ctx->power);
	}

	ctx->reset = devm_gpiod_get(&dsi->dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->reset)) {
		dev_err(&dsi->dev, "Couldn't get our reset GPIO\n");
		return PTR_ERR(ctx->reset);
	}

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return ret;

	drm_panel_add(&ctx->panel);

	dsi->mode_flags = ctx->desc->flags;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->lanes = 2;

	ret = mipi_dsi_attach(dsi);
	if (ret)
		drm_panel_remove(&ctx->panel);

	return ret;
}

static int nt35510_dsi_remove(struct mipi_dsi_device *dsi)
{
	struct nt35510 *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct nt35510_desc m5hsd35510s_desc = {
	.mode = &m5hsd35510s_default_mode,
	.flags = MIPI_DSI_MODE_VIDEO_HSE | MIPI_DSI_MODE_VIDEO | MIPI_DSI_CLOCK_NON_CONTINUOUS,
};

static const struct of_device_id nt35510_of_match[] = {
	{ .compatible = "m5,m5hsd35510s", .data = &m5hsd35510s_desc },
	{}
};
MODULE_DEVICE_TABLE(of, nt35510_of_match);

static struct mipi_dsi_driver nt35510_dsi_driver = {
	.probe		= nt35510_dsi_probe,
	.remove		= nt35510_dsi_remove,
	.driver = {
		.name		= "nt35510-dsi",
		.of_match_table	= nt35510_of_match,
	},
};
module_mipi_dsi_driver(nt35510_dsi_driver);

MODULE_AUTHOR("Nicholas Huskie <huskie@idealfuture.org.cn>");
MODULE_DESCRIPTION("m5hsd35510s nt35510 LCD Panel Driver");
MODULE_LICENSE("GPL");