#pragma once
#include <QWidget>
#include <QString>

class SpeedometerWidget : public QWidget {
    Q_OBJECT
public:
    explicit SpeedometerWidget(QWidget* parent = nullptr);
    void setSpeed(float speed_kmh);
    float speed() const { return speed_; }

protected:
    SpeedometerWidget(float maximum, float step, const QString& unit,
                      bool rpmScale, QWidget* parent);
    void paintEvent(QPaintEvent*) override;

private:
    float speed_ = 0;
    float maximum_;
    float major_step_;
    QString unit_;
    bool rpm_scale_;
    bool received_ = false;
};
