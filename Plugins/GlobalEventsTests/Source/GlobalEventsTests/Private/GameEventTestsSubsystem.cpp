// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEventTestsSubsystem.h"
#include "GameEventSubsystem.h"
#include "Details/Signature.h"
#include "DynamicEventContext.h"

static_assert(!UE::GlobalEvents::Details::TTypeInfo<UObject>::IsSupportedType(), "check");
static_assert(UE::GlobalEvents::Details::TTypeInfo<UObject*>::IsSupportedType(), "check");
static_assert(UE::GlobalEvents::Details::TTypeInfo<FVector>::IsSupportedType(), "check");
static_assert(!UE::GlobalEvents::Details::TTypeInfo<FVector*>::IsSupportedType(), "check");
static_assert(UE::GlobalEvents::Details::TTypeInfo<FVector&>::IsSupportedType(), "check");
static_assert(UE::GlobalEvents::Details::TTypeInfo<const FVector&>::IsSupportedType(), "check");
static_assert(UE::GlobalEvents::Details::TTypeInfo<int>::IsSupportedType(), "check");
static_assert(UE::GlobalEvents::Details::TTypeInfo<FString>::IsSupportedType(), "check");
static_assert(!UE::GlobalEvents::Details::TTypeInfo<FString*>::IsSupportedType(), "check");
static_assert(UE::GlobalEvents::Details::TTypeInfo<const FString&>::IsSupportedType(), "check");


DEFINE_TYPESAFE_GLOBAL_EVENT(ReferenceTestsEvent, bool, bool&, int, int&, FString, FString&, FVector, FVector&);
DEFINE_TYPESAFE_GLOBAL_EVENT(ArrayEvent, TArray<int>, const TArray<int>&, TArray<FString>, const TArray<FString>&, TArray<UObject*>, const TArray<UObject*>&);
DEFINE_TYPESAFE_GLOBAL_EVENT(SetMapEvent, 
    TSet<int>,
    const TSet<FName>&,
    TMap<int, FString>,
    const TMap<FName, FString>&,
    TMap<int, UObject*>,
    const TMap<FString, UObject*>&
);

void GlobalRefTests(bool bValue, bool& bRef, int iValue, int& iRef, FString sValue, FString& sRef, FVector vValue, FVector& vRef)
{
    UE_LOG(GlobalEventsLog, Warning, TEXT("%s"), FUNC_NAME);
}

void UGameEventTestsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    RawObjPtr = MakeShared<FRawTestsObject>();
    TestsObj = NewObject<UTestObject>();

    UGameEventSubsystem* EventCenter = UGameEventSubsystem::GetInstance(this);
    check(EventCenter != nullptr);

    // register global event process call back
    EventCenter->OnReceiveGlobalEvent.AddDynamic(this, &UGameEventTestsSubsystem::OnGlobalEventReceived);

    TestDynamicTuple();

    RegisterDebugEvent();

    SendDebugEvent();

    TestReferenceParameter();
}

void UGameEventTestsSubsystem::Deinitialize()
{
    UnRegisterDebugEvent();

    Super::Deinitialize();
}

void UGameEventTestsSubsystem::OnGlobalEventReceived(const FName& InName, UDynamicEventContext* InContext)
{
    UE_LOG(GlobalEventsLog, Log, TEXT("Received Event:%s Event Signature (Param Count=%d):%s"), *InName.ToString(), InContext->Num(), *InContext->GetParams().GetSignature()->ToString());
}

void UGameEventTestsSubsystem::TestDynamicTuple()
{
#define _CheckOffset(index) \
    const int32 TupleOffset##index = (uint8*)&Tuples.Get<index>() - (uint8*)&Tuples; \
    const int32 ContextOffset##index = Context->GetParams().GetOffset(index); \
    if(TupleOffset##index != ContextOffset##index) \
    { \
        UE_LOG(GlobalEventsLog, Warning, TEXT("Offset Missing match at %d, Tuple's %d != Context's %d, ElementSize=%d"), \
        index, \
        TupleOffset##index, \
        ContextOffset##index, \
        (int)sizeof(Tuples.Get<index>()) \
        ); \
        check(TupleOffset##index == ContextOffset##index); \
    } \
    else \
    { \
        UE_LOG(GlobalEventsLog, Warning, TEXT("Valid Offset of %d is %d, ElementSize=%d, EndOffset=%d"), \
        index, \
        TupleOffset##index, \
        (int)sizeof(Tuples.Get<index>()), \
        ContextOffset##index + (int)sizeof(Tuples.Get<index>()) \
        ); \
    }

#define _CheckTupleOffset(index, offset) \
    if((uint8*)&Tuples.Get<index>() - (uint8*)&Tuples != offset) \
    { \
        UE_LOG(GlobalEventsLog, Warning, TEXT("Offset Missing match at %d, Tuple's %d != %d"), index, (int)((uint8*)&Tuples.Get<index>() - (uint8*)&Tuples), offset); \
        check((uint8*)&Tuples.Get<index>() - (uint8*)&Tuples == offset); \
    } \
    else \
    { \
        UE_LOG(GlobalEventsLog, Warning, TEXT("Offset match at %d, Tuple's %d, ElementSize=%d"), index, (int)((uint8*)&Tuples.Get<index>() - (uint8*)&Tuples), (int)sizeof(Tuples.Get<index>())); \
    }

    {
        TTuple<bool, bool, uint8, uint8> Tuples;
        _CheckTupleOffset(0, 0);
        _CheckTupleOffset(1, 1);
        _CheckTupleOffset(2, 2);
        _CheckTupleOffset(3, 3);
    }

    {
        TTuple<bool, bool, uint8, int> Tuples;
        _CheckTupleOffset(0, 0);
        _CheckTupleOffset(1, 1);
        _CheckTupleOffset(2, 2);
        _CheckTupleOffset(3, 4);
    }

    {
        TTuple<bool, bool, int, int> Tuples;
        _CheckTupleOffset(0, 0);
        _CheckTupleOffset(1, 1);
        _CheckTupleOffset(2, 4);
        _CheckTupleOffset(3, 8);
    }

    {
        TTuple<bool, bool, int, double> Tuples;
        _CheckTupleOffset(0, 0);
        _CheckTupleOffset(1, 1);
        _CheckTupleOffset(2, 4);
        _CheckTupleOffset(3, 8);
    }

    {
        TTuple<bool, bool, int, double, bool, bool, int> Tuples;
        _CheckTupleOffset(0, 0);
        _CheckTupleOffset(1, 1);
        _CheckTupleOffset(2, 4);
        _CheckTupleOffset(3, 8);
        _CheckTupleOffset(4, 16);
        _CheckTupleOffset(5, 17);
        _CheckTupleOffset(6, 20);
    }

    {
        TTuple<bool, bool, double, int> Tuples;
        _CheckTupleOffset(0, 0);
        _CheckTupleOffset(1, 1);
        _CheckTupleOffset(2, 8);
        _CheckTupleOffset(3, 16);
    }

   // static_assert(sizeof(FString) == 16, "Error");
   // static_assert(sizeof(FName) == 12, "Error");

    {
        TTuple<bool, bool, FString, int> Tuples;

        _CheckTupleOffset(0, 0);
        _CheckTupleOffset(1, 1);
        _CheckTupleOffset(2, 8);
        _CheckTupleOffset(3, 16 + 8);
    }

    {
        TTuple<bool, bool, FString, FName> Tuples;

        _CheckTupleOffset(0, 0);
        _CheckTupleOffset(1, 1);
        _CheckTupleOffset(2, 8);
        _CheckTupleOffset(3, 24);
    }

    {
        TTuple<bool, uint8, int32, int64, int32, FString, FString, FName, FName, FVector, FVector, UObject*, TArray<int>, TSet<FString>, TMap<int, FName>> Tuples;
        Tuples.Get<0>() = false;
        Tuples.Get<1>() = 127;
        Tuples.Get<2>() = 300;
        Tuples.Get<3>() = 1024;
        Tuples.Get<4>() = 4096;
        Tuples.Get<5>() = TEXT("StringValue_2345");
        Tuples.Get<6>() = TEXT("StringRef_2345");
        Tuples.Get<7>() = TEXT("NameValue2");
        Tuples.Get<8>() = TEXT("NameRef2");
        Tuples.Get<9>() = FVector(7, 8, 9);
        Tuples.Get<10>() = FVector(1234, 5678, 9012);
        Tuples.Get<11>() = this;
        Tuples.Get<12>() = { 1,2,3 };
        Tuples.Get<13>() = { TEXT("hello"), TEXT("world") };
        Tuples.Get<14>() = { {100, TEXT("100")} };

        UDynamicEventContext* Context = NewObject<UDynamicEventContext>();
        Context->Add(Tuples.Get<0>());
        Context->Add(Tuples.Get<1>());
        Context->Add(Tuples.Get<2>());
        Context->Add(Tuples.Get<3>());
        Context->Add(Tuples.Get<4>());
        Context->Add(Tuples.Get<5>());
        Context->Add(Tuples.Get<6>());
        Context->Add(Tuples.Get<7>());
        Context->Add(Tuples.Get<8>());
        Context->Add(Tuples.Get<9>());
        Context->Add(Tuples.Get<10>());
        Context->Add(Tuples.Get<11>());
        Context->Add(Tuples.Get<12>());
        Context->Add(Tuples.Get<13>());
        Context->Add(Tuples.Get<14>());

        _CheckOffset(0);
        _CheckOffset(1);
        _CheckOffset(2);
        _CheckOffset(3);
        _CheckOffset(4);
        _CheckOffset(5);
        _CheckOffset(6);
        _CheckOffset(7);
        _CheckOffset(8);
        _CheckOffset(9);
        _CheckOffset(10);
        _CheckOffset(11);
        _CheckOffset(12);
        _CheckOffset(13);
        _CheckOffset(14);
    }
    
}

void UGameEventTestsSubsystem::RegisterDebugEvent()
{
    UGameEventSubsystem* EventCenter = UGameEventSubsystem::GetInstance(this);
    check(EventCenter != nullptr);

    // [dynamic mode]register a global function 
    EventCenter->Register(FDebugEvent::GetEventName(), TestGlobalCFunctionObserver);

    // [dynamic mode]register a class static member function
    EventCenter->Register(FDebugEvent::GetEventName(), &UTestObject::TestUObjectStaticMemberFunctionObserver);

    // [dynamic mode]register a UObject member function in C++ mode
    EventCenter->Register(FDebugEvent::GetEventName(), TestsObj, &UTestObject::TestUObjectMemberFunctionObserver);

    // [dynamic mode]register a UObject UFunction in script mode(same as dynamic delegate)
    EventCenter->Register(FDebugEvent::GetEventName(), TestsObj, GET_FUNCTION_NAME_CHECKED(UTestObject, TestUObjectUFunctionObserver));

    // [static mode]register a global function 
    EventCenter->Register<FDebugEvent>(TestGlobalCFunctionObserver);

    // [static mode]register a class static member function
    EventCenter->Register<FDebugEvent>(&UTestObject::TestUObjectStaticMemberFunctionObserver);

    // [static mode]register a normal class's member function on raw pointer
    EventCenter->Register<FDebugEvent>(&RawObj, &FRawTestsObject::TestRawMemberFunctionObserver);

    // [static mode]register a normal class's member function on TSharedPtr
    EventCenter->Register<FDebugEvent>(RawObjPtr, &FRawTestsObject::TestRawMemberFunctionObserver);
}

void UGameEventTestsSubsystem::UnRegisterDebugEvent()
{
    UGameEventSubsystem* EventCenter = UGameEventSubsystem::GetInstance(this);
    check(EventCenter != nullptr);

    const FName EventName = FDebugEvent::GetEventName();

    EventCenter->UnRegister<FDebugEvent>(&RawObj, &FRawTestsObject::TestRawMemberFunctionObserver);
    EventCenter->UnRegister<FDebugEvent>(&UTestObject::TestUObjectStaticMemberFunctionObserver);
    EventCenter->UnRegister<FDebugEvent>(RawObjPtr, &FRawTestsObject::TestRawMemberFunctionObserver);

    EventCenter->UnRegister(FDebugEvent::GetEventName(), TestGlobalCFunctionObserver);
    EventCenter->UnRegister(FDebugEvent::GetEventName(), &UTestObject::TestUObjectStaticMemberFunctionObserver);
    EventCenter->UnRegister(FDebugEvent::GetEventName(), TestsObj, &UTestObject::TestUObjectMemberFunctionObserver);
    EventCenter->UnRegister(FDebugEvent::GetEventName(), TestsObj, GET_FUNCTION_NAME_CHECKED(UTestObject, TestUObjectUFunctionObserver));
}

void UGameEventTestsSubsystem::SendDebugEvent()
{
    UGameEventSubsystem* EventCenter = UGameEventSubsystem::GetInstance(this);
    check(EventCenter != nullptr);

    bool bValue = false;
    uint8 u8Value = 127;
    int32 i32Value = 1024;
    int64 i64Value = 1024 * 1024 * 1024;
    EGlobalEventParameterType enumValue = EGlobalEventParameterType::GEPT_Boolean;
    FString StrValue = TEXT("StrValue");    
    FString StrRef = TEXT("StrRef");
    FString StrConstRef = TEXT("StrConstRef");
    FName NameValue = TEXT("NameValue");
    FName NameRef = TEXT("NameRef");
    FVector VecValue = FVector(1, 2, 3);
    FVector VecRef = FVector(100, 200, 300);
    FVector VecConstRef = FVector(1000, 2000, 3000);
    UObject* ObjectValue = this;
    TArray<int> IntArrayValue{ 1,2,3,4 };
    TArray<int> IntArrayRef{ 100, 200, 300, 400, 500 };
    TArray<FString> StringArrayValue{ TEXT("Hello"), TEXT("World"), TEXT("!") };
    TArray<FString> StringArrayRef{ TEXT("Hello"), TEXT("Unreal"), TEXT("!!") };
    TArray<UObject*> ObjectArrayValue{ EventCenter, this };
    TArray<UObject*> ObjectArrayRef{ TestsObj, EventCenter, this };
    TSet<int> IntSetValue{ 1,2,3 };
    TSet<FName> NameSetValue{ TEXT("Hi"), TEXT("NiHao") };
    TMap<FName, FString> NameStringMapValue{ {TEXT("Hi"), TEXT("Bodong")} };
    TMap<int, FString> IntStringMapRef{ {100, TEXT("100")}, {200, TEXT("200")} };
    TMap<int, UObject*> IntObjectMap{ {100, this}, {200, EventCenter} };
    TMap<FString, UObject*> StringObjectMap{ {TEXT("This"), this}, {TEXT("EventCenter"), EventCenter} };

    // Parameter mismatch will not cause a compilation error, but an error message will be output at run time.
    // Template parameters are optional, but most of the time, in order to ensure that the parameter type is correctly inferred, 
    // template parameters should be provided. 
    // For example, C++ template inference cannot infer const TCHAR* to FString.
    EventCenter->Broadcast<__TestParamsType>(
            FDebugEvent::GetEventName(),
            bValue,
            u8Value,
            i32Value,
            i64Value,
            enumValue,
            StrValue,
            StrRef,
            StrConstRef,
            NameValue,
            NameRef,
            VecValue,
            VecRef,
            VecConstRef,
            ObjectValue,
            IntArrayValue,
            IntArrayRef,
            StringArrayValue,
            StringArrayRef,
            ObjectArrayValue,
            ObjectArrayRef,
            IntSetValue,
            NameSetValue,
            NameStringMapValue,
            IntStringMapRef,
            IntObjectMap,
            StringObjectMap
        );

    check(StrRef == TEXT("StrRef"));
    check(VecRef == FVector(100, 200, 300));

    // register by common interface
    //     auto lambda = [](__TestParams) {
    //         strRef = TEXT("StrRef2");
    //         vecRef = FVector(7, 8, 9);
    //         };
    // FDelegateHandle Handle = EventCenter->Register<decltype(lambda), __TestParamsType>(FDebugEvent::GetEventName(), MoveTemp(lambda));

    // register by typesafe interface
    FDelegateHandle Handle = EventCenter->Register<FDebugEvent>([](__TestParams) {
        strRef = TEXT("StrRef2");
        vecRef = FVector(7, 8, 9);
        });

    // If the parameter type does not match the event definition, a compilation error will be triggered
    EventCenter->Broadcast<FDebugEvent>(
        bValue,
        u8Value,
        i32Value,
        i64Value,
        enumValue,
        StrValue,
        StrRef,
        StrConstRef,
        NameValue,
        NameRef,
        VecValue,
        VecRef,
        VecConstRef,
        ObjectValue,
        IntArrayValue,
        IntArrayRef,
        StringArrayValue,
        StringArrayRef,
        ObjectArrayValue,
        ObjectArrayRef,
        IntSetValue,
        NameSetValue,
        NameStringMapValue,
        IntStringMapRef,
        IntObjectMap,
        StringObjectMap
    );

    check(StrRef != TEXT("StrRef"));
    check(VecRef != FVector(100, 200, 300));

    EventCenter->UnRegister(FDebugEvent::GetEventName(), Handle);

}

DEFINE_TYPESAFE_GLOBAL_EVENT(TestReferenceEvent, bool, bool&, FString, FString&, FVector, FVector&);

void UGameEventTestsSubsystem::TestReferenceParameter()
{
    bool bValue = false, bRef = false;
    FString strValue = TEXT("strValue"), strRef = TEXT("strRef");
    FVector vecValue = FVector(1, 2, 3), vecRef = FVector(7, 8, 9);

    struct FLocalReferenceTest
    {
        void DoSth(bool bv, bool& bref, FString sv, FString& sr, FVector vv, FVector& vr)
        {
            bref = true;
            sr = TEXT("strRef2");
            vr = FVector(100, 200, 300);
        }
    };

    UGameEventSubsystem* EventCenter = UGameEventSubsystem::GetInstance(this);
    check(EventCenter != nullptr);

    FLocalReferenceTest LocalTestObj;
    EventCenter->Register<FTestReferenceEvent>(&LocalTestObj, &FLocalReferenceTest::DoSth);

    EventCenter->Broadcast<FTestReferenceEvent>(bValue, bRef, strValue, strRef, vecValue, vecRef);
    check(bRef);
    check(strRef == TEXT("strRef2"));
    check(vecRef == FVector(100, 200, 300));

    EventCenter->UnRegister<FTestReferenceEvent>(&LocalTestObj, &FLocalReferenceTest::DoSth);

    EventCenter->Register<FTestReferenceEvent>(TestsObj, GET_FUNCTION_NAME_CHECKED(UTestObject, TestUFunctionWithReference));

    EventCenter->Broadcast<FTestReferenceEvent>(bValue, bRef, strValue, strRef, vecValue, vecRef);

    check(!bRef);
    check(strRef == GET_FUNCTION_NAME_CHECKED(UTestObject, TestUFunctionWithReference).ToString());
    check(vecRef == FVector(1024, 2048, 4096));

    EventCenter->ClearEventObservers<FTestReferenceEvent>();
}
