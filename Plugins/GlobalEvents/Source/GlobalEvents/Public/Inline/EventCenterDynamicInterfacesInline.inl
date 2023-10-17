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

public:
    // Send an event, the parameters of this event are provided using DynamicTuple 
    inline bool BroadcastDynamic(const FName& InEventName, UDynamicEventContext* InContext)
    {
        checkSlow(InContext != nullptr);

        if (InContext == nullptr)
        {
            return false;
        }

        auto* Ptr = EventMaps.Find(InEventName);

        if (Ptr != nullptr)
        {
            auto Signal = Ptr->Get();

            checkSlow(Signal != nullptr);

            auto SourceSignature = InContext->GetParams().GetSignature();

            if (!Signal->GetSignature()->CheckInvokeableFrom(SourceSignature))
            {
                UE_LOG(GlobalEventsLog, Error,
                    TEXT("Invalid Operation, failed convert signature. EventName = (%s), Signal Signature = (%s), Broadcast Signature = (%s)"),
                    *InEventName.ToString(),
                    *Signal->GetSignature()->ToString(),
                    *SourceSignature->ToString()
                );

                return false;
            }

            Signal->ExecuteRaiseEvent(InContext->GetParams().GetData());

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
                OnReceiveGlobalEvent.Broadcast(InEventName, InContext);
            }
#endif
            return true;
        }

        return false;
    }

