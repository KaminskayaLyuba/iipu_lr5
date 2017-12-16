#pragma once
#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <iostream>
#include <cfgmgr32.h>
#include <vector>
using namespace std;
#pragma comment (lib, "Setupapi.lib")
#pragma comment (lib, "Advapi32.lib")

#define MAX_DEV_LEN 1000
#define REG_PATH "SYSTEM\\CurrentControlSet\\Services\\\0"
#define REG_IMAGE "ImagePath"
#define DISC_DISABLE 2
#define DISC_ENABLE 1
#define PROBLEM_CODE 22

class Device												
{
public:
	TCHAR name[MAX_DEV_LEN];					//имя устройства
	GUID guid;									//GUID устройства
	TCHAR HID[MAX_DEV_LEN];						//hardware ID
	TCHAR mfg[MAX_DEV_LEN];						//производитель
	TCHAR provider[MAX_DEV_LEN];				//создатель драйвера
	TCHAR driver[MAX_DEV_LEN];					//описание драйвера
	char *sys;									// путь к .sys файлу
	char *devicePath;							//путь к устройству в системе
	HDEVINFO parent;							//класс к кторому отмосится это устройство
	SP_DEVINFO_DATA handle;						
	bool alive;									//подключено/отключено устройство

	DWORD index;								//порядковый номер устройства в его классе

	Device()
	{
		devicePath = new char[250];
		sys = new char[150];
	}

	Device(LPCWSTR name)
	{
		wcscpy(this->name, name);
	}

};

class DeviceClass
{
public:
	TCHAR text[MAX_DEV_LEN];					//название класса устройств
	TCHAR desc[MAX_DEV_LEN];					
	vector<Device> devices;						//вектор устройств этоо класса
};


void getDevicePath(HDEVINFO hDevInfo, SP_DEVINFO_DATA spDevInfoData, Device* currentDevice)
{
	SP_DEVICE_INTERFACE_DATA spDevInterfaceData = { 0 };
	DWORD required = 0;

	spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	if (SetupDiCreateDeviceInterface(hDevInfo,				//получаем информацию об интерфейсе
		&spDevInfoData,
		&spDevInfoData.ClassGuid,
		0,
		0,
		&spDevInterfaceData))

	{
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &spDevInterfaceData, 0, 0, &required, 0);		//получаем размер детальной информации

		SP_DEVICE_INTERFACE_DETAIL_DATA_A *spDevInterfaceDetail;
		spDevInterfaceDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)LocalAlloc(LPTR,
			sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A)*required);

		spDevInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
		if (SetupDiGetDeviceInterfaceDetailA(hDevInfo,											//плучаем детальную информацию об интерфейсе
			&spDevInterfaceData,
			spDevInterfaceDetail,
			required,
			&required,
			0))
		{
			strcpy(currentDevice->devicePath, spDevInterfaceDetail->DevicePath);				//получаем DevicePath
			if (spDevInterfaceDetail)
				LocalFree(spDevInterfaceDetail);

		}
	}
}

void getDriverFullName(HDEVINFO hDevInfo, SP_DEVINFO_DATA spDevInfoData, Device* currentDevice)
{
	char serviceName[64];
	//Get service name of device
	if (SetupDiGetDeviceRegistryPropertyA(hDevInfo,					//получаем свойства реестра (имя службы)
		&spDevInfoData,
		SPDRP_SERVICE,
		0,
		(PBYTE)serviceName,
		63,
		0))
	{
		HKEY  hKey = 0;
		char szSubKey[128] = { REG_PATH };
		char szPath[MAX_PATH] = { 0 };
		DWORD cbData;
		DWORD dwType;
		strcat(szSubKey, serviceName);
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,						//открываем ключ реестра
			szSubKey,
			0,
			KEY_ALL_ACCESS,
			&hKey) == ERROR_SUCCESS)
		{
			cbData = MAX_PATH - 1;
			dwType = REG_EXPAND_SZ;									//в формате читабелной строки
			if (RegQueryValueExA(hKey, REG_IMAGE, 0L,				//запрос значения к ключу реестра (путь)
				&dwType,
				(unsigned char*)szPath,
				&cbData) == ERROR_SUCCESS)
			{
				char szRoot[MAX_PATH] = { 0 };
				GetWindowsDirectoryA(szRoot, MAX_PATH - 1);			//получить папку по пути
				strcat(szRoot, "\\");
				strcat(szRoot, szPath);
				strcpy(currentDevice->sys, szPath);
			}
		}
	}
}

bool deviceChangeStatus(Device currentDevice)
{
	HDEVINFO hDevInfo = currentDevice.parent;
	SP_DEVINFO_DATA spDevInfoData = currentDevice.handle; 
	bool status = currentDevice.alive;
	SP_PROPCHANGE_PARAMS spPropChangeParams;													//изменение параметров

	spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
	spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;					//изменить свойство
	spPropChangeParams.Scope = DICS_FLAG_GLOBAL;												//глобальное свойство
	if (!status)
		spPropChangeParams.StateChange = DISC_ENABLE;											//если выключено, то включить
	else
		spPropChangeParams.StateChange = DISC_DISABLE;											//если включено, то выключить
	DWORD errorCode;
	//   
	if (SetupDiSetClassInstallParams(hDevInfo, &spDevInfoData, &spPropChangeParams.ClassInstallHeader, sizeof(spPropChangeParams)))
	{
		if (SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &spDevInfoData))
		{
			return true;
		}
	}
	errorCode = GetLastError();
	return false;
}


bool isEnabled(Device currentDevice)						//включено/выключено устройство
{
	SP_DEVINFO_DATA spDevInfoData = currentDevice.handle;
	ULONG status = 0, problem = 0;
	CONFIGRET cr = CM_Get_DevNode_Status(&status, &problem, spDevInfoData.DevInst, 0);
	return problem != PROBLEM_CODE;
}

vector<DeviceClass> allClasses;							//вектор содержащий все классы

int EnumDeviceClasses(int index,						//номер класса
	TCHAR* DeviceClassName,								//имя класса
	TCHAR* DeviceClassDesc,								//описание класса
	BOOL* DevicePresent)								//существует ли устройства
{
	GUID ClassGuid;
	ZeroMemory(&ClassGuid, sizeof(GUID));

	HDEVINFO NewDeviceInfoSet;

	int result;
	TCHAR* name = new TCHAR[MAX_DEV_LEN];				
	BOOL resNam = FALSE;
	ULONG RequiredSize = MAX_DEV_LEN;

	result = CM_Enumerate_Classes(index, &ClassGuid, 0);									//получить GUID по индексу

	*DevicePresent = FALSE;

	if (result == CR_INVALID_DATA)															//заведомо некарректные данные
		return -2;

	if (result == CR_NO_SUCH_VALUE)															//если индекс списка классов вышел за границу
		return -1;

	if (result != CR_SUCCESS)																//все остальные ошибки
		return -3;

	resNam = SetupDiClassNameFromGuid(&ClassGuid, name, RequiredSize, &RequiredSize);		
	if (RequiredSize > 0)
	{
		delete[] name;
		name = new TCHAR[RequiredSize];
		resNam = SetupDiClassNameFromGuid(&ClassGuid, name, RequiredSize, &RequiredSize);
	}

	NewDeviceInfoSet = SetupDiGetClassDevs(&ClassGuid, 0, NULL, DIGCF_PRESENT);				//получить хендл на список устройств этого класса (на данный момент в системе)

	if (NewDeviceInfoSet == INVALID_HANDLE_VALUE)											//если пустой хендл
	{
		*DevicePresent = FALSE;
		wcscpy(DeviceClassName, name);

		delete[] name;
		name = NULL;
		return 0;
	}

	HKEY KeyClass = SetupDiOpenClassRegKeyEx(&ClassGuid,									//открыть ключ реестра 
		MAXIMUM_ALLOWED,																	//права доступа (полные)
		DIOCR_INSTALLER,																	//данные об установщике
		NULL,
		0);

	if (KeyClass == INVALID_HANDLE_VALUE)
	{
		*DevicePresent = FALSE;
		wcscpy(DeviceClassName, name);

		delete[] name;
		name = NULL;
		return 0;
	}
	else
	{
		long dwSize = MAX_DEV_LEN;
		int res = RegQueryValue(KeyClass, NULL, DeviceClassDesc, &dwSize);					//запрос к этому ключу реестра об описании класса

		if (res != ERROR_SUCCESS)
			wcscpy(DeviceClassDesc, L"");
	}

	wcscpy(DeviceClassName, name);
	*DevicePresent = TRUE;

	RegCloseKey(KeyClass);

	delete[] name;
	name = NULL;
	return 0;
}

int EnumDevices(int index,
	TCHAR* DeviceClassName,
	Device* currentDevice)
{
	ULONG RequiredSize = 0;
	GUID* guids = new GUID[1];

	HDEVINFO NewDeviceInfoSet;
	SP_DEVINFO_DATA DeviceInfoData;

	BOOL res = SetupDiClassGuidsFromName(DeviceClassName, &guids[0],
		RequiredSize, &RequiredSize);

	if (RequiredSize == 0)
	{
		//incorrect class name:
		wcscpy(currentDevice->name, L"");
		return -2;
	}

	if (!res)
	{
		delete[] guids;
		guids = new GUID[RequiredSize];
		res = SetupDiClassGuidsFromName(DeviceClassName, &guids[0],
			RequiredSize, &RequiredSize);

		if (!res || RequiredSize == 0)
		{
			//incorrect class name:
			wcscpy(currentDevice->name, L"");
			return -2;
		}
	}

	//get device info set for our device class
	NewDeviceInfoSet = SetupDiGetClassDevs(&guids[0], 0, NULL, DIGCF_PRESENT);					//получить список устройств в классе по этому GUID (которые в системе)

	if (NewDeviceInfoSet == INVALID_HANDLE_VALUE)
		if (!res)
		{
			//device information is unavailable:
			wcscpy(currentDevice->name, L"");
			return -3;
		}

	DeviceInfoData.cbSize = 28;
	//is devices exist for class
	DeviceInfoData.DevInst = 0;
	ZeroMemory(&DeviceInfoData.ClassGuid, sizeof(GUID));
	DeviceInfoData.Reserved = 0;

	res = SetupDiEnumDeviceInfo(NewDeviceInfoSet, index, &DeviceInfoData);						//получить данные для устройства по номеру index из списка

	if (!res)
	{
		wcscpy(currentDevice->name, L"");
		return -1;
	}


	if (!SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet, &DeviceInfoData,					
		SPDRP_FRIENDLYNAME, 0, (BYTE*)currentDevice->name,
		MAX_DEV_LEN, NULL))
	{
		res = SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet, &DeviceInfoData,
			SPDRP_DEVICEDESC, 0, (BYTE*)currentDevice->name,
			MAX_DEV_LEN, NULL);
		if (!res)
		{
			wcscpy(currentDevice->name, L"");
			return -4;
		}
	}
	currentDevice->guid = DeviceInfoData.ClassGuid;
	currentDevice->handle = DeviceInfoData;
	currentDevice->parent = NewDeviceInfoSet;
	SP_DRVINFO_DATA driverInfo = {0};																	//структура с информацией о драйвере
	driverInfo.cbSize = sizeof(SP_DRVINFO_DATA);
	res = SetupDiBuildDriverInfoList(NewDeviceInfoSet, &DeviceInfoData, SPDIT_COMPATDRIVER);			//построить список драйверов для этого устройства
	DWORD a = GetLastError();
	res = SetupDiEnumDriverInfo(NewDeviceInfoSet, &DeviceInfoData, SPDIT_COMPATDRIVER, 0, &driverInfo);	//получить нулевой(тот с которым он в данный момент работает) драйвер из этого списка

	SP_DRVINFO_DETAIL_DATA driverDetailInfo = { 0 };													//структура с подробной информацией о драйвере
	driverDetailInfo.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
	//получить подробную информацию о драйвере
	res = SetupDiGetDriverInfoDetail(NewDeviceInfoSet, &DeviceInfoData, &driverInfo, &driverDetailInfo, sizeof(SP_DRVINFO_DETAIL_DATA), &a);
	wcscpy(currentDevice->driver, driverDetailInfo.DrvDescription);
	wcscpy(currentDevice->provider, driverInfo.ProviderName);

	res = SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet, &DeviceInfoData,
		SPDRP_HARDWAREID, 0, (BYTE*)currentDevice->HID,
		MAX_DEV_LEN, NULL);

	res = SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet, &DeviceInfoData,
		SPDRP_MFG, 0, (BYTE*)currentDevice->mfg,
		MAX_DEV_LEN, NULL);
	getDevicePath(NewDeviceInfoSet,DeviceInfoData,currentDevice);
	getDriverFullName(NewDeviceInfoSet, DeviceInfoData, currentDevice);
	currentDevice->alive = isEnabled(currentDevice[0]);

	return 0;
}



void EnumDevices()
{
	DWORD dwSize = MAX_PATH;																
	TCHAR classes[MAX_DEV_LEN];															//имя класса
	TCHAR classesDesc[MAX_DEV_LEN];														//описание класса

	BOOL DevExist = FALSE;																//устройство существует
	int index = 0;																
	int res = EnumDeviceClasses(index, classes, classesDesc, &DevExist);				//получить информацию о классе
	while (res != -1)
	{
		DeviceClass a;
		if (res >= -1 && DevExist)
		{
			if (_wcsicmp(classesDesc, L"") == 0)										//если не вернулось описание класса
				wcscpy(a.text, classes);												// то запмнить имя класса
			else
				wcscpy(a.desc, classesDesc);											//иначе запомнить описание класса

			int result, DevIndex = 0;
			TCHAR DeviceName[MAX_DEV_LEN] = L"";
			Device newDevice;
			result = EnumDevices(DevIndex, classes, &newDevice);

			while (result != -1)
			{
				if (result == 0)
				{
					a.devices.push_back(newDevice);
				}

				DevIndex++;
				result = EnumDevices(DevIndex, classes, &newDevice);
			}

		}

		index++;
		res = EnumDeviceClasses(index, classes, classesDesc, &DevExist);
		if (a.devices.size()>0) allClasses.push_back(a);
	}

}

