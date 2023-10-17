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
#pragma once

#include "Details/Signature.h"
#include "Details/EventObservers.h"
#include "Details/Signals.h"

template <typename... ParamTypes>
class TDynamicEventContextFactory;

namespace UE 
{
    namespace GlobalEvents 
    {
        namespace Details
        {
            /*
            * In addition to using ordinary methods to send events,
            * you can also use the EventDefine mechanism to provide stricter compile-time type safety protection.
            * If you use a type-safe interface,
            * you will get a compile-time error when the signature of the event does not match the signature of the listener.
            */
            template <typename... ParamTypes>
            class TEventDefines
            {
            public:
                inline static const ISignature* StaticSignature()
                {
                    static const TGenericSignature<ParamTypes...> Z_Signature;
                    return &Z_Signature;
                }

                using FCommonEventObserverType = TCommonEventObserver<ParamTypes...>;
                using FSignalType = TSignal<ParamTypes...>;

                template <typename UserClass>
                using TMemberFunctionEventObserverType = TMemberFunctionEventObserver<UserClass, ParamTypes...>;

                template <typename UserClass, ESPMode Mode>
                using TSPMemberFunctionEventObserverType = TSPMemberFunctionEventObserver<UserClass, Mode, ParamTypes...>;

                template <typename UserClass>
                using TObjectMemberFunctionEventObserverType = TBaseUObjectMemberFunctionEventObserver<UserClass, ParamTypes...>;

                using FInvokerType = TSignalInvoker<ParamTypes...>;

                using FDynamicEventContextFactory = TDynamicEventContextFactory<ParamTypes...>;

                template <typename LambdaExpressionType>
                using TFunctorEventObserverType = TFunctorEventObserver<LambdaExpressionType, ParamTypes...>;

            protected:
                // convert _ to .
                // so we can use it as GameplayTag 
                inline static FName GetDomainEventName(const char* InInputName)
                {
                    FString Text(InInputName);
                    for (int32 i = 0; i < Text.Len(); ++i)
                    {
                        if (Text[i] == TEXT('_'))
                        {
                            Text[i] = TEXT('.');
                        }
                    }

                    return FName(Text);
                }
            };
        }
    }
}

/*
* Use this macro to define your own messages.
* Note that underscores in names will be replaced with periods.
* This way it can be expanded into GameplayTags in the future
*/
#define DEFINE_TYPESAFE_GLOBAL_EVENT(name, ...)  \
	class F##name : public UE::GlobalEvents::Details::TEventDefines<__VA_ARGS__> \
	{ \
	private: \
		F##name() = delete; \
		~F##name() = delete; \
	public: \
		static const FName& GetEventName() \
		{ \
			static const FName Z_Name = GetDomainEventName(#name); \
			return Z_Name; \
		} \
	} 



