# cs-windows-power-management

TODO:

write DC values, read/write AC values

* GetSchemeGuid(string friendlyname)
* GetSubgroupGuid(string friendlyname)
* GetSettingGuid(string friendlyname)
* SetSetting(GUID subgroupguid, unsigned int settingIndex, DWORD value, unsigned int option)
* SetSetting(GUID settingguid, DWORD value, unsigned int option)
* ResetToDefaultSetting(GUID subgroupguid, unsigned int settingIndex, unsigned int option)
* ResetToDefaultSetting(GUID settingguid, unsigned int option)
* GetSettingValue(GUID subgroupguid, unsigned int settingIndex, unsigned int option)
* GetSettingValue(GUID settingguid, unsigned int option)
* GetDefaultSettingValue(GUID subgroupguid, unsigned int settingIndex, unsigned int option)
* GetDefaultSettingValue(GUID settingguid, unsigned int option)
* ApplySchemeGuid(string friendlyname) -> SetActiveScheme
* DeletePowerScheme(string friendlyname) -> DeletePwrScheme
* CreateCustomeScheme(string friendlyname) -> PowerGetActiveScheme -> PowerDuplicateActiveScheme -> PowerSetActiveScheme

option:
	0: both
	1: AC only
	2: DC only