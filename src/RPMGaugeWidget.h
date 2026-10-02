#pragma once
#include "SpeedometerWidget.h"

class RPMGaugeWidget : public SpeedometerWidget {
    Q_OBJECT
public:
    explicit RPMGaugeWidget(QWidget* parent = nullptr);
    void setRpm(int rpm);
};
