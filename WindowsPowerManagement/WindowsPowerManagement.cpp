// WindowsPowerManagement.cpp : Defines the entry point for the console application.
//

#include<stdio.h>
#include<Windows.h>
#include<PowrProf.h>
#include<wchar.h>
#include<string>
#include<fstream>
#include<sstream>
#include<vector>

// Prints all Power Plans, their subgroups, and settings under each subgroup
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
					printf("\n");
				}
				else {
					printf("Setting Error: %d\n", error);
				}

				DWORD DCValueIndex = 0;
				DWORD DCDefaultIndex = 0;
				DWORD ACValueIndex = 0;
				DWORD ACDefaultIndex = 0;
				PowerReadDCValueIndex(0, &guid, &subguid, &settingguid, &DCValueIndex);
				PowerReadDCDefaultIndex(0, &guid, &subguid, &settingguid, &DCDefaultIndex);
				PowerReadACValueIndex(0, &guid, &subguid, &settingguid, &ACValueIndex);
				PowerReadACDefaultIndex(0, &guid, &subguid, &settingguid, &ACDefaultIndex);
				printf("    ");
				printf("  DC Value: %d, AC Value: %d, DC Default: %d, AC Default: %d\n", DCValueIndex, ACValueIndex, DCDefaultIndex, ACDefaultIndex);
				settingIndex++;
			}
			subgroupIndex++;
		}
		schemeIndex++;
	}
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

// Convert the string returned by the Power API to the same format as the inputted wstring
CHAR* remove_zeros(UCHAR a[])
{
	int  si;   // Source index
	int  di;   // Destination index
	int len = 0;
	int iCount = 0;
	while (a[iCount] != NULL || a[iCount + 1] != NULL)
	{
		iCount++;
		len++;
	}
	len = len + 1;
	CHAR* b = new CHAR[len];
	// Scan the array from left to right, removing '\x00' elements
	di = 0;
	for (si = 0; si < len; si++)
	{
		if ((unsigned int)a[si] != 0 && (unsigned int)a[si + 1] == 0) {
			b[di] = a[si];
			di++;
		}
	}
	b[di] = '\0'; // Indicates end of string
	return b;
}

// Returns GUID of the power plan based on its friendly name
GUID GetSchemeGUID(char friendlyName[]) {
	GUID guid;
	DWORD guidSize = sizeof(guid);
	unsigned int schemeIndex = 0;
	GUID schemeguid = GUID_NULL;
	bool found = false;
	while (PowerEnumerate(0, 0, 0, ACCESS_SCHEME, schemeIndex, (UCHAR*)&guid, &guidSize) == 0) {
		UCHAR name[2048];
		DWORD nameSize = 2048;
		std::wstring wideName = utf8_decode(friendlyName);
		DWORD error;
		if ((error = PowerReadFriendlyName(0, &guid, &NO_SUBGROUP_GUID, 0, name, &nameSize)) == 0) {
			if (strcmp(remove_zeros(name), friendlyName) == 0 && found == false) {
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

// Returns GUID of the subgroup based on its power plan's and its friendly name
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
			if (strcmp(remove_zeros(name), friendlyName) == 0 && found == false) {
				while (PowerEnumerate(0, &guid, 0, ACCESS_SUBGROUP, subgroupIndex, (UCHAR*)&subguid, &subguidSize) == 0) {
					UCHAR subname[2048];
					DWORD subnameSize = 2048;
					std::wstring wideSubName = utf8_decode(friendlySubName);
					DWORD error;
					if ((error = PowerReadFriendlyName(0, &guid, &subguid, 0, subname, &subnameSize)) == 0) {
						if (strcmp(remove_zeros(subname), friendlySubName) == 0 && found == false) {
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

// Returns GUID of the setting based on its power plans', subgroup's and its friendly names
GUID GetSettingGUID(char friendlyName[], char friendlySubName[], char friendlySettingName[]) {
	GUID guid;
	DWORD guidSize = sizeof(guid);
	unsigned int schemeIndex = 0;
	GUID settingsguid = GUID_NULL;
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
			if (strcmp(remove_zeros(name), friendlyName) == 0 && found == false) {
				while (PowerEnumerate(0, &guid, 0, ACCESS_SUBGROUP, subgroupIndex, (UCHAR*)&subguid, &subguidSize) == 0) {
					UCHAR subname[2048];
					DWORD subnameSize = 2048;
					std::wstring wideSubName = utf8_decode(friendlySubName);
					GUID settingguid;
					DWORD settingguidSize = sizeof(settingguid);
					unsigned int settingIndex = 0;
					DWORD error;
					if ((error = PowerReadFriendlyName(0, &guid, &subguid, 0, subname, &subnameSize)) == 0) {
						if (strcmp(remove_zeros(subname), friendlySubName) == 0 && found == false) {
							while (PowerEnumerate(0, &guid, &subguid, ACCESS_INDIVIDUAL_SETTING, settingIndex, (UCHAR*)&settingguid, &settingguidSize) == 0) {
								UCHAR settingname[2048];
								DWORD settingSize = 2048;
								std::wstring wideSettingName = utf8_decode(friendlySettingName);
								DWORD error;
								if ((error = PowerReadFriendlyName(0, &guid, &subguid, &settingguid, settingname, &settingSize)) == 0) {
									if (strcmp(remove_zeros(settingname), friendlySettingName) == 0 && found == false) {
										settingsguid = settingguid;
										found = true;
									}
								}
								else {
									printf("Setting Error: %d\n", error);
								}
								settingIndex++;
							}
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
	return settingsguid;
}

// Sets the nth setting under a specified power plan and subgroup with a user specified value, where n is the settingIndex
void SetSetting(GUID schemeguid, GUID subgroupguid, unsigned int settingIndex, DWORD value, unsigned int option) {
	GUID settingguid;
	DWORD settingguidSize = sizeof(settingguid);
	PowerEnumerate(0, &schemeguid, &subgroupguid, ACCESS_INDIVIDUAL_SETTING, settingIndex, (UCHAR*)&settingguid, &settingguidSize);
	DWORD DCValueIndex = value;
	DWORD ACValueIndex = value;
	if (option == 0) {
		PowerWriteDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, DCValueIndex);
		PowerWriteACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, ACValueIndex);
	}
	else if (option == 1) {
		PowerWriteACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, ACValueIndex);
	}
	else if (option == 2) {
		PowerWriteDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, DCValueIndex);
	}
}

// Sets the value of the setting specified by settingguid
void SetSetting(GUID schemeguid, GUID subgroupguid, GUID settingguid, DWORD value, unsigned int option) {
	DWORD DCValueIndex = value;
	DWORD ACValueIndex = value;
	if (option == 0) {
		PowerWriteDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, DCValueIndex);
		PowerWriteACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, ACValueIndex);
	}
	else if (option == 1) {
		PowerWriteACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, ACValueIndex);
	}
	else if (option == 2) {
		PowerWriteDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, DCValueIndex);
	}
}

// Resets the nth setting under a specified power plan and subgroup with its default value, where n is the settingIndex
void ResetToDefaultSetting(GUID schemeguid, GUID subgroupguid, unsigned int settingIndex, unsigned int option) {
	GUID settingguid;
	DWORD settingguidSize = sizeof(settingguid);
	PowerEnumerate(0, &schemeguid, &subgroupguid, ACCESS_INDIVIDUAL_SETTING, settingIndex, (UCHAR*)&settingguid, &settingguidSize);
	DWORD DCDefaultIndex = 0;
	DWORD ACDefaultIndex = 0;
	PowerReadDCDefaultIndex(0, &schemeguid, &subgroupguid, &settingguid, &DCDefaultIndex);
	PowerReadACDefaultIndex(0, &schemeguid, &subgroupguid, &settingguid, &ACDefaultIndex);
	if (option == 0) {
		PowerWriteDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, DCDefaultIndex);
		PowerWriteACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, ACDefaultIndex);
	}
	else if (option == 1) {
		PowerWriteACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, ACDefaultIndex);
	}
	else if (option == 2) {
		PowerWriteDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, DCDefaultIndex);
	}
}

// Resets the value of the setting specified by settingguid to its default
void ResetToDefaultSetting(GUID schemeguid, GUID subgroupguid, GUID settingguid, unsigned int option) {
	DWORD DCDefaultIndex = 0;
	DWORD ACDefaultIndex = 0;
	PowerReadDCDefaultIndex(0, &schemeguid, &subgroupguid, &settingguid, &DCDefaultIndex);
	PowerReadACDefaultIndex(0, &schemeguid, &subgroupguid, &settingguid, &ACDefaultIndex);
	if (option == 0) {
		PowerWriteDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, DCDefaultIndex);
		PowerWriteACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, ACDefaultIndex);
	}
	else if (option == 1) {
		PowerWriteACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, ACDefaultIndex);
	}
	else if (option == 2) {
		PowerWriteDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, DCDefaultIndex);
	}
}

// Returns the current value of the nth setting under a specified power plan and subgroup, where n is the settingIndex
DWORD GetSettingValue(GUID schemeguid, GUID subgroupguid, unsigned int settingIndex, unsigned int option) {
	GUID settingguid;
	DWORD settingguidSize = sizeof(settingguid);
	PowerEnumerate(0, &schemeguid, &subgroupguid, ACCESS_INDIVIDUAL_SETTING, settingIndex, (UCHAR*)&settingguid, &settingguidSize);
	DWORD DCValueIndex = 0;
	DWORD ACValueIndex = 0;
	if (option == 0) {
		printf("Error: Choose option = 1 for AC or option = 2 for DC");
		return -1;
	}
	else if (option == 1) {
		PowerReadACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, &ACValueIndex);
		return ACValueIndex;
	}
	else if (option == 2) {
		PowerReadDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, &DCValueIndex);
		return DCValueIndex;
	}
}

// Returns the current value of the setting specified by settingguid
DWORD GetSettingValue(GUID schemeguid, GUID subgroupguid, GUID settingguid, unsigned int option) {
	DWORD DCValueIndex = 0;
	DWORD ACValueIndex = 0;
	if (option == 0) {
		printf("Error: Choose option = 1 for AC or option = 2 for DC");
		return -1;
	}
	else if (option == 1) {
		PowerReadACValueIndex(0, &schemeguid, &subgroupguid, &settingguid, &ACValueIndex);
		return ACValueIndex;
	}
	else if (option == 2) {
		PowerReadDCValueIndex(0, &schemeguid, &subgroupguid, &settingguid, &DCValueIndex);
		return DCValueIndex;
	}
}

// Returns the default value of the nth setting under a specified power plan and subgroup, where n is the settingIndex
DWORD GetDefaultSettingValue(GUID schemeguid, GUID subgroupguid, unsigned int settingIndex, unsigned int option) {
	GUID settingguid;
	DWORD settingguidSize = sizeof(settingguid);
	PowerEnumerate(0, &schemeguid, &subgroupguid, ACCESS_INDIVIDUAL_SETTING, settingIndex, (UCHAR*)&settingguid, &settingguidSize);
	DWORD DCDefaultIndex = 0;
	DWORD ACDefaultIndex = 0;
	if (option == 0) {
		printf("Error: Choose option = 1 for AC or option = 2 for DC");
		return -1;
	}
	else if (option == 1) {
		PowerReadACDefaultIndex(0, &schemeguid, &subgroupguid, &settingguid, &ACDefaultIndex);
		return ACDefaultIndex;
	}
	else if (option == 2) {
		PowerReadDCDefaultIndex(0, &schemeguid, &subgroupguid, &settingguid, &DCDefaultIndex);
		return DCDefaultIndex;
	}
}

// Returns the default value of the setting specified by settingguid
DWORD GetDefaultSettingValue(GUID schemeguid, GUID subgroupguid, GUID settingguid, unsigned int option) {
	DWORD DCDefaultIndex = 0;
	DWORD ACDefaultIndex = 0;
	if (option == 0) {
		printf("Error: Choose option = 1 for AC or option = 2 for DC");
		return -1;
	}
	else if (option == 1) {
		PowerReadACDefaultIndex(0, &schemeguid, &subgroupguid, &settingguid, &ACDefaultIndex);
		return ACDefaultIndex;
	}
	else if (option == 2) {
		PowerReadDCDefaultIndex(0, &schemeguid, &subgroupguid, &settingguid, &DCDefaultIndex);
		return DCDefaultIndex;
	}
}

// Helper function for parsing input from file
void split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}

// Helper function for parsing input from file
std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

// Sets all the settings and values listed in an inputted file
void handleFile(std::ifstream& infile) {
	std::string line;
	while (std::getline(infile, line)) {
		std::vector<std::string> values = split(line, ',');
		GUID scheme = GetSchemeGUID((CHAR*)values[1].c_str());
		GUID subgroup = GetSubgroupGUID((CHAR*)values[1].c_str(), ((CHAR*)values[2].c_str()));
		if (strcmp(values[0].c_str(),"index") == 0) {
			if (strcmp(values[4].c_str(), "default") == 0) {
				ResetToDefaultSetting(scheme, subgroup, atoi(values[3].c_str()), atoi(values[5].c_str()));
			}
			else {
				SetSetting(scheme, subgroup, atoi(values[3].c_str()), atoi(values[4].c_str()), atoi(values[5].c_str()));
			}
		}
		else if (strcmp(values[0].c_str(), "name") == 0) {
			GUID setting = GetSettingGUID((CHAR*)values[1].c_str(), (CHAR*)values[2].c_str(), (CHAR*)values[3].c_str());
			if (strcmp(values[4].c_str(), "default") == 0) {
				ResetToDefaultSetting(scheme, subgroup, setting, atoi(values[5].c_str()));
			}
			else {
				SetSetting(scheme, subgroup, setting, atoi(values[4].c_str()), atoi(values[5].c_str()));
			}
		}
	}
}

void ResetAllToDefault() {
	GUID guid;
	DWORD guidSize = sizeof(guid);
	unsigned int schemeIndex = 0;
	while (PowerEnumerate(0, 0, 0, ACCESS_SCHEME, schemeIndex, (UCHAR*)&guid, &guidSize) == 0) {
		UCHAR name[2048];
		DWORD nameSize = 2048;
		DWORD error;
		if ((error = PowerReadFriendlyName(0, &guid, &NO_SUBGROUP_GUID, 0, name, &nameSize)) != 0) {
			printf("Error: %d\n", error);
		}

		GUID subguid;
		DWORD subguidSize = sizeof(subguid);
		unsigned int subgroupIndex = 0;
		while (PowerEnumerate(0, &guid, 0, ACCESS_SUBGROUP, subgroupIndex, (UCHAR*)&subguid, &subguidSize) == 0) {
			nameSize = 2048;
			if ((error = PowerReadFriendlyName(0, &guid, &subguid, 0, name, &nameSize)) != 0) {
				printf("Subgroup Error: %d\n", error);
			}

			GUID settingguid;
			DWORD settingguidSize = sizeof(settingguid);
			unsigned int settingIndex = 0;
			while (PowerEnumerate(0, &guid, &subguid, ACCESS_INDIVIDUAL_SETTING, settingIndex, (UCHAR*)&settingguid, &settingguidSize) == 0) {
				nameSize = 2048;
				if ((error = PowerReadFriendlyName(0, &guid, &subguid, &settingguid, name, &nameSize)) == 0) {
					ResetToDefaultSetting(guid, subguid, settingguid, 0);
				}
				else {
					printf("Setting Error: %d\n", error);
				}
				
				DWORD DCValueIndex = 0;
				DWORD DCDefaultIndex = 0;
				DWORD ACValueIndex = 0;
				DWORD ACDefaultIndex = 0;
				PowerReadDCValueIndex(0, &guid, &subguid, &settingguid, &DCValueIndex);
				PowerReadDCDefaultIndex(0, &guid, &subguid, &settingguid, &DCDefaultIndex);
				PowerReadACValueIndex(0, &guid, &subguid, &settingguid, &ACValueIndex);
				PowerReadACDefaultIndex(0, &guid, &subguid, &settingguid, &ACDefaultIndex);
				settingIndex++;
			}
			subgroupIndex++;
		}
		schemeIndex++;
	}
	printf("Reset all settings.\n");
}

int main()
{
	std::ifstream infile("settings_input.txt");
	handleFile(infile);
	printf("\n\n");
	ResetAllToDefault();
	//GetAll();
	int pause;
	scanf_s("%d\n", &pause);
    return 0;
}

