#include "SpeedometerWidget.h"
#include <QPainter>
#include <QRadialGradient>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace {
QPointF point(float value, float maximum, float radius) {
    const double angle = (225.0 - value / maximum * 270.0) * 3.141592653589793 / 180.0;
    return {radius * std::cos(angle), -radius * std::sin(angle)};
}
QFont gaugeFont(int pixels, bool bold = false) {
    QFont font("DejaVu Sans");
    font.setPixelSize(pixels);
    font.setBold(bold);
    return font;
}
}

SpeedometerWidget::SpeedometerWidget(QWidget* parent)
    : SpeedometerWidget(220, 20, "km/h", false, parent) {}

SpeedometerWidget::SpeedometerWidget(float maximum, float step, const QString& unit,
                                   bool rpmScale, QWidget* parent)
    : QWidget(parent), maximum_(maximum), major_step_(step), unit_(unit), rpm_scale_(rpmScale) {
    setMinimumSize(280, 280);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void SpeedometerWidget::setSpeed(float value) {
    if (!std::isfinite(value)) return;
    speed_ = std::max(0.0f, value);
    received_ = true;
    update();
}

void SpeedometerWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal side = std::min(width(), height());
    p.translate(width() / 2.0, height() / 2.0);
    p.scale(side / 500.0, side / 500.0);
    p.setPen(Qt::NoPen);
    QRadialGradient face(0, 55, 255);
    face.setColorAt(0, QColor("#0c1721"));
    face.setColorAt(1, QColor("#080f16"));
    p.setBrush(face);
    p.drawEllipse(QPointF(0, 0), 229, 229);

    p.setBrush(Qt::NoBrush);
    for (int width : {24, 14, 7}) {
        p.setPen(QPen(QColor(15, 124, 255, 25), width));
        p.drawEllipse(QPointF(0, 0), 230, 230);
    }
    p.setPen(QPen(QColor("#329fff"), 2.5));
    p.drawEllipse(QPointF(0, 0), 230, 230);
    p.setPen(QPen(QColor("#16395a"), 1));
    p.drawEllipse(QPointF(0, 0), 237, 237);

    const int divisions = static_cast<int>(maximum_ / major_step_) * 5;
    for (int i = 0; i <= divisions; ++i) {
        const float value = maximum_ * i / divisions;
        const bool major = i % 5 == 0;
        QColor color = rpm_scale_ && value >= 6000 ? QColor("#ee565b")
                      : major ? QColor("#e7effc") : QColor("#738497");
        p.setPen(QPen(color, major ? 2.8 : 1));
        p.drawLine(point(value, maximum_, major ? 197 : 205), point(value, maximum_, 217));
        if (major) {
            const QPointF position = point(value, maximum_, 174);
            p.setFont(gaugeFont(rpm_scale_ ? 28 : 23));
            p.setPen(QColor("#e9eff8"));
            const int label = qRound(rpm_scale_ ? value / 1000 : value);
            p.drawText(QRectF(position.x() - 32, position.y() - 20, 64, 40),
                       Qt::AlignCenter, QString::number(label));
        }
    }
    if (rpm_scale_) {
        p.setFont(gaugeFont(17));
        p.setPen(QColor("#a5b8d0"));
        p.drawText(QRectF(-100, -119, 200, 30), Qt::AlignCenter, "RPM x1000");
    }
    if (received_) {
        const QPointF tip = point(std::clamp(speed_, 0.0f, maximum_), maximum_, 204);
        for (int width : {12, 7}) {
            p.setPen(QPen(QColor(27, 137, 255, 55), width, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(0, 0), tip);
        }
        p.setPen(QPen(QColor("#58c5ff"), 2.6, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(0, 0), tip);
    }
    p.setBrush(QColor("#0c1927"));
    p.setPen(QPen(QColor("#1b4163"), 1));
    p.drawEllipse(QPointF(0, 0), 22, 22);
    p.setPen(QPen(QColor("#69baff"), 1.5));
    p.drawEllipse(QPointF(0, 0), 16, 16);
    p.setFont(gaugeFont(rpm_scale_ ? 48 : 58, true));
    p.setPen(QColor("#f2f6ff"));
    p.drawText(QRectF(-115, 91, 230, 73), Qt::AlignCenter,
               received_ ? QString::number(qRound(speed_)) : "--");
    p.setFont(gaugeFont(19));
    p.setPen(QColor("#a7b8cd"));
    p.drawText(QRectF(-100, 165, 200, 32), Qt::AlignCenter, unit_);
}
