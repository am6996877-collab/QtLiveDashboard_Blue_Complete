#include "CANWorker.h"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

CANWorker::CANWorker(QObject* parent) : QObject(parent) {}
CANWorker::~CANWorker() { if (can_socket_ >= 0) close(can_socket_); }

bool CANWorker::openSocket() {
    can_socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket_ < 0) return false;
    struct ifreq ifr{};
    std::strncpy(ifr.ifr_name, "vcan0", IFNAMSIZ - 1);
    struct sockaddr_can address{};
    address.can_family = AF_CAN;
    if (ioctl(can_socket_, SIOCGIFINDEX, &ifr) == 0) {
        address.can_ifindex = ifr.ifr_ifindex;
        if (bind(can_socket_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0)
            return true;
    }
    close(can_socket_);
    can_socket_ = -1;
    return false;
}

void CANWorker::run() {
    if (!running_) return;
    if (!openSocket()) {
        qWarning("CANWorker: cannot open vcan0; check interface setup.");
        return;
    }
    while (running_) {
        timeval timeout{0, 500000};
        fd_set ready;
        FD_ZERO(&ready);
        FD_SET(can_socket_, &ready);
        int result = select(can_socket_ + 1, &ready, nullptr, nullptr, &timeout);
        if (result < 0) {
            if (errno == EINTR) continue;
            qWarning("CANWorker: select failed.");
            break;
        }
        if (result == 0 || !running_) continue;
        struct can_frame frame{};
        const auto count = read(can_socket_, &frame, sizeof(frame));
        if (count < 0) {
            if (errno == EINTR) continue;
            qWarning("CANWorker: read failed.");
            break;
        }
        if (count != sizeof(frame) || frame.can_dlc > 8) continue;
        if (frame.can_id & (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_ERR_FLAG)) continue;
        const auto id = frame.can_id;
        const int required = id == 0x0C0 ? 8 : (id == 0x0D0 || id == 0x320) ? 3 : id == 0x0C1 ? 1 : 0;
        if (required && frame.can_dlc >= required) decodeAndEmit(id, frame.data);
    }
    close(can_socket_);
    can_socket_ = -1;
}

void CANWorker::stop() { running_ = false; }

void CANWorker::decodeAndEmit(uint32_t id, const uint8_t* data) {
    if (id == 0x0C0) {
        emit rpmUpdated(static_cast<int>((data[0] + data[1] * 256) * 0.25f));
        emit tempUpdated(data[4] * 0.5f - 40.0f);
        emit throttleUpdated(data[5] * 0.4f);
        emit engineRunningChanged((data[6] & 1) != 0);
    } else if (id == 0x0D0) {
        emit speedUpdated((data[1] + data[2] * 256) * 0.01f);
        emit gearUpdated(data[0]);
    } else if (id == 0x320) {
        emit doorStatusUpdated(data[0] & 1, data[0] & 2, data[0] & 4, data[0] & 8);
        emit hazardChanged((data[0] & 32) != 0);
        emit ignitionChanged((data[1] & 1) != 0);
        emit turnSignalsChanged((data[1] & 2) != 0, (data[1] & 4) != 0);
        emit batteryVoltageUpdated(data[2] * 0.1f);
    } else if (id == 0x0C1) {
        emit faultStatusUpdated((data[0] & 1) != 0);
    }
}
