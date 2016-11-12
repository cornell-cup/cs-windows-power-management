// WindowsPowerManagement.cpp : Defines the entry point for the console application.
//

#include<stdio.h>
#include<Windows.h>
#include<PowrProf.h>
#include<wchar.h>
#include<string>


void GetAll() {
	GUID guid;
	DWORD guidSize = sizeof(guid);
	unsigned int schemeIndex = 0;
	while (PowerEnumerate(0, 0, 0, ACCESS_SCHEME, schemeIndex, (UCHAR*)&guid, &guidSize) == 0) {
		// buffer contains the GUID
		UCHAR name[2048];
		DWORD nameSize = 2048;
		DWORD error;
		if ((error = PowerReadFriendlyName(0, &guid, &NO_SUBGROUP_GUID, 0, name, &nameSize)) == 0) {
			wprintf(L"%s", name);
			//for (int i = 0; i < nameSize; i+=2) printf("%c", name[i]);
			printf("\n");
		}
		else {
			printf("Error: %d\n", error);
		}

		GUID subguid;
		DWORD subguidSize = sizeof(subguid);
		unsigned int subgroupIndex = 0;
		while (PowerEnumerate(0, &guid, 0, ACCESS_SUBGROUP, subgroupIndex, (UCHAR*)&subguid, &subguidSize) == 0) {
			nameSize = 2048;
			if ((error = PowerReadFriendlyName(0, &guid, &subguid, 0, name, &nameSize)) == 0) {
				printf("  ");
				wprintf(L"%s", name);
				//for (int i = 0; i < nameSize; i += 2) printf("%c", name[i]);
				printf("\n");
			}
			else {
				printf("Subgroup Error: %d\n", error);
			}

			GUID settingguid;
			DWORD settingguidSize = sizeof(settingguid);
			unsigned int settingIndex = 0;
			while (PowerEnumerate(0, &guid, &subguid, ACCESS_INDIVIDUAL_SETTING, settingIndex, (UCHAR*)&settingguid, &settingguidSize) == 0) {
				nameSize = 2048;
				if ((error = PowerReadFriendlyName(0, &guid, &subguid, &settingguid, name, &nameSize)) == 0) {
					printf("    ");
					wprintf(L"%s", name);
					//for (int i = 0; i < nameSize; i += 1) printf("%c", name[i]);
					printf("\n");
				}
				else {
					printf("Setting Error: %d\n", error);
				}

				DWORD DCValueIndex = 0;
				DWORD DCDefaultIndex = 0;
				PowerReadDCValueIndex(0, &guid, &subguid, &settingguid, &DCValueIndex);
				PowerReadDCDefaultIndex(0, &guid, &subguid, &settingguid, &DCDefaultIndex);
				printf("    ");
				printf("  Value: %d, Default: %d\n", DCValueIndex, DCDefaultIndex);
				settingIndex++;
			}

			subgroupIndex++;
		}

		schemeIndex++;
	}
}

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

GUID GetSchemeGUID(char friendlyName[]) {
	GUID guid;
	DWORD guidSize = sizeof(guid);
	unsigned int schemeIndex = 0;
	GUID schemeguid = GUID_NULL;
	bool found = false;
	while (PowerEnumerate(0, 0, 0, ACCESS_SCHEME, schemeIndex, (UCHAR*)&guid, &guidSize) == 0) {
		UCHAR name[2048];
		DWORD nameSize = 2048;
		//printf("%s\n", friendlyName);
		std::wstring wideName = utf8_decode(friendlyName);
		//wprintf(L"%s\n", wideName.c_str());
		DWORD error;
		if ((error = PowerReadFriendlyName(0, &guid, &NO_SUBGROUP_GUID, 0, name, &nameSize)) == 0) {
			if (memcmp(name, reinterpret_cast<UCHAR*> (const_cast<wchar_t *> (wideName.c_str())), sizeof(name)) && found == false) {
				schemeguid = guid;
				found = true;
			}
		}
		else {
			printf("Error: %d\n", error);
		}
		schemeIndex++;
	}
	return schemeguid;
}

GUID GetSubgroupGUID(char friendlyName[], char friendlySubName[]) {
	GUID guid;
	DWORD guidSize = sizeof(guid);
	unsigned int schemeIndex = 0;
	GUID subgroupguid = GUID_NULL;
	bool found = false;
	while (PowerEnumerate(0, 0, 0, ACCESS_SCHEME, schemeIndex, (UCHAR*)&guid, &guidSize) == 0) {
		GUID subguid;
		DWORD subguidSize = sizeof(subguid);
		unsigned int subgroupIndex = 0;
		UCHAR name[2048];
		DWORD nameSize = 2048;
		std::wstring wideName = utf8_decode(friendlyName);
		DWORD error;
		if ((error = PowerReadFriendlyName(0, &guid, &NO_SUBGROUP_GUID, 0, name, &nameSize)) == 0) {
			if (memcmp(name, reinterpret_cast<UCHAR*> (const_cast<wchar_t *> (wideName.c_str())), sizeof(name)) && found == false) {
				while (PowerEnumerate(0, &guid, 0, ACCESS_SUBGROUP, subgroupIndex, (UCHAR*)&subguid, &subguidSize) == 0) {
					UCHAR subname[2048];
					DWORD subnameSize = 2048;
					std::wstring wideSubName = utf8_decode(friendlySubName);
					DWORD error;
					if ((error = PowerReadFriendlyName(0, &guid, &subguid, 0, subname, &subnameSize)) == 0) {
						if (memcmp(subname, reinterpret_cast<UCHAR*> (const_cast<wchar_t *> (wideSubName.c_str())), sizeof(subname)) && found == false) {
							subgroupguid = subguid;
							found = true;
						}
					}
					else {
						printf("Subgroup Error: %d\n", error);
					}
					subgroupIndex++;
				}
			}
		}
		else {
			printf("Error: %d\n", error);
		}
		
		schemeIndex++;
	}
	return subgroupguid;
}

int main()
{
	//GetAll();
	LPOLESTR strGuid;
	StringFromCLSID(GetSchemeGUID("Samsung Eco Mode"), &strGuid);
	wprintf(L"%s\n", strGuid);
	LPOLESTR strSubGuid;
	StringFromCLSID(GetSubgroupGUID("Samsung Eco Mode", "Hard disk"), &strSubGuid);
	wprintf(L"%s\n", strSubGuid);
	int pause;
	scanf_s("%d\n", &pause);
    return 0;
}

