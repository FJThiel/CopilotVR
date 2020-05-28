#ifndef TRACKEDDEVICE_H
#define TRACKEDDEVICE_H

#include <QtWidgets/QWidget>
#include <openvr.h>

class TrackedDevice
{
public:
    TrackedDevice();

    void setDeviceIndex(int index);
    void setSerial(char serial[1028]);
    void setManufacturer(char manufacturer[1028]);
    void setModel(char model[1028]);
    void setDeviceClassStr(std::string deviceClassStr);

    int getDeviceIndex();
    std::string getSerial();
    std::string getManufacturer();
    std::string getModel();
    std::string getDeviceClassStr();



private:
    int m_deviceIndex;
    std::string m_serial;
    std::string m_manufacturer;
    std::string m_model;
    std::string m_deviceClassStr;
};



#endif // TRACKEDDEVICE_H
