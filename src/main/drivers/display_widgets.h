#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "drivers/osd.h"

typedef struct widgetRect_s {
    int x;
    int y;
    unsigned w;
    unsigned h;
} widgetRect_t;

typedef enum {
    DISPLAY_WIDGET_TYPE_AHI,
    DISPLAY_WIDGET_TYPE_SIDEBAR,
} displayWidgetType_e;

typedef enum {
    DISPLAY_WIDGET_AHI_STYLE_STAIRCASE = 0,
    DISPLAY_WIDGET_AHI_STYLE_LINE = 1,
} widgetAHIStyle_e;

typedef enum {
    DISPLAY_WIDGET_AHI_OPTION_SHOW_CORNERS = 1 << 0,
} widgetAHIOptions_t;

typedef struct widgetAHIConfiguration_s {
    widgetRect_t rect;
    widgetAHIStyle_e style;
    widgetAHIOptions_t options;
    unsigned crosshairMargin;
    unsigned strokeWidth;
} widgetAHIConfiguration_t;

typedef struct widgetAHIData_s {
    float pitch; // radians
    float roll; // radians
} widgetAHIData_t;

typedef enum
{
    DISPLAY_WIDGET_SIDEBAR_OPTION_LEFT = 1 << 0,      // Display the sidebar oriented to the left. Default is to the right
    DISPLAY_WIDGET_SIDEBAR_OPTION_REVERSE = 1 << 1,   // Reverse the sidebar direction, so positive values move it down
    DISPLAY_WIDGET_SIDEBAR_OPTION_UNLABELED = 1 << 2, // Don't display the central label with the value.
    DISPLAY_WIDGET_SIDEBAR_OPTION_STATIC = 1 << 3,    // The sidebar doesn't scroll nor display values along it.
} widgetSidebarOptions_t;

typedef struct widgetSidebarConfiguration_s {
    widgetRect_t rect;
    widgetSidebarOptions_t options;
    uint8_t divisions;                  // How many divisions the sidebar will have
    uint16_t counts_per_step;           // How much the value increases/decreases per division BEFORE applying the unit scale
    osdUnit_t unit;                     // The unit used to display the values in the sidebar
} widgetSidebarConfiguration_t;

typedef struct displayWidgetsVTable_s displayWidgetsVTable_t;

typedef struct displayWidgets_s {
    const displayWidgetsVTable_t *vTable;
    void *device;
} displayWidgets_t;

typedef struct displayWidgetsVTable_s {
    int (*supportedInstances)(displayWidgets_t *widgets, displayWidgetType_e widgetType);
    bool (*configureAHI)(displayWidgets_t *widgets, unsigned instance, const widgetAHIConfiguration_t *config);
    bool (*drawAHI)(displayWidgets_t *widgets, unsigned instance, const widgetAHIData_t *data);
    bool (*configureSidebar)(displayWidgets_t *widgets, unsigned instance, const widgetSidebarConfiguration_t *config);
    bool (*drawSidebar)(displayWidgets_t *widgets, unsigned instance, int32_t data);
} displayWidgetsVTable_t;

int displayWidgetsSupportedInstances(displayWidgets_t *widgets, displayWidgetType_e widgetType);
bool displayWidgetsConfigureAHI(displayWidgets_t *widgets, unsigned instance, const widgetAHIConfiguration_t *config);
bool displayWidgetsDrawAHI(displayWidgets_t *widgets, unsigned instance, const widgetAHIData_t *data);
bool displayWidgetsConfigureSidebar(displayWidgets_t *widgets, unsigned instance, const widgetSidebarConfiguration_t *config);
bool displayWidgetsDrawSidebar(displayWidgets_t *widgets, unsigned instance, int32_t data);
