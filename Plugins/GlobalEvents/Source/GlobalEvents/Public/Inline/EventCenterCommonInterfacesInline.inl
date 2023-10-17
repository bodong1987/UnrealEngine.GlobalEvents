/*
	MIT License

	Copyright (c) 2023 GlobalEvents Plugin For UnrealEngine

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

	Project URL: https://github.com/bodong1987/UnrealEngine.GlobalEvents
*/
/* This is template include file, don't include it directly. */
#if !CPP
#error "don't include this file directly."
#endif

private:
	template <typename SignatureType, bool bAddNewIfNotExists, bool bIgnoreCheck = false>
	inline SignatureType* QuerySignalImpl(const FName& InEventName, const UE::GlobalEvents::ISignature* ObserverSignature)
	{
		auto* Ptr = EventMaps.Find(InEventName);

		if (Ptr != nullptr)
		{
			if constexpr (!bIgnoreCheck)
			{
				checkSlow(ObserverSignature != nullptr);

				// The signature of the delegate to be added must allow conversion from the signature of an existing signal
				if (!ObserverSignature->CheckInvokeableFrom((*Ptr)->GetSignature()))
				{
					UE_LOG(GlobalEventsLog,
						Error,
						TEXT("Invalid Operation, failed convert signature. EventName = (%s), Signal Signature = (%s), Observer Signature = (%s)"),
						*InEventName.ToString(),
						*((*Ptr)->GetSignature()->ToString()),
						*(ObserverSignature->ToString())
					);

					return nullptr;
				}
			}

			return (SignatureType*)(*Ptr).Get();
		}

		if constexpr (bAddNewIfNotExists)
		{
			return (SignatureType*)(EventMaps.Emplace(InEventName, MakeShared<SignatureType>()).Get());
		}
		else
		{
			return nullptr;
		}
	}

	template <typename SignatureType>
	inline FDelegateHandle RegisterImpl(const FName& InEventName, UE::GlobalEvents::Details::IEventObserver* InInstance)
	{
		auto* Signal = QuerySignalImpl<SignatureType, true>(InEventName, SignatureType::StaticSignature());
		
		return Signal != nullptr ? Signal->Connect(InInstance) : FDelegateHandle();
	}

	template <typename SignatureType>
	inline bool UnRegisterImpl(const FName& InEventName, UE::GlobalEvents::Details::IEventObserver* InInstance)
	{
		auto* Signal = QuerySignalImpl<SignatureType, false, true>(InEventName, SignatureType::StaticSignature());

		return Signal != nullptr && Signal->Disconnect(InInstance);
	}

public:
	// register for global function or class's static function
	template <typename... ParamTypes>
	inline FDelegateHandle Register(const FName& InEventName, void(*InFunc)(ParamTypes...))
	{
		UE::GlobalEvents::Details::TCommonEventObserver<ParamTypes...> Observer(InFunc);

		return RegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	template <typename... ParamTypes>
	inline bool UnRegister(const FName& InEventName, void(*InFunc)(ParamTypes...))
	{
		UE::GlobalEvents::Details::TCommonEventObserver<ParamTypes...> Observer(InFunc);

		return UnRegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	// for raw pointer listener
	// You must UnRegister yourself, otherwise it will cause a crash when the pointer is invalidated.
	template <typename UserClass, typename... ParamTypes, typename TEnableIf<!TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline FDelegateHandle Register(const FName& InEventName, UserClass* InTarget, void (UserClass::* InFunc)(ParamTypes...))
	{
		static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You can't use UObject method in this method.");
		checkSlow(InTarget);

		UE::GlobalEvents::Details::TMemberFunctionEventObserver<UserClass, ParamTypes...> Observer(InTarget, InFunc);

		return RegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	template <typename UserClass, typename... ParamTypes, typename TEnableIf<!TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline bool UnRegister(const FName& InEventName, UserClass* InTarget, void (UserClass::* InFunc)(ParamTypes...))
	{
		static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You cannot use UObject method in this method.");
		checkSlow(InTarget);

		UE::GlobalEvents::Details::TMemberFunctionEventObserver<UserClass, ParamTypes...> Observer(InTarget, InFunc);

		return UnRegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	// for UObject* pointer
	// it will save pointer by TWeakObjectPtr
	template <typename UserClass, typename... ParamTypes, typename TEnableIf<TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline FDelegateHandle Register(const FName& InEventName, UserClass* InTarget, void (UserClass::* InFunc)(ParamTypes...))
	{
		static_assert(UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You can only use UObject method in this method.");
		checkSlow(InTarget);

		UE::GlobalEvents::Details::TBaseUObjectMemberFunctionEventObserver<UserClass, ParamTypes...> Observer(InTarget, InFunc);

		return RegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	template <typename UserClass, typename... ParamTypes, typename TEnableIf<TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline bool UnRegister(const FName& InEventName, UserClass* InTarget, void (UserClass::* InFunc)(ParamTypes...))
	{
		static_assert(UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You can only use UObject method in this method.");
		checkSlow(InTarget);

		UE::GlobalEvents::Details::TBaseUObjectMemberFunctionEventObserver<UserClass, ParamTypes...> Observer(InTarget, InFunc);

		return UnRegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	// for TSharedPtr
	// it will save pointer by TWeakPtr
	template <typename UserClass, ESPMode Mode, typename... ParamTypes, typename TEnableIf<!TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline FDelegateHandle Register(const FName& InEventName, const TSharedPtr<UserClass, Mode>& InTarget, void (UserClass::* InFunc)(ParamTypes...))
	{
		static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You cannot use UObject method in this method.");
		checkSlow(InTarget);

		UE::GlobalEvents::Details::TSPMemberFunctionEventObserver<UserClass, Mode, ParamTypes...> Observer(InTarget, InFunc);
		return RegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	template <typename UserClass, ESPMode Mode, typename... ParamTypes, typename TEnableIf<!TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline bool UnRegisterUnsafe(const FName& InEventName, const TSharedPtr<UserClass, Mode>& InTarget, void (UserClass::* InFunc)(ParamTypes...))
	{
		static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You cannot use UObject method in this method.");
		checkSlow(InTarget);

		UE::GlobalEvents::Details::TSPMemberFunctionEventObserver<UserClass, Mode, ParamTypes...> Observer(InTarget, InFunc);
		return UnRegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	// for lambda expression
	// It does not have a corresponding UnRegister function, you must use Handle to : 
	//		inline bool UnRegister(const FName& InEventName, FDelegateHandle InHandle)
	template <typename FunctorType, typename... ParamTypes, typename TEnableIf<TIsClass<FunctorType>::Value, int>::Type = 0>
	inline FDelegateHandle Register(const FName& InEventName, FunctorType&& InFunctor)
	{
		typename UE::GlobalEvents::Details::TFunctorEventObserver<FunctorType, ParamTypes...> Observer(MoveTemp(InFunctor));

		return RegisterImpl<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InEventName, &Observer);
	}

	// for Unreal UFunction
	template <typename UserClass, typename TEnableIf<TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline FDelegateHandle Register(const FName& InEventName, UserClass* InTarget, const FName& InFunctionName)
	{
		static_assert(UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You can only use UObject method in this method.");
		checkSlow(InTarget);

		// create a new signal
		UFunction* Function = InTarget->FindFunctionChecked(InFunctionName);

		if (Function == nullptr)
		{
			UE_LOG(GlobalEventsLog,
				Error,
				TEXT("Failed find function (%s) on object (%s) when register event (%s)"),
				*InFunctionName.ToString(),
				*GetNameSafe(InTarget),
				*InEventName.ToString()
			);

			return FDelegateHandle();
		}

		UE::GlobalEvents::Details::FUFunctionSignature ObserverSignature(Function);

		if (!ObserverSignature.IsValid())
		{
			UE_LOG(GlobalEventsLog,
				Error,
				TEXT("Failed find function (%s) on object (%s) when register event (%s), target function is not acceptable[no return value, no invalid parameter type]."),
				*InFunctionName.ToString(),
				*GetNameSafe(InTarget),
				*InEventName.ToString()
			);

			return FDelegateHandle();
		}

		UE::GlobalEvents::Details::TUFunctionEventObserver<UserClass> Observer(InTarget, Function);

		UE::GlobalEvents::ISignal* Signal = QuerySignalImpl<UE::GlobalEvents::Details::FUFunctionSignal, false, true>(InEventName, &ObserverSignature);

		if (Signal != nullptr)
		{
			if (!ObserverSignature.CheckInvokeableFrom(Signal->GetSignature()))
			{
				UE_LOG(GlobalEventsLog,
					Error,
					TEXT("Invalid Operation, failed convert signature. EventName = (%s), Signal Signature = (%s), Observer Signature = (%s)"),
					*InEventName.ToString(),
					*(Signal->GetSignature()->ToString()),
					*(ObserverSignature.ToString())
				);

				return FDelegateHandle();
			}
		}
		else
		{
			Signal = EventMaps.Emplace(InEventName, MakeShared<UE::GlobalEvents::Details::FUFunctionSignal>(Function)).Get();
		}

		return Signal != nullptr ? Signal->Connect(&Observer) : FDelegateHandle();
	}

	template <typename UserClass, typename TEnableIf<TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline bool UnRegister(const FName& InEventName, UserClass* InTarget, const FName& InFunctionName)
	{
		static_assert(UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You can only use UObject method in this method.");
		checkSlow(InTarget);

		UE::GlobalEvents::Details::TUFunctionEventObserver<UserClass> Observer(InTarget, InFunctionName);

		auto* Signal = QuerySignalImpl<UE::GlobalEvents::Details::FUFunctionSignal, false, true>(InEventName, nullptr);
		
		return Signal != nullptr && Signal->Disconnect(&Observer);
	}

	// Unregister by handle
	inline bool UnRegister(const FName& InEventName, FDelegateHandle InHandle)
	{
		auto* Ptr = EventMaps.Find(InEventName);

		return Ptr != nullptr && (*Ptr)->Disconnect(InHandle);
	}

private:



public:
	/*
	* Send an event and use template parameters to automatically infer the event signature. 
	* If the signature does not strictly match, the event will fail to be sent. 
	* You can find this error through the log.
	*/
	template <typename... ParamTypes>
	inline bool Broadcast(const FName& InEventName, ParamTypes... InParams)
	{
		static_assert(UE::GlobalEvents::Details::TIsSupportedTypes<ParamTypes...>::Value, "Don't use unsupported type");

		auto* Ptr = EventMaps.Find(InEventName);

		if (Ptr != nullptr)
		{
			if (!UE::GlobalEvents::Details::TSignalInvoker<ParamTypes...>::Invoke(*Ptr, InEventName, InParams...))
			{
				return false;
			}
					
#ifdef ENABLE_EVENT_CENTER_ON_RECEIVE_GLOBAL_EVENT
            /*
            * To enable this capability, you need to add the following delegate definition to the host class:
            * public:
	              UPROPERTY(BlueprintAssignable)
	              FGlobalEventDelegateType           OnReceiveGlobalEvent;
            * Then you need to define this macro: 
                  ENABLE_EVENT_CENTER_ON_RECEIVE_GLOBAL_EVENT
            */
			if (OnReceiveGlobalEvent.IsBound())
			{
				UDynamicEventContext* Context = TDynamicEventContextFactory<ParamTypes...>::NewContext(InParams...);

				OnReceiveGlobalEvent.Broadcast(InEventName, Context);
			}
#endif

			return true;
		}

		return false;
	}

	// shutdown this service
	inline void Shutdown()
	{
		// release all delegates
		for (auto Pair : EventMaps)
		{
			Pair.Value->DisconnectAll();
		}

		// Broadcast will keep Signal instance reference
		// so clear this map is safe
		EventMaps.Empty();
	}

	// Clear all observers for an event
	inline void ClearEventObservers(const FName& InEventName)
	{
		auto* Ptr = EventMaps.Find(InEventName);

		if (Ptr == nullptr)
		{
			return;
		}

		// clear all 
		// if it is locked, it only set pending destroy flags.
		(*Ptr)->DisconnectAll();

		// remove container
		EventMaps.Remove(InEventName);
	}
