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

namespace UE
{
    namespace GlobalEvents
    {
        namespace Details
        {
            /*
            * Listener base class for internal use
            */
            class GLOBALEVENTS_API IEventObserver
            {
            public:
                virtual ~IEventObserver() = default;

                virtual bool EqualTo(const IEventObserver* InOther) const = 0;
                virtual int GetType() const = 0;
                virtual bool IsGeneric() const = 0;

                virtual bool IsPendingDestroy() const = 0;
                virtual void SetPendingDestroy(bool bIsPendingDestroy) = 0;
                virtual IEventObserver* CloneAndMove() = 0;
                virtual FDelegateHandle GetHandle() const = 0;

                virtual void ExecuteInvoke(const void* InParams) = 0;
                virtual const ISignature* GetSignature() const = 0;
            };

            class GLOBALEVENTS_API FBaseEventObserver : public IEventObserver
            {
            public:
                FBaseEventObserver();
                FBaseEventObserver(FBaseEventObserver&& InOther) noexcept;

                virtual bool EqualTo(const IEventObserver* InOther) const override;
                virtual bool IsPendingDestroy() const override;
                virtual void SetPendingDestroy(bool bIsPendingDestroy) override;
                virtual FDelegateHandle GetHandle() const override;
                virtual bool IsGeneric() const override;

            protected:
                FDelegateHandle     Handle;
            private:
                bool                bPendingDestroy = false;
            };

            template <typename... ParamTypes>
            class TBaseEventObserver : public FBaseEventObserver
            {
            public:
                // don't use std::tuple
                typedef TTuple<typename TDecay<ParamTypes>::Type...>  ScriptableParamList;

                TBaseEventObserver()
                {
                }

                TBaseEventObserver(TBaseEventObserver&& InOther) noexcept :
                    FBaseEventObserver(MoveTemp(InOther))
                {
                }

            private:
                // help function, used to unpack tuple
                template <SIZE_T... I>
                void InvokeFuncWithTuple(ScriptableParamList& InParams, TIntegerSequence<SIZE_T, I...>)
                {
                    Invoke(InParams.template Get<I>()...);
                }

            public:
                virtual const ISignature* GetSignature() const override
                {
                    return &Signature;
                }

                virtual bool IsGeneric() const override
                {
                    return true;
                }

                virtual void ExecuteInvoke(const void* InParams) override
                {
                    if constexpr (sizeof...(ParamTypes) <= 0)
                    {
                        Invoke();
                    }
                    else
                    {
                        ScriptableParamList* ParamsTuplePtr = (ScriptableParamList*)InParams;

                        check(ParamsTuplePtr != nullptr);

                        InvokeFuncWithTuple(*ParamsTuplePtr, TMakeIntegerSequence<SIZE_T, sizeof...(ParamTypes)>{});
                    }
                }

                virtual void Invoke(ParamTypes... InParams) = 0;

            private:
                TGenericSignature<ParamTypes...>       Signature;
            };
        }
    }
}


