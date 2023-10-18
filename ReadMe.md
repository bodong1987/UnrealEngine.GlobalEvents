# Global Events Plugins for UnrealEngine

**Your bug reports and improvements are very welcome, you can submit them through the issues page. Of course, you can also fix it yourself and submit a Pull Request.**

## Introduction
This is a plugin for Unreal Engine. Its main purpose is to implement a simple message sending system in Unreal to solve the strong coupling problem caused by using function calls, delegation and other means.Through this plug-in, you can register a message listener (observer) with the message service, and you can also dispatch messages (publisher) through the message service.  

This plugin supports the following capabilities:
* Register event callbacks in Blueprints, C++ or other scripts.
* Send events in Blueprint, C++ or other scripting languages.
* The C++ level has two registration modes: static and dynamic. The static mode will check whether the message sending and message receiving codes match during compilation.
* Others are in dynamic mode. If the signatures between the sender and the recipient are incompatible, an error log will be printed and the message delivery will fail.
* Provides corresponding blueprint nodes for dispatching events and receiving event callbacks
* Need Unreal Engine 4.25 or newer

## How To Use
First of all, your engine should be 4.25 or newer. I have successfully tested it under Windows and MacOS.  
Then copy the Plugins/GlobalEvents directory under this project to the Plugins directory of your project. If you want to run test code, you can also copy the GlobalEventsTests directory together.  
After successful compilation, you can use and test it.

### Register message callback
To register a callback in C++, you simply call UGameEventSubsystem::Register.This interface supports binding the following objects to an event. These targets are similar to UnrealEngine's Delegate:  
* Global Function
* Class's Static Member Function
* RawPointer + Class's Member Function
* TSharedPtr + Class's Member Function
* UObject* + Class's Member Function
* UObject* + UFunction's Name
* Lambda Expression  

***It should be noted that, in order to be compatible with blueprints, not all C++ types can be used as event parameters. In theory, only parameters supported by UFUNCTION can be supported.***

Each Register interface will return an FDelegateHandle as a handle to represent the callback you registered.  
For details, please refer to the sample files in the project:  
```C++
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
```
To register an event in a blueprint, use the Register Global Event node. You need to provide an event name, target object and a function name as parameters:  
![RegisterNode](./Docs/Images/RegisterNode.png)

**Please note: the signature of an event is determined by the callback function that first registered it by default. Of course, if you want to force a message signature, you can do it through the interface: UGameEventSubsystem::BindSignature.**  

### Unregister message callback
In C++, you only need to change the interface from Register to UnRegister, and the rest remains unchanged. like this:  

```C++
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

```
Most registered message callbacks support un-registration as they are, which can reduce the cost of understanding. You also don’t need to save FDelegateHandle for un-registration. Of course, the only exception is when using a Lambda expression as a callback. At this time, you can only record the FDelegateHandle and unregister it when it is no longer needed, such as:  
```C++
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

    // do something

    // unregister by handle
    EventCenter->UnRegister(FDebugEvent::GetEventName(), Handle);
```

Of course, using Handle to deregister is also supported in other situations.  
In the blueprint, you only need to use the UnRegister Global Event to unregister, and its parameters are the same as when registering.  
![UnRegister](./Docs/Images/RegisterNode.png)

### Dispatch Event(Broadcast Event)  
To send events in C++, you need to use the Broadcast, BroadcastDynamic and other interfaces of UGameEventSubsystem.   
For Broadcast, there are two interfaces, one is static type safety; the other is dynamic type safety:
``` C++
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

    // Parameter mismatch will not cause a compilation error, but an error message will be output at run time.
    // Template parameters are optional, but most of the time, in order to ensure that the parameter type is correctly inferred, template parameters should be provided. For example, C++ template inference cannot infer const TCHAR* to FString.
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
```
**The so-called static security is just that I provide a macro to define an event. When this event is defined, it forcibly constrains an event and the parameter list of this event, so that the parameters do not match when you register, unregister, or dispatch an event ( number or type), it will cause a compilation error. you can use this macro to define an static event: DEFINE_TYPESAFE_GLOBAL_EVENT**  

```C++
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
    
DEFINE_TYPESAFE_GLOBAL_EVENT(DebugEvent, __TestParamsType);
```

To dispatch an event in a blueprint, you need to use the Broadcast Global Event node, and you need to use the "Add Argument" button on the node to add the correct parameters. You need to ensure that the number and type of parameters match the number of parameters required for the target message. The type matches, otherwise the message you dispatch will not be delivered correctly, and the previously registered callback function will not be triggered.  
![Broadcast](./Docs/Images/BroadcastEvent.png)   
The blueprint sends messages by calling the BroadcastDynamic interface, which is also the method used to send events in other scripting languages.  


## FAQ   
1. Why are versions before 4.25 not supported?   
Because I used FProperty related code and I didn’t want to think about UProperty compatibility anymore, then I asked ChatGPT:  
Question: In which version of Unreal did FProperty replace UProperty, and from which version did it stop supporting 32-bit?
Answer: In Unreal Engine 4, FProperty replaced UProperty starting from version 4.25. This change was made to improve the Unreal Engine's metadata system and code generator. From version 4.25 onwards, you should use FProperty instead of UProperty.
Additionally, starting from Unreal Engine 4.25, official support for 32-bit operating systems was discontinued. Unreal Engine 4.25 and later versions only support 64-bit operating systems. If you need to run Unreal Engine on a 32-bit operating system, you can try using version 4.24 or earlier. However, please note that older versions may lack some new features and performance optimizations, and they may no longer receive official support and updates. For the best performance and features, it is recommended that you use the latest version of Unreal Engine on a 64-bit operating system.

**Of course, if you want the code to be compatible with older engine versions and have improved the relevant code, I will be very happy to see your Pull Request.**  
  
2. How to monitor these events in a script, such as Type Script?  
UGameEventSubsystem has a public global callback OnReceiveGlobalEvent. This callback is an unreal dynamic delegate, so most scripts should be able to access this delegate. You only need to register this delegate, and then this delegate will be triggered when a global message is received. The parameters of this delegate are an event name and a UDynamicEventContext object. The message name can be obtained through the former, and the parameter type and parameter data can be obtained through the latter.   
```C++
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGlobalEventDelegateType, const FName&, EventName, UDynamicEventContext*, Context);

public:
	UPROPERTY(BlueprintAssignable)
	FGlobalEventDelegateType           OnReceiveGlobalEvent;
```  
This way you can trigger related code further in the script engine. To send a message, use UDynamicEventFunctionLibrary::BroadcastEvent. You need to construct the UDynamicEventContext object yourself and use it as a parameter.  
```C++
UFUNCTION(BlueprintCallable, Category = "Global Events", meta=(BlueprintInternalUseOnly = "true"))
static bool BroadcastEvent(FName EventName, UDynamicEventContext* Context);
```  
Of course, you can also bind a C++ function of your own to your script and use it to send messages.  
As for registering message callbacks inside the script, you can maintain a dictionary inside the script yourself, and when the relevant message is triggered, extract the parameters in the UDynamicEventContext object to call the corresponding script function.  


3. Can this system be used in other classes?  
Yes. By looking at the source code, you can find that the interfaces for registration, deregistration, and message dispatch are actually added to UGameEventSubSystem through the include method. Of course, you can add them to your own class through the same method.  
```C++
UCLASS()
class GLOBALEVENTS_API UGameEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;

	static UGameEventSubsystem* GetInstance(const UObject* InContext);

#define ENABLE_EVENT_CENTER_ON_RECEIVE_GLOBAL_EVENT

public:
	UPROPERTY(BlueprintAssignable)
	FGlobalEventDelegateType           OnReceiveGlobalEvent;

public:
#if CPP
#include "Inline/EventCenterDataInterfacesInline.inl"
#include "Inline/EventCenterCommonInterfacesInline.inl"
#include "Inline/EventCenterTypeSafeInterfacesInline.inl"
#include "Inline/EventCenterDynamicInterfacesInline.inl"
#include "Inline/EventCenterSignatureInterfacesInline.inl"
#endif

#undef ENABLE_EVENT_CENTER_ON_RECEIVE_GLOBAL_EVENT
};
```
If you don't need some interfaces, you can just not include the corresponding inl file. However, EventCenterDataInterfacesInline.inl is necessary.  




