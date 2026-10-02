#pragma once
#include <QObject>
#include <cstdint>
#include <atomic>

class CANWorker : public QObject {
    Q_OBJECT
public:
    explicit CANWorker(QObject* parent = nullptr);
    ~CANWorker();

public slots:
    void run();
    void stop();

signals:
    void rpmUpdated(int rpm);
    void speedUpdated(float speed_kmh);
    void tempUpdated(float temp_c);
    void gearUpdated(int gear);
    void doorStatusUpdated(bool fl, bool fr, bool rl, bool rr);
    void faultStatusUpdated(bool cel_on);
    void throttleUpdated(float throttle_pct);
    void engineRunningChanged(bool running);
    void ignitionChanged(bool on);
    void turnSignalsChanged(bool left, bool right);
    void hazardChanged(bool on);
    void batteryVoltageUpdated(float volts);

private:
    bool openSocket();
    void decodeAndEmit(uint32_t id, const uint8_t* data);

    int  can_socket_ = -1;
    // Set before the thread starts; run() must not undo an early stop().
    std::atomic<bool> running_{true};
};
