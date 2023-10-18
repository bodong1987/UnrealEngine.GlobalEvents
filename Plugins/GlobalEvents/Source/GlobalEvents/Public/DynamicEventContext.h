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

#include "CoreMinimal.h"
#include "DynamicTuple.h"
#include "DynamicEventContext.generated.h"

/*
* A wrapper class that provides data storage, signatures, etc., 
* and exposes various UFunctions for obtaining specific data, 
* which can be used to provide data for some external scripts, such as type-script.
*/
UCLASS(BlueprintType)
class GLOBALEVENTS_API UDynamicEventContext : public UObject
{
    GENERATED_BODY()
public:
    const UE::GlobalEvents::FDynamicTuple& GetParams() const{ return Params; }
	
    void Reset(UE::GlobalEvents::FDynamicTuple&& InParams);

	void Clear();

public:
	// Push a value to tuple
	// Exposing these interfaces enables secondary expansion externally
	template <typename T>
	void Add(const T& InValue)
	{
		Params.Push<T>(InValue);
	}

	// get a value reference from tuple
	// Exposing these interfaces enables secondary expansion externally
	template <typename T>
	T& Get(int InIndex)
	{
		return Params.Get<T>(InIndex);
	}

	// get a value reference from tuple
	// Exposing these interfaces enables secondary expansion externally
	template <typename T>
	const T& Get(int InIndex) const
	{
		return Params.Get<T>(InIndex);
	}

	// Add value based on Property
	void Add(const FProperty* InProperty, const void* InSourceAddress);

	// Add struct value dynamically
	void Add(const UScriptStruct* InStruct, const void* InSourceAddress);

public:
    UFUNCTION(BlueprintCallable)
    int Num() const{ return Params.Num(); }

	/*
	* Add these interface for external script engine
	* so these engine can add parameter to this context dynamically
	*/
#pragma region Export For External Script Engine
public:
    UFUNCTION()
    void AddBoolean(bool Value){ Params.Push(Value); }

    UFUNCTION(meta=(DisplayName="Add Int32"))
    void AddInt32(int32 Value){ Params.Push(Value); }

    UFUNCTION(meta = (DisplayName = "Add Int64"))
    void AddInt64(int64 Value){ Params.Push(Value); }

    UFUNCTION()
    void AddFloat(float Value){ Params.Push(Value); }

    UFUNCTION()
    void AddDouble(double Value){ Params.Push(Value); }

    UFUNCTION()
    void AddString(FString Value){ Params.Push(Value); }

    UFUNCTION()
    void AddName(FName Value){ Params.Push(Value); }

    UFUNCTION()
    void AddText(FText Value){ Params.Push(Value); }

    UFUNCTION()
    void AddObject(UObject* Value){ Params.Push(Value); }

    UFUNCTION(meta = (DisplayName = "Add Vector2D"))
	void AddVector2D(FVector2D Value) { Params.Push(Value); }

    UFUNCTION()
    void AddVector(FVector Vector) { Params.Push(Vector); }

	UFUNCTION(meta = (DisplayName = "Add Vector4"))
    void AddVector4(FVector4 Value) { Params.Push(Value); }

    UFUNCTION()
    void AddRotator(FRotator Value) { Params.Push(Value); }

    UFUNCTION()
	void AddQuat(FQuat Value) { Params.Push(Value); }

	UFUNCTION()
	void AddTransform(FTransform Value) { Params.Push(Value); }

    UFUNCTION()
	void AddColor(FColor Value) { Params.Push(Value); }

	UFUNCTION()
	void AddLinearColor(FLinearColor Value) { Params.Push(Value); }

public:
    UFUNCTION()
	bool GetBoolean(int Index) const { return Params.Get<bool>(Index); }

	UFUNCTION()
    int32 GetInt32(int Index) const { return Params.Get<int32>(Index); }

    UFUNCTION()
    int64 GetInt64(int Index) const { return Params.Get<int64>(Index); }
	
	UFUNCTION()
	float GetFloat(int Index) const { return Params.Get<float>(Index); }

	UFUNCTION()
	double GetDouble(int Index) const { return Params.Get<double>(Index); }
		
	UFUNCTION()
	FString GetString(int Index) const { return Params.Get<FString>(Index); }

	UFUNCTION()
	FName GetName(int Index) const { return Params.Get<FName>(Index); }

	UFUNCTION()
	FText GetText(int Index) const { return Params.Get<FText>(Index); }

	UFUNCTION()
	UObject* GetObject(int Index) const { return Params.Get<UObject*>(Index); }

	UFUNCTION()
	FVector2D GetVector2D(int Index) const { return Params.Get<FVector2D>(Index); }

	UFUNCTION()
	FVector GetVector(int Index) const { return Params.Get<FVector>(Index); }

	UFUNCTION()
	FVector4 GetVector4(int Index) const { return Params.Get<FVector4>(Index); }

	UFUNCTION()
	FRotator GetRotator(int Index) const { return Params.Get<FRotator>(Index); }

	UFUNCTION()
	FQuat GetQuat(int Index) const { return Params.Get<FQuat>(Index); }

	UFUNCTION()
	FTransform GetTransform(int Index) const { return Params.Get<FTransform>(Index); }

	UFUNCTION()
	FColor GetColor(int Index) const { return Params.Get<FColor>(Index); }

	UFUNCTION()
	FLinearColor GetLinearColor(int Index) const { return Params.Get<FLinearColor>(Index); }

#pragma endregion

private:
    UE::GlobalEvents::FDynamicTuple Params;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGlobalEventDelegateType, const FName&, EventName, UDynamicEventContext*, Context);

template <typename... ParamTypes>
class TDynamicEventContextFactory
{
public:
	// Create new dynamic event context from C++ variadic template parameters
	static UDynamicEventContext* NewContext(ParamTypes... InParams)
	{
		UDynamicEventContext* Context = NewObject<UDynamicEventContext>();

		if constexpr (sizeof...(ParamTypes) > 0)
		{
			Add(Context, InParams...);
		}

		return Context;
	}

private:
	template <typename T, typename... ExtraParamTypes>
	static void Add(UDynamicEventContext* InContext, T InValue, ExtraParamTypes... InParams)
	{
		InContext->Add<T>(InValue);

		if constexpr (sizeof...(ExtraParamTypes) > 0)
		{
			Add<ExtraParamTypes...>(InContext, InParams...);
		}		
	}
};


