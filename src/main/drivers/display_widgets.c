
#include "platform.h"

#if defined(USE_CANVAS)

#include "drivers/display_widgets.h"

int displayWidgetsSupportedInstances(displayWidgets_t *widgets, displayWidgetType_e widgetType)
{
    return widgets->vTable->supportedInstances ? widgets->vTable->supportedInstances(widgets, widgetType) : 0;
}

bool displayWidgetsConfigureAHI(displayWidgets_t *widgets, unsigned instance, const widgetAHIConfiguration_t *config)
{
    return widgets->vTable->configureAHI ? widgets->vTable->configureAHI(widgets, instance, config) : false;
}

bool displayWidgetsDrawAHI(displayWidgets_t *widgets, unsigned instance, const widgetAHIData_t *data)
{
    return widgets->vTable->drawAHI ? widgets->vTable->drawAHI(widgets, instance, data) : false;
}

#endif
