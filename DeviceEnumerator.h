#pragma once
#include <vector>
#include <string>
#include <set>
#include <Windows.h>
#include <setupapi.h>
#include "Device.h"
using namespace std;

#pragma comment(lib, "setupapi.lib")

typedef struct DEVICE_INFO
{
	HDEVINFO hDevInfo;						//хендл семейства 
	SP_DEVINFO_DATA spDevInfoData;			//хендл устройства
	string classDescription;				//описание класса
	string deviceName;						//имя устройства
	string guid_string;						//GUID стокой
	GUID guid;								//GUID
	string hardwareID;						//hardwareID
	string manufacturer;					//производитель
	string provider;						//провайдер
	string driverDescription;				//описание драйвера
	string devicePath;						//путь к устройству
	string driverFullName;					//полнй путь к драйверу
	bool isEnabled;							//состояне устройства
}DEV_INFO;

class DeviceEnumerator
{
private:
	static vector<DEVICE_INFO> vectorDeviceInfo;
public:
	static vector<DEVICE_INFO> getDevices();
	static set<string> getDeviceTypes();
	DeviceEnumerator();
	~DeviceEnumerator();
};

