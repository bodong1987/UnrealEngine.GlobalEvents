// Fill out your copyright notice in the Description page of Project Settings.


#include "TestObject.h"
#include "GlobalEventsLog.h"
#include <inttypes.h>

#define __PrintDebugString() \
    UTestObject::PrintDebugString(   \
        FUNC_NAME, \
        boolValue, \
        uint8Value, \
        int32Value, \
        int64Value, \
        enumValue, \
        strValue, \
        strRef, \
        strConstRef, \
        nameValue, \
        nameRef, \
        vecValue, \
        vecRef, \
        vecConstRef, \
        objectTarget, \
        IntArrayValue, \
	    IntArrayRef, \
	    StringArrayValue, \
	    StringArrayRef, \
	    ObjectArrayValue, \
	    ObjectArrayRef, \
	    IntSetValue, \
	    NameSetRef, \
	    NameStringMapValue, \
	    IntStringMapRef, \
	    IntObjectMapValue, \
	    StringObjectMapRef \
    )

void UTestObject::TestUObjectUFunctionObserver(__TestParams)
{
    __PrintDebugString();
}

void UTestObject::TestUObjectMemberFunctionObserver(__TestParams)
{
    __PrintDebugString();
}

void UTestObject::TestUObjectStaticMemberFunctionObserver(__TestParams)
{
    __PrintDebugString();
}

void UTestObject::eventParamTests_Implementation(
    bool boolValue, 
    uint8 uint8Value, 
    int32 int32Value, 
    int64 int64Value,
    EGlobalEventParameterType enumValue, 
    const FString& strValue, 
    FString& strRef, 
    const FString& strConstRef, 
    FName nameValue,
    FName const& nameRef,
    FVector vecValue,
    FVector& vecRef,
    FVector const& vecConstRef, 
    UObject* objectTarget,
    const TArray<int32>& IntArrayValue, 
    TArray<int32> const& IntArrayRef,
    const TArray<FString>& StringArrayValue,
    TArray<FString> const& StringArrayRef,
    const TArray<UObject*>& ObjectArrayValue, 
    TArray<UObject*> const& ObjectArrayRef,
    const TSet<int32>& IntSetValue, 
    TSet<FName> const& NameSetRef, 
    const TMap<FName, FString>& NameStringMapValue, 
    TMap<int32, FString> const& IntStringMapRef, 
    const TMap<int32, UObject*>& IntObjectMapValue, 
    TMap<FString, UObject*> const& StringObjectMapRef
)
{
    
}

inline FString ConvertToString(int value)
{
    return FString::Printf(TEXT("%d"), value);
}

inline FString ConvertToString(const FString& value)
{
    return value;
}

inline FString ConvertToString(UObject* value)
{
    return GetNameSafe(value);
}

inline FString ConvertToString(FName value)
{
    return value.ToString();
}

template <typename TKey, typename TValue>
inline FString ConvertToString(const TTuple<TKey, TValue>& Pair)
{
    return FString::Printf(TEXT("{%s,%s}"), *ConvertToString(Pair.template Get<0>()), *ConvertToString(Pair.template Get<1>()));
}

void UTestObject::PrintDebugString(const FString& Tag, __TestParams)
{
    FString Message;
    Message += FString::Printf(TEXT("boolValue=%s\n"), boolValue ? TEXT("true") : TEXT("false"));
    Message += FString::Printf(TEXT("uint8Value=%d\n"), (int)uint8Value);
    Message += FString::Printf(TEXT("int32Value=%d\n"), int32Value);
    Message += FString::Printf(TEXT("int64Value=") PRId64 TEXT("\n"), int64Value);
    Message += FString::Printf(TEXT("enumValue=%s\n"), *StaticEnum<decltype(enumValue)>()->GetDisplayNameTextByValue((int)enumValue).ToString());

#define __PARAM_NAME(param) #param

#define APPEND_PARAM(param, text) \
    Message += FString::Printf(TEXT(__PARAM_NAME(param)) TEXT("=%s\n"), text);
    APPEND_PARAM(strValue, *strValue);
    APPEND_PARAM(strRef, *strRef);
    APPEND_PARAM(strConstRef, *strConstRef);
    APPEND_PARAM(nameValue, *nameValue.ToString());
    APPEND_PARAM(nameRef, *nameRef.ToString());
    APPEND_PARAM(vecValue, *vecValue.ToString());
    APPEND_PARAM(vecRef, *vecRef.ToString());
    APPEND_PARAM(vecConstRef, *vecConstRef.ToString());
    APPEND_PARAM(objectTarget, *GetNameSafe(objectTarget));

#define APPEND_ARRAY(array) \
    Message += TEXT(__PARAM_NAME(array)); \
    Message += TEXT("\n  "); \
    for(auto& value : array) \
    { \
        Message += ConvertToString(value); \
        Message += TEXT(", "); \
    }; \
    Message += TEXT("\n")

    APPEND_ARRAY(IntArrayValue);
    APPEND_ARRAY(IntArrayRef);
    APPEND_ARRAY(StringArrayValue);
    APPEND_ARRAY(StringArrayRef);
    APPEND_ARRAY(ObjectArrayValue);
    APPEND_ARRAY(ObjectArrayRef);
    APPEND_ARRAY(IntSetValue);
    APPEND_ARRAY(NameSetRef);
    APPEND_ARRAY(NameStringMapValue);
    APPEND_ARRAY(IntStringMapRef);
    APPEND_ARRAY(IntObjectMapValue);
    APPEND_ARRAY(StringObjectMapRef);


    UE_LOG(GlobalEventsLog, Log, TEXT("%s:\n%s"), *Tag, *Message);
}

void FRawTestsObject::TestRawMemberFunctionObserver(__TestParams)
{
    __PrintDebugString();
}

extern "C"
{
    void TestGlobalCFunctionObserver(__TestParams)
    {
        __PrintDebugString();
    }
}

void UTestObject::TestUFunctionWithReference(bool bv, bool& bref, FString sv, FString& sr, FVector vv, FVector& vr)
{
    bref = false;
    sr = GET_FUNCTION_NAME_CHECKED(UTestObject, TestUFunctionWithReference).ToString();
    vr = FVector(1024, 2048, 4096);
}

