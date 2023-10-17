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

#include "EventParamType.h"

namespace UE
{
    namespace GlobalEvents
    {
        /*
        * Signature information, this information is used to identify the constraint information required for a certain target.
        * It can be judged whether the messages are compatible with each other, etc.
        */
        class GLOBALEVENTS_API ISignature
        {
        public:
            virtual ~ISignature() = default;

        public:
            virtual const FName& GetName() const = 0;
            virtual bool IsValid() const = 0;

            // Check whether the two signatures are exactly the same
            virtual bool EqualTo(const ISignature* InOtherSignature) const = 0;

            // check if this signature's method can be invoked from the target signature's method
            virtual bool CheckInvokeableFrom(const ISignature* InInvokerSignature) const = 0;

            virtual const TArray<FGlobalEventParamType>& GetParameters() const = 0;
            virtual void Clear() = 0;

            inline FString ToString() const
            {
                return GetName().ToString();
            }
        };
    }
}
