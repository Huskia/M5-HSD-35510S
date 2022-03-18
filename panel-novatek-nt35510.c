#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

struct nt35510_panel_desc {
   const struct drm_display_mode* mode;
   unsigned int lanes;
   unsigned long flags;
   enum mipi_dsi_pixel_format format;
   const char* const* supply_names;
   unsigned int num_supplies;
   unsigned int panel_sleep_delay;
};

struct nt35510 {
   struct drm_panel panel;
   struct mipi_dsi_device* dsi;
   const struct nt35510_panel_desc* desc;
   struct regulator_bulk_data* supplies;
   struct gpio_desc* reset;
   struct gpio_desc* backlight;
   unsigned int sleep_delay;
};

static inline struct nt35510* panel_to_nt35510(struct drm_panel* panel) {
   return container_of(panel, struct nt35510, panel);
}

// MIPI DCS
static inline int nt35510_dsi_write(struct nt35510* nt35510, uint8_t cmd, const void* seq, size_t len) {
   return mipi_dsi_dcs_write(nt35510->dsi, cmd, seq, len);
}

#define NT35510_DSI(nt35510, cmd, seq...)                \
   {                                                     \
      const u8 d[] = {seq};                              \
      nt35510_dsi_write(nt35510, cmd, d, ARRAY_SIZE(d)); \
   }

static void nt35510_init_sequence(struct nt35510* nt35510) { //NT35510 initilize 
   // NT35510_DSI(nt35510, MIPI_DCS_SOFT_RESET, 0x00);
   // msleep(nt35510->sleep_delay);
   //    NT35510_DSI(nt35510, 0x38, 0x00);
   //    msleep(nt35510->sleep_delay);
   NT35510_DSI(nt35510, 0xf0, 0x55, 0xAA, 0x52, 0x08, 0x01);
   NT35510_DSI(nt35510, 0xB0, 0x0D, 0x0D, 0x0D);  // 0A
   NT35510_DSI(nt35510, 0xB6, 0x44, 0x44, 0x44);  // 44
   NT35510_DSI(nt35510, 0xB1, 0x0D, 0x0D, 0x0D);  // 0A
   NT35510_DSI(nt35510, 0xB7, 0x34, 0x34, 0x34);  // 34
   NT35510_DSI(nt35510, 0xB2, 0x00, 0x00, 0x00);
   NT35510_DSI(nt35510, 0xB8, 0x34, 0x34, 0x34);  // 34
   NT35510_DSI(nt35510, 0xBF, 0x01);
   NT35510_DSI(nt35510, 0xB3, 0x0F, 0x0F, 0x0F);  // 08
   NT35510_DSI(nt35510, 0xB9, 0x34, 0x34, 0x34);
   NT35510_DSI(nt35510, 0xB5, 0x08, 0x08, 0x08);
   NT35510_DSI(nt35510, 0xC2, 0x03);
   NT35510_DSI(nt35510, 0xBA, 0x34, 0x34, 0x34);
   NT35510_DSI(nt35510, 0xBC, 0x00, 0x78, 0x00);
   NT35510_DSI(nt35510, 0xBD, 0x00, 0x78, 0x00);

   NT35510_DSI(nt35510, 0xBE, 0x00, 0x54);  // 54  vcom---------
   NT35510_DSI(nt35510, 0xd1, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(nt35510, 0xd2, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(nt35510, 0xd3, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(nt35510, 0xd4, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(nt35510, 0xd5, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   NT35510_DSI(nt35510, 0xd6, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x34, 0x00, 0x55, 0x00, 0x75, 0x00, 0xA9, 0x00, 0xCF, 0x01, 0x0B, 0x01, 0x33, 0x01, 0x71, 0x01, 0x9E, 0x01, 0xDE, 0x02, 0x0E, 0x02, 0x10, 0x02, 0x3C, 0x02, 0x67, 0x02, 0x7F, 0x02, 0x9C, 0x02, 0xAE, 0x02, 0xC5, 0x02, 0xD3, 0x02, 0xE6, 0x02, 0xF4, 0x03, 0x08, 0x03, 0x39, 0x03, 0xFA);
   // page 0
   NT35510_DSI(nt35510, 0xf0, 0x55, 0xAA, 0x52, 0x08, 0x00);
   NT35510_DSI(nt35510, 0xB5, 0x6B);
   NT35510_DSI(nt35510, 0xB1, 0xF8, 0x00);
   NT35510_DSI(nt35510, 0xB6, 0x01);
   NT35510_DSI(nt35510, 0xB7, 0x70, 0x70);
   NT35510_DSI(nt35510, 0xB8, 0x01, 0x02, 0x02, 0x02);
   NT35510_DSI(nt35510, 0xBC, 0x00, 0x00, 0x00);
   NT35510_DSI(nt35510, 0xBD, 0x01, 0x84, 0x07, 0x32, 0x00);
   NT35510_DSI(nt35510, 0xC9, 0xD0, 0x02, 0x50, 0x50, 0x50);
   NT35510_DSI(nt35510, 0x34, 0x00);
   NT35510_DSI(nt35510, 0x11, 0x00);
   msleep(nt35510->sleep_delay);
   // NT35510_DSI(nt35510, 0x29,0x00);  // Display on
}

static int nt35510_read_id(struct nt35510* nt35510) {
   uint8_t id1, id2, id3;
   int ret;
   ret = mipi_dsi_dcs_read(nt35510->dsi, 0xDA, &id1, 1);
   if (ret < 0) {
      dev_err(&nt35510->dsi->dev, "Could not read MTP ID1\n");
      return ret;
   }
   ret = mipi_dsi_dcs_read(nt35510->dsi, 0xDB, &id2, 1);
   if (ret < 0) {
      dev_err(&nt35510->dsi->dev, "Could not read MTP ID2\n");
      return ret;
   }
   ret = mipi_dsi_dcs_read(nt35510->dsi, 0xDC, &id3, 1);
   if (ret < 0) {
      dev_err(&nt35510->dsi->dev, "Could not read MTP ID3\n");
      return ret;
   }
   dev_info(&nt35510->dsi->dev, "MTP manufacture id: %02x %02x %02x\n", id1, id2, id3);

   return 0;
}

// The following function is needed by driver, boot  sequence probe->prepare->enable, turn off screen: disable->unprepare
static int nt35510_prepare(struct drm_panel* panel) {
   struct nt35510* nt35510 = panel_to_nt35510(panel);
   int ret;

   gpiod_set_value(nt35510->reset, 1); //Set 1 to pull down reset pin (only gpio45 logic level is reversed)

   ret = regulator_bulk_enable(nt35510->desc->num_supplies, nt35510->supplies);
   if (ret < 0)
      return ret;
   msleep(20);

   gpiod_set_value(nt35510->reset, 0);
   msleep(150);

   nt35510_init_sequence(nt35510);
   dev_info(&nt35510->dsi->dev, "nt35510 initilize sequence send finish\n");
   nt35510_read_id(nt35510);

   return 0;
}

static int nt35510_enable(struct drm_panel* panel) {
   struct nt35510* nt35510 = panel_to_nt35510(panel);

   NT35510_DSI(nt35510, 0x29, 0x00); //Turn on display
   //NT35510_DSI(nt35510, 0x2C, 0x00);
   gpiod_set_value(nt35510->backlight, 1); //Turn on backlight
   dev_info(&nt35510->dsi->dev, "nt35510 enabled\n");
   return 0;
}

static int nt35510_disable(struct drm_panel* panel) {
   struct nt35510* nt35510 = panel_to_nt35510(panel);

   NT35510_DSI(nt35510, 0x28, 0x00); //Turn off display
   dev_info(&nt35510->dsi->dev, "nt35510 disabled\n");
   return 0;
}

static int nt35510_unprepare(struct drm_panel* panel) {
   struct nt35510* nt35510 = panel_to_nt35510(panel);

   NT35510_DSI(nt35510, 0x10, 0x00); //Screen sleep mode

   msleep(nt35510->sleep_delay);

   gpiod_set_value(nt35510->reset, 1); //Pull down reset pin
   gpiod_set_value(nt35510->backlight, 0); //Turn off display

   msleep(nt35510->sleep_delay);

   regulator_bulk_disable(nt35510->desc->num_supplies, nt35510->supplies);
   dev_info(&nt35510->dsi->dev, "nt35510 unprepared\n");
   return 0;
}

static int nt35510_get_modes(struct drm_panel* panel,
                             struct drm_connector* connector) {
   struct nt35510* nt35510 = panel_to_nt35510(panel);
   const struct drm_display_mode* desc_mode = nt35510->desc->mode;
   struct drm_display_mode* mode;

   mode = drm_mode_duplicate(connector->dev, desc_mode);
   if (!mode) {
      dev_err(&nt35510->dsi->dev, "failed to add mode %ux%u@%u\n",
              desc_mode->hdisplay, desc_mode->vdisplay,
              drm_mode_vrefresh(desc_mode));
      return -ENOMEM;
   }

   drm_mode_set_name(mode);
   drm_mode_probed_add(connector, mode);

   connector->display_info.width_mm = desc_mode->width_mm;
   connector->display_info.height_mm = desc_mode->height_mm;

   return 1;
}

static const struct drm_panel_funcs nt35510_funcs = {
    .disable = nt35510_disable,
    .unprepare = nt35510_unprepare,
    .prepare = nt35510_prepare,
    .enable = nt35510_enable,
    .get_modes = nt35510_get_modes,
};

static const struct drm_display_mode m5hsd35510s_mode = {
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

    .type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static const char* const m5hsd35510s_supply_names[] = {
    "vdd",
    "vddi",
};

static const struct nt35510_panel_desc m5hsd35510s_desc = {
    .mode = &m5hsd35510s_mode,
    .lanes = 2,
    .flags = MIPI_DSI_MODE_VIDEO_HSE | MIPI_DSI_MODE_VIDEO | MIPI_DSI_CLOCK_NON_CONTINUOUS, // MIPI mode set
    .format = MIPI_DSI_FMT_RGB888, // Display format
    .supply_names = m5hsd35510s_supply_names,
    .num_supplies = ARRAY_SIZE(m5hsd35510s_supply_names),
    .panel_sleep_delay = 80,
};

static int nt35510_dsi_probe(struct mipi_dsi_device* dsi) {
   const struct nt35510_panel_desc* desc;
   struct nt35510* nt35510;
   int ret, i;

   nt35510 = devm_kzalloc(&dsi->dev, sizeof(*nt35510), GFP_KERNEL);
   if (!nt35510)
      return -ENOMEM;

   desc = of_device_get_match_data(&dsi->dev);
   dsi->mode_flags = desc->flags;
   dsi->format = desc->format;
   dsi->lanes = desc->lanes;
   dsi->hs_rate = 349440000;
   dsi->lp_rate = 9600000;

   nt35510->supplies = devm_kcalloc(&dsi->dev, desc->num_supplies,
                                    sizeof(*nt35510->supplies), GFP_KERNEL);
   if (!nt35510->supplies)
      return -ENOMEM;

   for (i = 0; i < desc->num_supplies; i++)
      nt35510->supplies[i].supply = desc->supply_names[i];

   ret = devm_regulator_bulk_get(&dsi->dev, desc->num_supplies,
                                 nt35510->supplies);
   if (ret < 0)
      return ret;

   nt35510->reset = devm_gpiod_get(&dsi->dev, "reset", GPIOD_OUT_LOW);
   if (IS_ERR(nt35510->reset)) {
      dev_err(&dsi->dev, "Couldn't get our reset GPIO\n");
      return PTR_ERR(nt35510->reset);
   }

   nt35510->backlight = devm_gpiod_get(&dsi->dev, "backlight", GPIOD_OUT_LOW);
   if (IS_ERR(nt35510->backlight)) {
      dev_err(&dsi->dev, "Couldn't get our backlight GPIO\n");
      return PTR_ERR(nt35510->backlight);
   }

   drm_panel_init(&nt35510->panel, &dsi->dev, &nt35510_funcs, DRM_MODE_CONNECTOR_DSI);

   nt35510->sleep_delay = 120 + desc->panel_sleep_delay;

   ret = drm_panel_of_backlight(&nt35510->panel);
   if (ret)
      return ret;

   drm_panel_add(&nt35510->panel);

   mipi_dsi_set_drvdata(dsi, nt35510);

   nt35510->dsi = dsi;
   nt35510->desc = desc;

   return mipi_dsi_attach(dsi);
}

static int nt35510_dsi_remove(struct mipi_dsi_device* dsi) {
   struct nt35510* nt35510 = mipi_dsi_get_drvdata(dsi);

   mipi_dsi_detach(dsi);
   drm_panel_remove(&nt35510->panel);

   return 0;
}

static const struct of_device_id nt35510_of_match[] = {
    {.compatible = "m5,m5hsd35510s", .data = &m5hsd35510s_desc}, //Same as dts file
    {}};
MODULE_DEVICE_TABLE(of, nt35510_of_match);

static struct mipi_dsi_driver nt35510_dsi_driver = {
    .probe = nt35510_dsi_probe,
    .remove = nt35510_dsi_remove,
    .driver = {
        .name = "nt35510",
        .of_match_table = nt35510_of_match,
    },
};
module_mipi_dsi_driver(nt35510_dsi_driver);

MODULE_AUTHOR("Nicholas Huskie <huskie@idealfuture.org.cn>");
MODULE_DESCRIPTION("m5hsd35510s nt35510 LCD Panel Driver");
MODULE_LICENSE("GPL");
