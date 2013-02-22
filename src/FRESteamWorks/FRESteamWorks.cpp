/*
 *  FRESteamWorks.cpp
 *  This file is part of FRESteamWorks.
 *
 *  Created by David ´Oldes´ Oliva on 3/29/12.
 *  Contributors: Ventero <http://github.com/Ventero>
 *  Copyright (c) 2012 Amanita Design. All rights reserved.
 *  Copyright (c) 2012-2013 Level Up Labs, LLC. All rights reserved.
 */

#include "FRESteamWorks.h"

// Used to dispatch event back to AIR
FREContext AIRContext;

// Global access to Steam object
CSteam*	g_Steam = NULL;

// utility functions
FREObject FREBool(bool value) {
	FREObject result;
	FRENewObjectFromBool(value, &result);
	return result;
}

FREObject FREInt(int32 value) {
	FREObject result;
	FRENewObjectFromInt32(value, &result);
	return result;
}

FREObject FREFloat(float value) {
	FREObject result;
	FRENewObjectFromDouble(value, &result);
	return result;
}

FREObject FREString(std::string value) {
	FREObject result;
	FRENewObjectFromUTF8(value.size() + 1, (const uint8_t*)value.c_str(), &result);
	return result;
}

bool FREGetString(FREObject object, std::string& str) {
	uint32_t len;
	const uint8_t* data;
	FREResult res = FREGetObjectAsUTF8(object, &len, &data);
	if(res != FRE_OK) return false;

	str = std::string((const char*)data, len);
	return true;
}

void ANESteam::DispatchEvent(char* code, char* level) {
	// ignore unsuccessful dispatches
	FREDispatchStatusEventAsync(AIRContext, (const uint8_t*)code, (const uint8_t*)level);
}

extern "C" {

	FREObject AIRSteam_Init(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		// check if already initialized
		if (g_Steam) return FREBool(true);

		bool ret = SteamAPI_Init();
		if (ret) g_Steam = new ANESteam();

		return FREBool(ret);
	}

	FREObject AIRSteam_RunCallbacks(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		SteamAPI_RunCallbacks();
		return NULL;
	}

	FREObject AIRSteam_GetUserID(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if(!g_Steam) return FREString("");

		return FREString(g_Steam->GetUserID());
	}

	FREObject AIRSteam_GetPersonaName(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if(!g_Steam) return FREString("");

		return FREString(g_Steam->GetPersonaName());
	}

	FREObject AIRSteam_UseCrashHandler(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (argc != 4) return false;

		int32 appID = 0;
		if (FREGetObjectAsInt32(argv[0], &appID) != FRE_OK)
			return FREBool(false);

		std::string version, date, time;
		if(FREGetString(argv[1], version) != FRE_OK) return FREBool(false);
		if(FREGetString(argv[2], date) != FRE_OK) return FREBool(false);
		if(FREGetString(argv[3], time) != FRE_OK) return FREBool(false);

		SteamAPI_SetBreakpadAppID(SteamUtils()->GetAppID());
		SteamAPI_UseBreakpadCrashHandler(version.c_str(), date.c_str(), time.c_str(),
			false, NULL, NULL);
		return FREBool(true);
	}


	FREObject AIRSteam_RequestStats(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		bool ret = false;
		if (g_Steam) ret = g_Steam->RequestStats();

		SteamAPI_RunCallbacks();
		return FREBool(ret);
	}

	FREObject AIRSteam_SetAchievement(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) {
			// TODO: necessary to call this here?
			SteamAPI_RunCallbacks();
			return FREBool(false);
		}

		bool ret = false;
		std::string name;
		if(FREGetString(argv[0], name) == FRE_OK) {
			ret = g_Steam->SetAchievement(name.c_str());
		}

		SteamAPI_RunCallbacks();
		return FREBool(ret);
	}

	FREObject AIRSteam_ClearAchievement(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREBool(false);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREBool(false);

		return FREBool(g_Steam->ClearAchievement(name.c_str()));
	}

	FREObject AIRSteam_IsAchievement(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREBool(false);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREBool(false);

		return FREBool(g_Steam->IsAchievement(name.c_str()));
	}

	FREObject AIRSteam_GetStatInt(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREInt(0);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREInt(0);

		int32 value = 0;
		g_Steam->GetStat(name.c_str(), &value);
		return FREInt(value);
	}

	FREObject AIRSteam_GetStatFloat(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREFloat(0.0);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREFloat(0.0);

		float value = 0.0f;
		g_Steam->GetStat(name.c_str(), &value);
		return FREFloat(value);
	}

	FREObject AIRSteam_SetStatInt(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 2) return FREBool(false);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREBool(false);

		int32 value;
		if (FREGetObjectAsInt32(argv[1], &value) != FRE_OK) return FREBool(false);

		return FREBool(g_Steam->SetStat(name.c_str(), value));
	}
	FREObject AIRSteam_SetStatFloat(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 2) return FREBool(false);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREBool(false);

		double value;
		if (FREGetObjectAsDouble(argv[1], &value) != FRE_OK) return FREBool(false);

		return FREBool(g_Steam->SetStat(name.c_str(), (float)value));
	}

	FREObject AIRSteam_StoreStats(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam) return FREBool(false);

		return FREBool(g_Steam->StoreStats());
	}

	FREObject AIRSteam_ResetAllStats(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREBool(false);

		uint32_t achievementsToo;
		if (FREGetObjectAsBool(argv[0], &achievementsToo) != FRE_OK)
			return FREBool(false);

		return FREBool(g_Steam->ResetAllStats(achievementsToo != 0));
	}

	//Steam Cloud
	FREObject AIRSteam_GetFileCount(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam) return FREInt(0);

		return FREInt(SteamRemoteStorage()->GetFileCount());
	}

	FREObject AIRSteam_GetFileSize(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREInt(0);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREInt(0);

		return FREInt(SteamRemoteStorage()->GetFileSize(name.c_str()));
	}

	FREObject AIRSteam_FileExists(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREBool(false);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREBool(false);

		return FREBool(SteamRemoteStorage()->FileExists(name.c_str()));
	}

	FREObject AIRSteam_FileWrite(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 2) return FREBool(false);

		std::string name = FREGetString(argv[0]);
		if (name.empty()) return FREBool(false);

		FREByteArray byteArray;
		if (FREAcquireByteArray(argv[1], &byteArray) != FRE_OK)
			return false;

		bool ret = SteamRemoteStorage()->FileWrite(name.c_str(), byteArray.bytes, byteArray.length);
		FREReleaseByteArray(argv[1]);

		return FREBool(ret);
	}

	FREObject AIRSteam_FileRead(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 2) return FREBool(false);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREBool(false);

		FREByteArray byteArray;
		if (FREAcquireByteArray(argv[1], &byteArray) != FRE_OK)
			return FREBool(false);

		uint32 size = SteamRemoteStorage()->GetFileSize(name.c_str());
		int32 ret = 0;
		if (size > 0 && size <= byteArray.length) {
			char* data = (char*)malloc(size);
			ret = SteamRemoteStorage()->FileRead(name.c_str(), data, size);
			memcpy(byteArray.bytes, data, size);
			free(data);
		}
		FREReleaseByteArray(argv[1]);

		return FREBool(ret != 0);
	}

	FREObject AIRSteam_FileDelete(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREBool(false);

		std::string name;
		if(FREGetString(argv[0], name) != FRE_OK) return FREBool(false);

		return FREBool(SteamRemoteStorage()->FileDelete(name.c_str()));
	}

	FREObject AIRSteam_IsCloudEnabledForApp(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam) return FREBool(false);

		return FREBool(SteamRemoteStorage()->IsCloudEnabledForApp());
	}

	FREObject AIRSteam_SetCloudEnabledForApp(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		if (!g_Steam || argc != 1) return FREBool(false);

		uint32_t enabled = 0;
		if (FREGetObjectAsBool(argv[0], &enabled) != FRE_OK)
			return FREBool(false);

		bool bEnabled = (enabled != 0);
		SteamRemoteStorage()->SetCloudEnabledForApp(bEnabled);

		return FREBool(bEnabled == SteamRemoteStorage()->IsCloudEnabledForApp());
	}

	//============================

#define FRE_FUNC(fname) \
		{ (const uint8_t*) #fname, NULL, &fname }

	static FRENamedFunction func[] = {
		FRE_FUNC(AIRSteam_Init),
		FRE_FUNC(AIRSteam_RunCallbacks),
		FRE_FUNC(AIRSteam_GetUserID),
		FRE_FUNC(AIRSteam_GetPersonaName),
		FRE_FUNC(AIRSteam_UseCrashHandler),

		// stats / achievements
		FRE_FUNC(AIRSteam_RequestStats),
		FRE_FUNC(AIRSteam_SetAchievement),
		FRE_FUNC(AIRSteam_ClearAchievement),
		FRE_FUNC(AIRSteam_IsAchievement),
		FRE_FUNC(AIRSteam_GetStatInt),
		FRE_FUNC(AIRSteam_GetStatFloat),
		FRE_FUNC(AIRSteam_SetStatInt),
		FRE_FUNC(AIRSteam_SetStatFloat),
		FRE_FUNC(AIRSteam_StoreStats),
		FRE_FUNC(AIRSteam_ResetAllStats),

		// remote storage
		FRE_FUNC(AIRSteam_GetFileCount),
		FRE_FUNC(AIRSteam_GetFileSize),
		FRE_FUNC(AIRSteam_FileExists),
		FRE_FUNC(AIRSteam_FileWrite),
		FRE_FUNC(AIRSteam_FileRead),
		FRE_FUNC(AIRSteam_FileDelete),
		FRE_FUNC(AIRSteam_IsCloudEnabledForApp),
		FRE_FUNC(AIRSteam_SetCloudEnabledForApp)
	};

	// A native context instance is created
	void ContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx,
	                        uint32_t* numFunctions, const FRENamedFunction** functions) {
		AIRContext = ctx;
		*functions = func;
		*numFunctions = sizeof(func) / sizeof(func[0]);
	}

	// A native context instance is disposed
	void ContextFinalizer(FREContext ctx) {
		AIRContext = NULL;
		// Shutdown Steam
		SteamAPI_Shutdown();
		// Delete the SteamAchievements object
		if (g_Steam) g_Steam = NULL;
	}

	// Initialization function of each extension
	EXPORT void ExtInitializer(void** extDataToSet, FREContextInitializer* ctxInitializerToSet,
	                           FREContextFinalizer* ctxFinalizerToSet) {
		*extDataToSet = NULL;
		*ctxInitializerToSet = &ContextInitializer;
		*ctxFinalizerToSet = &ContextFinalizer;
	}

	// Called when extension is unloaded
	EXPORT void ExtFinalizer(void* extData) {
		return;
	}
}