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
// The specific implementation of the listeners
#pragma once

#include "Details/EventObserverInterfaces.h"

namespace UE
{
    namespace GlobalEvents
    {
        namespace Details
        {
            enum class EEventObserverType
            {
                CommonFunction = 0x1,
                RawMemberFunction,
                UObjectMemberFunction,
                SPMemberFunction,
                FunctorFunction,

                GenericType_Max,

                UFunctionFunction,

                EventObserver_Max
            };

            template <typename... ParamTypes>
            class TCommonEventObserver : public TBaseEventObserver<ParamTypes...>
            {
            public:
                typedef void (*FunctionType)(ParamTypes...);
                typedef TBaseEventObserver<ParamTypes...> Super;
                typedef TCommonEventObserver<ParamTypes...> SelfType;

                TCommonEventObserver(FunctionType CommonFunction) :
                    Function(CommonFunction)
                {
                }

                TCommonEventObserver(SelfType&& Other) noexcept :
                    Super(MoveTemp(Other)),
                    Function(Other.Function)
                {
                    Other.Function = nullptr;
                }

                virtual void Invoke(ParamTypes... InParams) override
                {
                    if (Function != nullptr)
                    {
                        Function(InParams...);
                    }
                }

                virtual int GetType() const override
                {
                    return (int)EEventObserverType::CommonFunction;
                }

                virtual bool EqualTo(const IEventObserver* InOther) const override
                {
                    return InOther != nullptr &&
                        InOther->GetType() == (int)EEventObserverType::CommonFunction &&
                        ((const SelfType*)InOther)->Function == this->Function;
                }

                virtual IEventObserver* CloneAndMove() override
                {
                    return new SelfType(MoveTemp(*this));
                }

            private:
                FunctionType  Function;
            };

            template <typename UserClass, typename... ParamTypes>
            class TMemberFunctionEventObserver : public TBaseEventObserver<ParamTypes...>
            {
            public:
                typedef void (UserClass::* MemberFunctionType)(ParamTypes...);
                typedef TBaseEventObserver<ParamTypes...> Super;
                typedef TMemberFunctionEventObserver<UserClass, ParamTypes...> SelfType;

                static_assert(!UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "This class is only valid for non UObject.");

                TMemberFunctionEventObserver(UserClass* InTarget, MemberFunctionType InFunc) :
                    Target(InTarget),
                    Function(InFunc)
                {
                }

                TMemberFunctionEventObserver(SelfType&& InOther) noexcept :
                    Super(MoveTemp(InOther)),
                    Target(InOther.Target),
                    Function(InOther.Function)
                {
                }

                TMemberFunctionEventObserver(const SelfType&) = delete;

                virtual int GetType() const override
                {
                    return (int)EEventObserverType::RawMemberFunction;
                }

                virtual bool EqualTo(const IEventObserver* InOther) const override
                {
                    return InOther != nullptr &&
                        InOther->GetType() == (int)EEventObserverType::RawMemberFunction &&
                        ((const SelfType*)InOther)->Target == this->Target &&
                        ((const SelfType*)InOther)->Function == this->Function;
                }

                virtual IEventObserver* CloneAndMove() override
                {
                    return new SelfType(MoveTemp(*this));
                }

                virtual void Invoke(ParamTypes... InParams) override
                {
                    if (Target != nullptr && Function != nullptr)
                    {
                        (Target->*Function)(InParams...);
                    }
                }

            private:
                UserClass* Target;
                MemberFunctionType          Function;
            };

            template <typename UserClass, ESPMode Mode, typename... ParamTypes>
            class TSPMemberFunctionEventObserver : public TBaseEventObserver<ParamTypes...>
            {
            public:
                typedef void (UserClass::* MemberFunctionType)(ParamTypes...);
                typedef TBaseEventObserver<ParamTypes...> Super;
                typedef TSPMemberFunctionEventObserver<UserClass, Mode, ParamTypes...> SelfType;
                typedef TSharedPtr<UserClass, Mode> UserClassPtr;

                TSPMemberFunctionEventObserver(const UserClassPtr& InTarget, MemberFunctionType InFunc) :
                    Target(InTarget),
                    Function(InFunc)
                {
                }

                TSPMemberFunctionEventObserver(SelfType&& InOther) noexcept :
                    Super(MoveTemp(InOther)),
                    Target(MoveTemp(InOther.Target)),
                    Function(MoveTemp(InOther.Function))
                {
                }

                virtual int GetType() const override
                {
                    return (int)EEventObserverType::SPMemberFunction;
                }

                virtual bool EqualTo(const IEventObserver* InOther) const override
                {
                    return InOther != nullptr &&
                        InOther->GetType() == (int)EEventObserverType::SPMemberFunction &&
                        ((const SelfType*)InOther)->Target == this->Target &&
                        ((const SelfType*)InOther)->Function == this->Function;
                }

                virtual IEventObserver* CloneAndMove() override
                {
                    return new SelfType(MoveTemp(*this));
                }

                virtual void Invoke(ParamTypes... InParams) override
                {
                    if (Target.IsValid() && Function != nullptr)
                    {
                        (Target.Pin().Get()->*Function)(InParams...);
                    }
                }

            private:
                TWeakPtr<UserClass, Mode>   Target;
                MemberFunctionType          Function;
            };

            template <typename UserClass, typename... ParamTypes>
            class TBaseUObjectMemberFunctionEventObserver : public TBaseEventObserver<ParamTypes...>
            {
            public:
                typedef void (UserClass::* MemberFunctionType)(ParamTypes...);
                typedef TBaseEventObserver<ParamTypes...> Super;
                typedef TBaseUObjectMemberFunctionEventObserver<UserClass, ParamTypes...> SelfType;

                static_assert(UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "This class is only valid for UObject.");

                TBaseUObjectMemberFunctionEventObserver(UserClass* InTarget, MemberFunctionType InFunc) :
                    Target(InTarget),
                    Function(InFunc)
                {
                }

                TBaseUObjectMemberFunctionEventObserver(SelfType&& InOther) noexcept :
                    Super(MoveTemp(InOther)),
                    Target(MoveTemp(InOther.Target)),
                    Function(MoveTemp(InOther.Function))
                {
                }

                virtual int GetType() const override
                {
                    return (int)EEventObserverType::UObjectMemberFunction;
                }

                virtual bool EqualTo(const IEventObserver* InOther) const override
                {
                    return InOther != nullptr &&
                        InOther->GetType() == (int)EEventObserverType::UObjectMemberFunction &&
                        ((const SelfType*)InOther)->Target == this->Target &&
                        ((const SelfType*)InOther)->Function == this->Function;
                }

                virtual IEventObserver* CloneAndMove() override
                {
                    return new SelfType(MoveTemp(*this));
                }

                virtual void Invoke(ParamTypes... InParams) override
                {
                    // Verify that the user object is still valid.  We only have a weak reference to it.
                    checkSlow(Target.IsValid());

                    if (Function != nullptr)
                    {
                        if (UserClass* ActualUserObject = this->Target.Get())
                        {
                            (ActualUserObject->*Function)(InParams...);
                        }
                    }
                }

            private:
                TWeakObjectPtr<UserClass>   Target;
                MemberFunctionType          Function;
            };

            template <typename FunctorType, typename... ParamTypes>
            class TFunctorEventObserver : public TBaseEventObserver<ParamTypes...>
            {
            public:
                typedef TBaseEventObserver<ParamTypes...> Super;
                typedef TFunctorEventObserver<FunctorType, ParamTypes...> SelfType;

                TFunctorEventObserver(FunctorType&& InFunctor) :
                    Functor(MoveTemp(InFunctor))
                {
                }

                TFunctorEventObserver(SelfType&& InOther) noexcept :
                    Super(MoveTemp(InOther)),
                    Functor(MoveTemp(InOther.Functor))
                {
                }

                virtual int GetType() const override
                {
                    return (int)EEventObserverType::FunctorFunction;
                }

                virtual bool EqualTo(const IEventObserver* InOther) const override
                {
                    return false;
                }

                virtual IEventObserver* CloneAndMove() override
                {
                    return new SelfType(MoveTemp(*this));
                }

                virtual void Invoke(ParamTypes... InParams) override
                {
                    Functor(InParams...);
                }

            private:
                FunctorType                 Functor;
            };

            template <typename UserClass>
            class TUFunctionEventObserver : public FBaseEventObserver
            {
            public:
                typedef FBaseEventObserver                 Super;
                typedef TUFunctionEventObserver<UserClass> SelfType;

                static_assert(UE::GlobalEvents::Details::IsUObjectPtr((UserClass*)nullptr), "This class is only valid for UObject.");

                TUFunctionEventObserver(UserClass* InTarget, const FName& InFunctionName) :
                    CachedFunction(nullptr),
                    FunctionName(InFunctionName),
                    Target(InTarget)
                {
                    check(InFunctionName != NAME_None);

                    if (InTarget != nullptr)
                    {
                        CachedFunction = InTarget->FindFunctionChecked(FunctionName);

                        if (CachedFunction != nullptr)
                        {
                            Signature = FUFunctionSignature(CachedFunction);
                        }
                    }
                }

                TUFunctionEventObserver(UserClass* InTarget, UFunction* InFunction) :
                    CachedFunction(InFunction),
                    FunctionName(*InFunction->GetName()),
                    Target(InTarget),
                    Signature(InFunction)
                {
                }

                TUFunctionEventObserver(SelfType&& InOther) noexcept :
                    Super(MoveTemp(InOther)),
                    CachedFunction(nullptr),
                    FunctionName(MoveTemp(InOther.FunctionName)),
                    Target(MoveTemp(InOther.Target)),
                    Signature(MoveTemp(InOther.Signature))
                {
                    check(FunctionName != NAME_None);

                    if (Target.IsValid())
                    {
                        CachedFunction = Target.Get()->FindFunctionChecked(FunctionName);
                    }
                }

                virtual int GetType() const override
                {
                    return (int)EEventObserverType::UFunctionFunction;
                }

                virtual const ISignature* GetSignature(void) const override
                {
                    return &Signature;
                }

                virtual bool EqualTo(const IEventObserver* InOther) const override
                {
                    return InOther != nullptr &&
                        InOther->GetType() == (int)EEventObserverType::UFunctionFunction &&
                        ((const SelfType*)InOther)->Target == this->Target &&
                        ((const SelfType*)InOther)->FunctionName == this->FunctionName;
                }

                virtual IEventObserver* CloneAndMove() override
                {
                    return new SelfType(MoveTemp(*this));
                }

                virtual void ExecuteInvoke(const void* InParams) override
                {
                    checkSlow(Target.IsValid());

                    if (CachedFunction != nullptr)
                    {
                        if (UserClass* ActualUserObject = this->Target.Get())
                        {
                            ActualUserObject->ProcessEvent(CachedFunction, (void*)InParams);
                        }
                    }
                }

            private:
                UFunction* CachedFunction;
                FName                       FunctionName;
                TWeakObjectPtr<UserClass>   Target;
                FUFunctionSignature         Signature;
            };
        }
    }
}


