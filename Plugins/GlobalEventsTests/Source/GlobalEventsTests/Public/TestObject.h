// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Details/EventDefine.h"
#include "TestObject.generated.h"

#ifdef _MSC_VER
#define FUNC_NAME    TEXT(__FUNCTION__)
#else
#define FUNC_NAME    *FString(__FUNCTION__)
#endif

/**
 * 
 */
UCLASS()
class GLOBALEVENTSTESTS_API UTestObject : public UObject
{
	GENERATED_BODY()
	
public:
#define __TestParamsType \
	bool, \
	uint8, \
	int32, \
	int64, \
	EGlobalEventParameterType, \
	FString, \
	FString&, \
	const FString&, \
	FName, \
	const FName&, \
	FVector, \
	FVector&, \
	const FVector&, \
	UObject*, \
	TArray<int>, \
	const TArray<int>&, \
	TArray<FString>, \
	const TArray<FString>&, \
	TArray<UObject*>, \
	const TArray<UObject*>&, \
	TSet<int>, \
	const TSet<FName>&, \
	TMap<FName, FString>, \
	const TMap<int, FString>&, \
	TMap<int, UObject*>, \
	const TMap<FString, UObject*>&
	

#define __TestParams \
    bool boolValue, \
	uint8 uint8Value, \
    int32 int32Value, \
	int64 int64Value, \
	EGlobalEventParameterType enumValue, \
    FString strValue, \
	FString& strRef, \
    const FString& strConstRef, \
	FName nameValue, \
    const FName& nameRef, \
    FVector vecValue, \
	FVector& vecRef, \
    const FVector& vecConstRef, \
    UObject* objectTarget, \
	TArray<int> IntArrayValue, \
	const TArray<int>& IntArrayRef, \
	TArray<FString> StringArrayValue, \
	const TArray<FString>& StringArrayRef, \
	TArray<UObject*> ObjectArrayValue, \
	const TArray<UObject*>& ObjectArrayRef, \
	TSet<int> IntSetValue, \
	const TSet<FName>& NameSetRef, \
	TMap<FName, FString> NameStringMapValue, \
	const TMap<int, FString>& IntStringMapRef, \
	TMap<int, UObject*> IntObjectMapValue, \
	const TMap<FString, UObject*>& StringObjectMapRef

	void TestUObjectMemberFunctionObserver(__TestParams);
	static void TestUObjectStaticMemberFunctionObserver(__TestParams);

	UFUNCTION()
	void TestUObjectUFunctionObserver(
		bool boolValue, 
		uint8 uint8Value, 
		int32 int32Value, 
		int64 int64Value, 
		EGlobalEventParameterType enumValue, 
		FString strValue, 
		FString& strRef, 
		const FString& strConstRef, 
		FName nameValue, 
		const FName& nameRef, 
		FVector vecValue, 
		FVector& vecRef, 
		const FVector& vecConstRef, 
		UObject* objectTarget, 
		TArray<int> IntArrayValue, 
		const TArray<int>& IntArrayRef, 
		TArray<FString> StringArrayValue, 
		const TArray<FString>& StringArrayRef, 
		TArray<UObject*> ObjectArrayValue, 
		const TArray<UObject*>& ObjectArrayRef, 
		TSet<int> IntSetValue, 
		const TSet<FName>& NameSetRef, 
		TMap<FName, FString> NameStringMapValue, 
		const TMap<int, FString>& IntStringMapRef, 
		TMap<int, UObject*> IntObjectMapValue, 
		const TMap<FString, UObject*>& StringObjectMapRef
	);

	UFUNCTION(BlueprintNativeEvent)
	void eventParamTests(
		bool boolValue,
		uint8 uint8Value,
		int32 int32Value,
		int64 int64Value,
		EGlobalEventParameterType enumValue,
		const FString& strValue,
		FString& strRef,
		const FString& strConstRef,
		FName nameValue,
		const FName& nameRef,
		FVector vecValue,
		FVector& vecRef,
		const FVector& vecConstRef,
		UObject* objectTarget,
		const TArray<int>& IntArrayValue,
		const TArray<int>& IntArrayRef,
		const TArray<FString>& StringArrayValue,
		const TArray<FString>& StringArrayRef,
		const TArray<UObject*>& ObjectArrayValue,
		const TArray<UObject*>& ObjectArrayRef,
		const TSet<int>& IntSetValue,
		const TSet<FName>& NameSetRef,
		const TMap<FName, FString>& NameStringMapValue,
		const TMap<int, FString>& IntStringMapRef,
		const TMap<int, UObject*>& IntObjectMapValue,
		const TMap<FString, UObject*>& StringObjectMapRef
	);

	static void PrintDebugString(const FString& Tag, __TestParams);
};

class FRawTestsObject
{
public:
	void TestRawMemberFunctionObserver(__TestParams);
};

extern "C"
{
	void TestGlobalCFunctionObserver(__TestParams);
}

DEFINE_TYPESAFE_GLOBAL_EVENT(DebugEvent, __TestParamsType);
