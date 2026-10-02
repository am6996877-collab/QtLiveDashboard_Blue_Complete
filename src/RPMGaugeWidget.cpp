#include "RPMGaugeWidget.h"

RPMGaugeWidget::RPMGaugeWidget(QWidget* parent)
    : SpeedometerWidget(8000, 1000, "RPM", true, parent) {}

void RPMGaugeWidget::setRpm(int rpm) {
    setSpeed(static_cast<float>(rpm));
}
