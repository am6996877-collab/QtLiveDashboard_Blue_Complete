#pragma once
#include <QSvgWidget>
#include <QString>

// A check-engine ("CEL") icon rendered from inline SVG markup, recolored at
// runtime rather than swapped between two static image files. Default grey
// when no fault is active; lit (amber/red) when the ECU's dtc_manager_ has
// warningIndicatorRequested set on any DTC (content.md Section 3.4 Pitfall
// -- the *only* legitimate trigger for this icon is that bit, routed here
// exclusively through CANWorker::faultStatusUpdated, never a raw sensor
// threshold re-checked on the dashboard side).
class CheckEngineIcon : public QSvgWidget {
    Q_OBJECT
public:
    explicit CheckEngineIcon(QWidget* parent = nullptr);

public slots:
    void setActive(bool active);

private:
    void render(const QString& color);
    bool active_ = false;
};
