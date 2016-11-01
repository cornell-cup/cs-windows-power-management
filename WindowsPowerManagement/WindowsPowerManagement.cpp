// WindowsPowerManagement.cpp : Defines the entry point for the console application.
//

#include<stdio.h>
#include<Windows.h>
#include<PowrProf.h>


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
			for (int i = 0; i < nameSize; i+=2) printf("%c", name[i]);
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
				for (int i = 0; i < nameSize; i += 2) printf("%c", name[i]);
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
					for (int i = 0; i < nameSize; i += 2) printf("%c", name[i]);
					printf("\n");
				}
				else {
					printf("Setting Error: %d\n", error);
				}

				DWORD minvalue = 0;
				DWORD maxvalue = 0;
				nameSize = 2048;
				if ((error = PowerReadPossibleValue(0, &subguid, &settingguid, 0, settingIndex, name, &nameSize)) == 0) {
					PowerReadValueMin(0, &subguid, &settingguid, &minvalue);
					PowerReadValueMax(0, &subguid, &settingguid, &maxvalue);
					printf("      ");
					for (int i = 0; i < nameSize; i++) printf("%d ", name[i]);
					printf(" (%d - %d)\n", minvalue, maxvalue);
				}
				else {
					printf("Read Error: %d\n", error);
				}

				settingIndex++;
			}

			subgroupIndex++;
		}

		schemeIndex++;
	}
}

int main()
{
	GetAll();
	int pause;
	scanf_s("%d\n", &pause);
    return 0;
}

