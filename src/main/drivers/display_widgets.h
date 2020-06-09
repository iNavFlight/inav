#pragma once

#include <stdbool.h>

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

typedef struct displayWidgetsVTable_s displayWidgetsVTable_t;

typedef struct displayWidgets_s {
    const displayWidgetsVTable_t *vTable;
    void *device;
} displayWidgets_t;

typedef struct displayWidgetsVTable_s {
    int (*supportedInstances)(displayWidgets_t *widgets, displayWidgetType_e widgetType);
    bool (*configureAHI)(displayWidgets_t *widgets, unsigned instance, const widgetAHIConfiguration_t *config);
    bool (*drawAHI)(displayWidgets_t *widgets, unsigned instance, const widgetAHIData_t *data);
} displayWidgetsVTable_t;

int displayWidgetsSupportedInstances(displayWidgets_t *widgets, displayWidgetType_e widgetType);
bool displayWidgetsConfigureAHI(displayWidgets_t *widgets, unsigned instance, const widgetAHIConfiguration_t *config);
bool displayWidgetsDrawAHI(displayWidgets_t *widgets, unsigned instance, const widgetAHIData_t *data);
