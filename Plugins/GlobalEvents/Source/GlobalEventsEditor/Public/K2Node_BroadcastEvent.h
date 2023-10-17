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
#include "K2Node.h"
#include "K2Node_BroadcastEvent.generated.h"

/**
 * 
 */
UCLASS()
class GLOBALEVENTSEDITOR_API UK2Node_BroadcastEvent : public UK2Node
{
	GENERATED_BODY()
	
public:
    virtual FText GetTooltipText() const override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FText GetMenuCategory() const override;
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& actionRegistrar) const override;

    virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& oldPins) override;
    virtual void AllocateDefaultPins() override;

    virtual void PinDefaultValueChanged(UEdGraphPin* pin) override;
    virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;

    virtual void EarlyValidation(class FCompilerResultsLog& messageLog) const override;

    virtual bool IsDefaultPin(const UEdGraphPin* pin) const;

    static UEdGraphPin* FindPin(const TArray<UEdGraphPin*>& InPins, const FName& InPinName);
    UEdGraphPin* FindPin(const FName& InPinName) const;

    virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
    virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
    virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

    void AddDynamicPin();
    void RemoveDynamicPin(const UEdGraphPin* pin);
    
private:
    void AllocateDynamicPins();
};

