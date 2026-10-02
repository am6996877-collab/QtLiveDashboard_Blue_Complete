#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QThread>
#include "SpeedometerWidget.h"
#include "RPMGaugeWidget.h"
#include "CheckEngineIcon.h"
#include "CANWorker.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    SpeedometerWidget* speedometer_;
    QProgressBar* rpm_bar_;
    QProgressBar* fuel_bar_;
    QLabel* gear_label_;
    QLabel* temp_label_;
    QLabel* door_fl_label_;
    QLabel* door_fr_label_;
    QLabel* door_rl_label_;
    QLabel* door_rr_label_;
    QProgressBar* throttle_bar_;
    QLabel* engine_status_label_;
    QLabel* ignition_label_;
    QLabel* turn_signal_label_;
    QLabel* battery_label_;
    CheckEngineIcon* cel_icon_;
    QLabel* fault_label_;

    RPMGaugeWidget* rpm_gauge_;
    bool turn_left_ = false, turn_right_ = false, hazard_ = false;
    void updateTurnSignalLabel();
    QThread* can_thread_ = nullptr;
    CANWorker* can_worker_ = nullptr;
};
