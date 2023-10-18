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
	template <typename EventType, bool bAddNewIfNotExists = true, bool bIgnoreCheck = false>
	inline typename EventType::FSignalType* QueryTypedSignalImpl()
	{
		auto* Ptr = EventMaps.Find(EventType::GetEventName());

		if (Ptr != nullptr)
		{
			if constexpr (!bIgnoreCheck)
			{
				if (!EventType::StaticSignature()->CheckInvokeableFrom((*Ptr)->GetSignature()))
				{
					UE_LOG(GlobalEventsLog, Error,						
						TEXT("Invalid Operation, failed convert signature. EventName = (%s), Signal Signature = (%s), Observer Signature = (%s)"),
						*EventType::GetEventName().ToString(),
						*((*Ptr)->GetSignature()->ToString()),
						*EventType::StaticSignature()->ToString()
					);

					return nullptr;
				}
			}

			return (typename EventType::FSignalType*)(*Ptr).Get();
		}

		if constexpr (bAddNewIfNotExists)
		{
			return (typename EventType::FSignalType*)(EventMaps.Emplace(EventType::GetEventName(), MakeShared<typename EventType::FSignalType>()).Get());
		}
		else
		{
			return nullptr;
		}
	}

	template <typename EventType>
	inline FDelegateHandle RegisterImpl(UE::GlobalEvents::Details::IEventObserver* InInstance)
	{
		auto* Signal = QueryTypedSignalImpl<EventType, true>();
		check(Signal);

		return Signal->Connect(InInstance);
	}

	template <typename EventType>
	inline bool UnRegisterImpl(UE::GlobalEvents::Details::IEventObserver* InInstance)
	{
		auto* Signal = QueryTypedSignalImpl<EventType, false>();

		return Signal != nullptr && Signal->Disconnect(InInstance);
	}

public:
	/*
	* Register A common Function
	*/
	template <typename EventType>
	inline FDelegateHandle Register(typename EventType::FCommonEventObserverType::FunctionType InFunc)
	{
		typename EventType::FCommonEventObserverType Observer(InFunc);

		return RegisterImpl<EventType>(&Observer);
	}

	template <typename EventType>
	inline bool UnRegister(typename EventType::FCommonEventObserverType::FunctionType InFunc)
	{
		typename EventType::FCommonEventObserverType Observer(InFunc);

		return UnRegisterImpl<EventType>(&Observer);
	}

	/*
	* Register A member function for common class/object
	*/
	template <typename EventType, typename UserClass, typename TEnableIf<!TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline FDelegateHandle Register(
		UserClass* InTarget,
		typename EventType::template TMemberFunctionEventObserverType<UserClass>::MemberFunctionType InFunc
	)
	{
		static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You cannot use UObject method in this method.");
		checkSlow(InTarget);

		typename EventType::template TMemberFunctionEventObserverType<UserClass> Observer(InTarget, InFunc);
		return RegisterImpl<EventType>(&Observer);
	}

	template <typename EventType, typename UserClass, typename TEnableIf<!TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline bool UnRegister(
		UserClass* InTarget,
		typename EventType::template TMemberFunctionEventObserverType<UserClass>::MemberFunctionType InFunc
	)
	{
		static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You cannot use UObject method in this method.");

		checkSlow(InTarget);

		typename EventType::template TMemberFunctionEventObserverType<UserClass> Observer(InTarget, InFunc);

		return UnRegisterImpl<EventType>(&Observer);
	}

	/*
	* Register A member function for common class with SharedPtr
	*/
	template <typename EventType, typename UserClass, ESPMode Mode, typename TEnableIf<!TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline FDelegateHandle Register(
		const TSharedPtr<UserClass, Mode>& InTarget,
		typename EventType::template TSPMemberFunctionEventObserverType<UserClass, Mode>::MemberFunctionType InFunc
	)
	{
		static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You cannot use UObject method in this method.");
		checkSlow(InTarget);

		typename EventType::template TSPMemberFunctionEventObserverType<UserClass, Mode> Observer(InTarget, InFunc);
		return RegisterImpl<EventType>(&Observer);
	}

	template <typename EventType, typename UserClass, ESPMode Mode, typename TEnableIf<!TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline bool UnRegister(
		const TSharedPtr<UserClass, Mode>& InTarget,
		typename EventType::template TSPMemberFunctionEventObserverType<UserClass, Mode>::MemberFunctionType InFunc
	)
	{
		static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You cannot use UObject method in this method.");

		checkSlow(InTarget);

		typename EventType::template TSPMemberFunctionEventObserverType<UserClass, Mode> Observer(InTarget, InFunc);

		return UnRegisterImpl<EventType>(&Observer);
	}

	/*
	* Register A member function for UObject
	*/
	template <typename EventType, typename UserClass, typename TEnableIf<TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline FDelegateHandle Register(
		UserClass* InTarget,
		typename EventType::template TObjectMemberFunctionEventObserverType<UserClass>::MemberFunctionType InFunc
	)
	{
		static_assert(UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You can only use UObject method in this method.");
		checkSlow(InTarget);

		typename EventType::template TObjectMemberFunctionEventObserverType<UserClass> Observer(InTarget, InFunc);
		return RegisterImpl<EventType>(&Observer);
	}

	template <typename EventType, typename UserClass, typename TEnableIf<TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline bool UnRegister(
		UserClass* InTarget,
		typename EventType::template TObjectMemberFunctionEventObserverType<UserClass>::MemberFunctionType InFunc
	)
	{
		static_assert(UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "You can only use UObject method in this method.");

		checkSlow(InTarget);

		typename EventType::template TObjectMemberFunctionEventObserverType<UserClass> Observer(InTarget, InFunc);

		return UnRegisterImpl<EventType>(&Observer);
	}

	/*
	* Register A lambda expression observer
	* It does not have a corresponding UnRegister function, you must use Handle to : 
	*	inline bool UnRegister(const FName& InEventName, FDelegateHandle InHandle)
	*/
	template <typename EventType, typename FunctorType, typename TEnableIf<TIsClass<FunctorType>::Value, int>::Type = 0>
	inline FDelegateHandle Register(FunctorType&& InFunctor)
	{
		typename EventType::template TFunctorEventObserverType<FunctorType> Observer(MoveTemp(InFunctor));

		return RegisterImpl<EventType>(&Observer);
	}

	// for Unreal UFunction
	template <typename EventType, typename UserClass, typename TEnableIf<TIsDerivedFrom<UserClass, UObject>::Value, int>::Type = 0>
	inline FDelegateHandle Register(UserClass* InTarget, const FName& InFunctionName)
	{
		return Register<UserClass>(EventType::GetEventName(), InTarget, InFunctionName);
	}

	/*
	* Clear all observers for an event
	*/
	template <typename EventType>
	inline void ClearEventObservers()
	{
		ClearEventObservers(EventType::GetEventName());
	}

	/*
	* Send an event and use the message definition structure as a template parameter. 
	* This will provide stricter compile-time checks for the entire system. 
	* It can also allow some implicit type conversions. 
	* For example, const TCHAR* can be used as FString, but other interfaces cannot.
	*/
	template <typename EventType, typename... ParamTypes>
	inline bool Broadcast(ParamTypes&&... InParams)
	{
		auto* Ptr = EventMaps.Find(EventType::GetEventName());

		using FInvokerBridgeType = typename EventType::FInvokerType;
		if (Ptr != nullptr)
		{
			if (!FInvokerBridgeType::Invoke(*Ptr, EventType::GetEventName(), InParams...))
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
				UDynamicEventContext* Context = EventType::FDynamicEventContextFactory::NewContext(InParams...);

				OnReceiveGlobalEvent.Broadcast(EventType::GetEventName(), Context);
			}
#endif

			return true;
		}

		return false;
	}

