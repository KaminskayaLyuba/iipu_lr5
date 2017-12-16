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
	TCHAR name[MAX_DEV_LEN];					//��� ����������
	GUID guid;									//GUID ����������
	TCHAR HID[MAX_DEV_LEN];						//hardware ID
	TCHAR mfg[MAX_DEV_LEN];						//�������������
	TCHAR provider[MAX_DEV_LEN];				//��������� ��������
	TCHAR driver[MAX_DEV_LEN];					//�������� ��������
	char *sys;									// ���� � .sys �����
	char *devicePath;							//���� � ���������� � �������
	HDEVINFO parent;							//����� � ������� ��������� ��� ����������
	SP_DEVINFO_DATA handle;						
	bool alive;									//����������/��������� ����������

	DWORD index;								//���������� ����� ���������� � ��� ������

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
	TCHAR text[MAX_DEV_LEN];					//�������� ������ ���������
	TCHAR desc[MAX_DEV_LEN];					
	vector<Device> devices;						//������ ��������� ���� ������
};


void getDevicePath(HDEVINFO hDevInfo, SP_DEVINFO_DATA spDevInfoData, Device* currentDevice)
{
	SP_DEVICE_INTERFACE_DATA spDevInterfaceData = { 0 };
	DWORD required = 0;

	spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	if (SetupDiCreateDeviceInterface(hDevInfo,				//�������� ���������� �� ����������
		&spDevInfoData,
		&spDevInfoData.ClassGuid,
		0,
		0,
		&spDevInterfaceData))

	{
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &spDevInterfaceData, 0, 0, &required, 0);		//�������� ������ ��������� ����������

		SP_DEVICE_INTERFACE_DETAIL_DATA_A *spDevInterfaceDetail;
		spDevInterfaceDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)LocalAlloc(LPTR,
			sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A)*required);

		spDevInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
		if (SetupDiGetDeviceInterfaceDetailA(hDevInfo,											//������� ��������� ���������� �� ����������
			&spDevInterfaceData,
			spDevInterfaceDetail,
			required,
			&required,
			0))
		{
			strcpy(currentDevice->devicePath, spDevInterfaceDetail->DevicePath);				//�������� DevicePath
			if (spDevInterfaceDetail)
				LocalFree(spDevInterfaceDetail);

		}
	}
}

void getDriverFullName(HDEVINFO hDevInfo, SP_DEVINFO_DATA spDevInfoData, Device* currentDevice)
{
	char serviceName[64];
	//Get service name of device
	if (SetupDiGetDeviceRegistryPropertyA(hDevInfo,					//�������� �������� ������� (��� ������)
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
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,						//��������� ���� �������
			szSubKey,
			0,
			KEY_ALL_ACCESS,
			&hKey) == ERROR_SUCCESS)
		{
			cbData = MAX_PATH - 1;
			dwType = REG_EXPAND_SZ;									//� ������� ���������� ������
			if (RegQueryValueExA(hKey, REG_IMAGE, 0L,				//������ �������� � ����� ������� (����)
				&dwType,
				(unsigned char*)szPath,
				&cbData) == ERROR_SUCCESS)
			{
				char szRoot[MAX_PATH] = { 0 };
				GetWindowsDirectoryA(szRoot, MAX_PATH - 1);			//�������� ����� �� ����
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
	SP_PROPCHANGE_PARAMS spPropChangeParams;													//��������� ����������

	spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
	spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;					//�������� ��������
	spPropChangeParams.Scope = DICS_FLAG_GLOBAL;												//���������� ��������
	if (!status)
		spPropChangeParams.StateChange = DISC_ENABLE;											//���� ���������, �� ��������
	else
		spPropChangeParams.StateChange = DISC_DISABLE;											//���� ��������, �� ���������
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


bool isEnabled(Device currentDevice)						//��������/��������� ����������
{
	SP_DEVINFO_DATA spDevInfoData = currentDevice.handle;
	ULONG status = 0, problem = 0;
	CONFIGRET cr = CM_Get_DevNode_Status(&status, &problem, spDevInfoData.DevInst, 0);
	return problem != PROBLEM_CODE;
}

vector<DeviceClass> allClasses;							//������ ���������� ��� ������

int EnumDeviceClasses(int index,						//����� ������
	TCHAR* DeviceClassName,								//��� ������
	TCHAR* DeviceClassDesc,								//�������� ������
	BOOL* DevicePresent)								//���������� �� ����������
{
	GUID ClassGuid;
	ZeroMemory(&ClassGuid, sizeof(GUID));

	HDEVINFO NewDeviceInfoSet;

	int result;
	TCHAR* name = new TCHAR[MAX_DEV_LEN];				
	BOOL resNam = FALSE;
	ULONG RequiredSize = MAX_DEV_LEN;

	result = CM_Enumerate_Classes(index, &ClassGuid, 0);									//�������� GUID �� �������

	*DevicePresent = FALSE;

	if (result == CR_INVALID_DATA)															//�������� ������������ ������
		return -2;

	if (result == CR_NO_SUCH_VALUE)															//���� ������ ������ ������� ����� �� �������
		return -1;

	if (result != CR_SUCCESS)																//��� ��������� ������
		return -3;

	resNam = SetupDiClassNameFromGuid(&ClassGuid, name, RequiredSize, &RequiredSize);		
	if (RequiredSize > 0)
	{
		delete[] name;
		name = new TCHAR[RequiredSize];
		resNam = SetupDiClassNameFromGuid(&ClassGuid, name, RequiredSize, &RequiredSize);
	}

	NewDeviceInfoSet = SetupDiGetClassDevs(&ClassGuid, 0, NULL, DIGCF_PRESENT);				//�������� ����� �� ������ ��������� ����� ������ (�� ������ ������ � �������)

	if (NewDeviceInfoSet == INVALID_HANDLE_VALUE)											//���� ������ �����
	{
		*DevicePresent = FALSE;
		wcscpy(DeviceClassName, name);

		delete[] name;
		name = NULL;
		return 0;
	}

	HKEY KeyClass = SetupDiOpenClassRegKeyEx(&ClassGuid,									//������� ���� ������� 
		MAXIMUM_ALLOWED,																	//����� ������� (������)
		DIOCR_INSTALLER,																	//������ �� �����������
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
		int res = RegQueryValue(KeyClass, NULL, DeviceClassDesc, &dwSize);					//������ � ����� ����� ������� �� �������� ������

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
	NewDeviceInfoSet = SetupDiGetClassDevs(&guids[0], 0, NULL, DIGCF_PRESENT);					//�������� ������ ��������� � ������ �� ����� GUID (������� � �������)

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

	res = SetupDiEnumDeviceInfo(NewDeviceInfoSet, index, &DeviceInfoData);						//�������� ������ ��� ���������� �� ������ index �� ������

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
	SP_DRVINFO_DATA driverInfo = {0};																	//��������� � ����������� � ��������
	driverInfo.cbSize = sizeof(SP_DRVINFO_DATA);
	res = SetupDiBuildDriverInfoList(NewDeviceInfoSet, &DeviceInfoData, SPDIT_COMPATDRIVER);			//��������� ������ ��������� ��� ����� ����������
	DWORD a = GetLastError();
	res = SetupDiEnumDriverInfo(NewDeviceInfoSet, &DeviceInfoData, SPDIT_COMPATDRIVER, 0, &driverInfo);	//�������� �������(��� � ������� �� � ������ ������ ��������) ������� �� ����� ������

	SP_DRVINFO_DETAIL_DATA driverDetailInfo = { 0 };													//��������� � ��������� ����������� � ��������
	driverDetailInfo.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
	//�������� ��������� ���������� � ��������
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
	TCHAR classes[MAX_DEV_LEN];															//��� ������
	TCHAR classesDesc[MAX_DEV_LEN];														//�������� ������

	BOOL DevExist = FALSE;																//���������� ����������
	int index = 0;																
	int res = EnumDeviceClasses(index, classes, classesDesc, &DevExist);				//�������� ���������� � ������
	while (res != -1)
	{
		DeviceClass a;
		if (res >= -1 && DevExist)
		{
			if (_wcsicmp(classesDesc, L"") == 0)										//���� �� ��������� �������� ������
				wcscpy(a.text, classes);												// �� �������� ��� ������
			else
				wcscpy(a.desc, classesDesc);											//����� ��������� �������� ������

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

