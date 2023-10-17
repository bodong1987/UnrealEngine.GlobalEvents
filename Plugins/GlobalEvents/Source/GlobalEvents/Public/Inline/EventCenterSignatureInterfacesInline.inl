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
	template <typename FunctorType>
	inline bool BindSignatureInternal(const FName& InEventName, FunctorType&& InFunctor)
	{
		auto* Ptr = EventMaps.Find(InEventName);

		if (Ptr != nullptr)
		{
			return false;
		}

		EventMaps.Emplace(InEventName, InFunctor());

		return true;

	}

public:
	// Bind a signature to an event in advance
	template <typename EventType>
	inline bool BindSignature()
	{
		return BindSignatureInternal(EventType::GetEventName(), []()
			{
				return MakeShared<typename EventType::FSignalType>();
			}
		);
	}

	// Bind a signature to an event in advance
	template <typename... ParamTypes>
	inline bool BindSignature(const FName& InEventName)
	{
		return BindSignatureInternal(InEventName, []()
			{
				return MakeShared<UE::GlobalEvents::Details::TSignal<ParamTypes...>>();
			}
		);
	}

	// Bind a signature to an event in advance
	inline bool BindSignature(const FName& InEventName, UE::GlobalEvents::Details::FAnonymousSignal&& InSignal)
	{
		return BindSignatureInternal(InEventName, [&]()
			{
				return MakeShared<UE::GlobalEvents::Details::FAnonymousSignal>(MoveTemp(InSignal));
			}
		);
	}

	// Unbind a signature
	inline bool UnBindSignature(const FName& InEventName)
	{
		auto* Ptr = EventMaps.Find(InEventName);

		if (Ptr != nullptr && Ptr->Get()->IsEmpty())
		{
			EventMaps.Remove(InEventName);

			return true;
		}
		else if (Ptr == nullptr)
		{
			return true;
		}

		return false;
	}

	/*
	* Find current signature bind with this event
	*/
	inline const UE::GlobalEvents::ISignature* FindSignature(const FName& InEventName) const
	{
		auto* Ptr = EventMaps.Find(InEventName);

		return Ptr != nullptr && Ptr->Get() != nullptr ? Ptr->Get()->GetSignature() : nullptr;
	}


