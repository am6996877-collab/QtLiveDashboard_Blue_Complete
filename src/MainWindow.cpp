#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QStringList>
#include <cmath>

namespace {
class ClusterBackground : public QWidget {
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.fillRect(rect(), QColor("#080f16"));
        QRadialGradient glow(width() * 0.5, height() * 0.40, width() * 0.45);
        glow.setColorAt(0, QColor(12, 80, 132, 85));
        glow.setColorAt(1, QColor(8, 15, 22, 0));
        p.fillRect(rect(), glow);
        QLinearGradient line(0, 0, width(), 0);
        line.setColorAt(0, QColor(35, 153, 255, 0));
        line.setColorAt(0.5, QColor(35, 153, 255, 150));
        line.setColorAt(1, QColor(35, 153, 255, 0));
        p.fillRect(QRectF(0, height() * 0.39, width(), 1), line);
    }
};

QFrame* separator() {
    auto* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet("background:#22425a; border:0;");
    return line;
}

QString reading(const QString& title, const QString& value, const QString& color = "#f0f5fd") {
    return QString("<span style='color:#a3b4c9;font-size:16px'>%1</span><br/>"
                   "<span style='color:%3;font-size:28px;font-weight:600'>%2</span>")
                   .arg(title, value, color);
}

void door(QLabel* label, const QString& name, bool open) {
    label->setText(QString("<span style='color:#9aaec5;font-size:26px'>&#9633;</span><br/>"
                          "%1<br/><span style='color:%2'>%3</span>")
                          .arg(name, open ? "#ffb33e" : "#b8c6d9", open ? "OPEN" : "closed"));
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Blue Horizon | Live CAN Dashboard");
    resize(1440, 860);
    setMinimumSize(1100, 740);
    setStyleSheet(R"(
        QMainWindow { background:#080f16; }
        QWidget { color:#eaf0fb; font-family:'DejaVu Sans'; background:transparent; }
        QLabel { font-size:16px; border:0; }
        QProgressBar {
            border:1px solid #405569; border-radius:6px;
            background:#101c28; height:12px; color:#cbd8e9;
        }
        QProgressBar::chunk {
            border-radius:5px;
            background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #28a2ee,stop:1 #62ceff);
        }
    )");
    auto* central = new ClusterBackground;
    setCentralWidget(central);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(22, 22, 22, 20);
    root->setSpacing(18);
    auto* instruments = new QHBoxLayout;
    instruments->setSpacing(0);
    speedometer_ = new SpeedometerWidget(central);
    speedometer_->setObjectName("speedometer");
    rpm_gauge_ = new RPMGaugeWidget(central);
    rpm_gauge_->setObjectName("rpmGauge");

    auto* middle = new QWidget(central);
    middle->setMinimumWidth(235);
    middle->setMaximumWidth(310);
    auto* center = new QVBoxLayout(middle);
    center->setContentsMargins(8, 20, 8, 12);
    center->setSpacing(12);
    turn_signal_label_ = new QLabel("<span style='color:#43576e'>&#9664; &nbsp; &#9654;</span>", middle);
    turn_signal_label_->setAlignment(Qt::AlignCenter);
    turn_signal_label_->setMinimumHeight(48);
    turn_signal_label_->setStyleSheet("font-size:30px;");
    gear_label_ = new QLabel("<span style='font-size:82px'>--</span><br/>GEAR", middle);
    gear_label_->setAlignment(Qt::AlignCenter);
    gear_label_->setMinimumHeight(130);
    temp_label_ = new QLabel(reading("Coolant", "-- &deg;C"), middle);
    battery_label_ = new QLabel(reading("Battery", "-- V"), middle);
    for (auto* label : {temp_label_, battery_label_}) {
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumHeight(78);
    }
    cel_icon_ = new CheckEngineIcon(middle);
    fault_label_ = new QLabel("Fault status: not received", middle);
    fault_label_->setWordWrap(true);
    fault_label_->setAlignment(Qt::AlignCenter);
    fault_label_->setMinimumHeight(66);
    fault_label_->setStyleSheet("font-size:12px; color:#8396af;");
    center->addWidget(turn_signal_label_);
    center->addWidget(separator());
    center->addWidget(gear_label_);
    center->addWidget(separator());
    center->addWidget(temp_label_);
    center->addWidget(separator());
    center->addWidget(battery_label_);
    center->addWidget(separator());
    center->addWidget(cel_icon_, 0, Qt::AlignHCenter);
    center->addWidget(fault_label_);
    center->addStretch();
    instruments->addWidget(speedometer_, 5);
    instruments->addWidget(middle, 3);
    instruments->addWidget(rpm_gauge_, 5);
    root->addLayout(instruments, 1);
    root->addWidget(separator());

    auto* strip = new QHBoxLayout;
    strip->setSpacing(22);
    auto makeBar = [central, strip](const QString& title, int maximum, QProgressBar*& bar) {
        auto* box = new QWidget(central);
        auto* layout = new QVBoxLayout(box);
        layout->setContentsMargins(0, 5, 0, 5);
        auto* caption = new QLabel(title + "  --", box);
        bar = new QProgressBar(box);
        bar->setRange(0, maximum);
        bar->setValue(0);
        bar->setTextVisible(false);
        bar->setFixedHeight(13);
        bar->setAccessibleName(title);
        auto* endpoints = new QLabel(QString("0<span style='color:#8396af'> &nbsp; / &nbsp; %1</span>").arg(maximum), box);
        endpoints->setStyleSheet("font-size:11px; color:#8396af;");
        layout->addWidget(caption);
        layout->addWidget(bar);
        layout->addWidget(endpoints);
        strip->addWidget(box, 3);
        return caption;
    };
    QLabel* rpmCaption = makeBar("RPM", 8000, rpm_bar_);
    QLabel* throttleCaption = makeBar("Throttle", 100, throttle_bar_);
    engine_status_label_ = new QLabel(reading("Engine", "--", "#47b8ff"), central);
    ignition_label_ = new QLabel(reading("Ignition", "--", "#47b8ff"), central);
    // Smaller status text keeps the full strip readable at the minimum width.
    for (auto* label : {engine_status_label_, ignition_label_}) {
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumWidth(105);
        strip->addWidget(label, 1);
    }
    door_fl_label_ = new QLabel("FL<br/>--", central);
    door_fr_label_ = new QLabel("FR<br/>--", central);
    door_rl_label_ = new QLabel("RL<br/>--", central);
    door_rr_label_ = new QLabel("RR<br/>--", central);
    for (auto* label : {door_fl_label_, door_fr_label_, door_rl_label_, door_rr_label_}) {
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumWidth(55);
        label->setStyleSheet("font-size:13px;");
        strip->addWidget(label, 1);
    }
    root->addLayout(strip);
    // Preserve this original widget, but do not invent a fuel reading: no fuel signal exists.
    fuel_bar_ = new QProgressBar(central);
    fuel_bar_->setRange(0, 100);
    fuel_bar_->setValue(0);
    fuel_bar_->setFormat("Fuel: unavailable - not transmitted by this project");
    fuel_bar_->setFixedHeight(23);
    fuel_bar_->setStyleSheet("QProgressBar { border:0; color:#73869e; font-size:11px; text-align:center; }");
    root->addWidget(fuel_bar_);

    can_thread_ = new QThread(this);
    can_worker_ = new CANWorker;
    can_worker_->moveToThread(can_thread_);
    connect(can_thread_, &QThread::started, can_worker_, &CANWorker::run);
    connect(can_thread_, &QThread::finished, can_worker_, &QObject::deleteLater);
    connect(can_worker_, &CANWorker::rpmUpdated, rpm_bar_, &QProgressBar::setValue);
    connect(can_worker_, &CANWorker::rpmUpdated, rpm_gauge_, &RPMGaugeWidget::setRpm);
    connect(can_worker_, &CANWorker::rpmUpdated, this, [rpmCaption](int rpm) {
        rpmCaption->setText(QString("RPM  <b>%1</b>").arg(rpm));
    });
    connect(can_worker_, &CANWorker::speedUpdated, speedometer_, &SpeedometerWidget::setSpeed);
    connect(can_worker_, &CANWorker::tempUpdated, this, [this](float t) {
        temp_label_->setText(reading("Coolant", QString("%1 &deg;C").arg(t, 0, 'f', 1)));
    });
    connect(can_worker_, &CANWorker::gearUpdated, this, [this](int gear) {
        const char* letters[] = {"P", "R", "N", "D"};
        const char* names[] = {"PARK", "REVERSE", "NEUTRAL", "DRIVE"};
        const bool known = gear >= 0 && gear < 4;
        gear_label_->setText(QString("<span style='font-size:82px;font-weight:600'>%1</span><br/>"
                                    "<span style='color:#9fb3cd;letter-spacing:3px'>%2</span>")
                                    .arg(known ? letters[gear] : "?", known ? names[gear] : "UNKNOWN"));
    });
    connect(can_worker_, &CANWorker::doorStatusUpdated, this, [this](bool fl, bool fr, bool rl, bool rr) {
        door(door_fl_label_, "FL", fl); door(door_fr_label_, "FR", fr);
        door(door_rl_label_, "RL", rl); door(door_rr_label_, "RR", rr);
    });
    connect(can_worker_, &CANWorker::throttleUpdated, this, [this, throttleCaption](float pct) {
        throttle_bar_->setValue(static_cast<int>(pct));
        throttleCaption->setText(QString("Throttle  <b>%1%</b>").arg(pct, 0, 'f', 1));
    });
    connect(can_worker_, &CANWorker::engineRunningChanged, this, [this](bool on) {
        engine_status_label_->setText(QString("<span style='color:#a3b4c9'>Engine</span><br/>"
                                              "<span style='color:#47b8ff;font-size:16px;font-weight:600'>%1</span>")
                                              .arg(on ? "RUNNING" : "OFF"));
    });
    connect(can_worker_, &CANWorker::ignitionChanged, this, [this](bool on) {
        ignition_label_->setText(QString("<span style='color:#a3b4c9'>Ignition</span><br/>"
                                         "<span style='color:#47b8ff;font-size:16px;font-weight:600'>%1</span>")
                                         .arg(on ? "ON" : "OFF"));
    });
    connect(can_worker_, &CANWorker::batteryVoltageUpdated, this, [this](float volts) {
        battery_label_->setText(reading("Battery", QString("%1 V").arg(volts, 0, 'f', 1)));
    });
    connect(can_worker_, &CANWorker::turnSignalsChanged, this, [this](bool left, bool right) {
        turn_left_ = left; turn_right_ = right; updateTurnSignalLabel();
    });
    connect(can_worker_, &CANWorker::hazardChanged, this, [this](bool on) {
        hazard_ = on; updateTurnSignalLabel();
    });
    // This project's worker exposes a single check-engine flag, not Session 14 DTCs.
    connect(can_worker_, &CANWorker::faultStatusUpdated, this, [this](bool cel) {
        cel_icon_->setActive(cel);
        fault_label_->setText(cel ? "CHECK ENGINE" : "No active faults");
        fault_label_->setStyleSheet(cel ? "font-size:12px; color:#ffb33e;"
                                       : "font-size:12px; color:#8396af;");
    });
    can_thread_->start();
}

MainWindow::~MainWindow() {
    can_worker_->stop();
    can_thread_->quit();
    can_thread_->wait();
}

void MainWindow::updateTurnSignalLabel() {
    const char* lit = hazard_ ? "#ffb33e" : "#48baff";
    const QString text = hazard_ ? "HAZARD" : turn_left_ && turn_right_ ? "LEFT + RIGHT"
                       : turn_left_ ? "LEFT" : turn_right_ ? "RIGHT" : "";
    turn_signal_label_->setText(QString("<span style='color:%1'>&#9664;</span> &nbsp; "
                                       "<span style='color:%2'>&#9654;</span><br/>"
                                       "<span style='font-size:11px;color:%3'>%4</span>")
                                       .arg(hazard_ || turn_left_ ? lit : "#43576e",
                                            hazard_ || turn_right_ ? lit : "#43576e", lit, text));
}
