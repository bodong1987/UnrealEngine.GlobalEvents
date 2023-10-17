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
#include "Details/EventObserverInterfaces.h"

namespace UE
{
    namespace GlobalEvents
    {
        namespace Details
        {
            FBaseEventObserver::FBaseEventObserver() :
                Handle(FDelegateHandle::GenerateNewHandle)
            {
            }

            FBaseEventObserver::FBaseEventObserver(FBaseEventObserver&& InOther) noexcept :
                Handle(InOther.Handle)
            {
            }

            bool FBaseEventObserver::IsPendingDestroy() const
            {
                return bPendingDestroy;
            }

            void FBaseEventObserver::SetPendingDestroy(bool bIsPendingDestroy)
            {
                bPendingDestroy = bIsPendingDestroy;
            }

            bool FBaseEventObserver::IsGeneric() const
            {
                return false;
            }

            FDelegateHandle FBaseEventObserver::GetHandle() const
            {
                return Handle;
            }

            bool FBaseEventObserver::EqualTo(const IEventObserver* InOther) const
            {
                return InOther != nullptr && GetType() == InOther->GetType();
            }
        }
    }
}

