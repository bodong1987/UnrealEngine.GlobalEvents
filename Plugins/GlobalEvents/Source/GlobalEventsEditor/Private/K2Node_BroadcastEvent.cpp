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
#include "K2Node_BroadcastEvent.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "EdGraphSchema_K2.h"
#include "GameplayTagContainer.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetNodes/SGraphNodeK2Base.h"
#include "GraphEditorSettings.h"
#include "K2Node_GenericCreateObject.h"
#include "DynamicEventContext.h"
#include "KismetCompiler.h"
#include "K2Node_CallFunction.h"
#include "DynamicEventFunctionLibrary.h"
#include "GlobalEventsLog.h"
#include "ToolMenu.h"

class SGraphNodeBroadcastScriptEvent : public SGraphNodeK2Base
{
public:
    SLATE_BEGIN_ARGS(SGraphNodeBroadcastScriptEvent) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UK2Node_BroadcastEvent* InNode)
    {
        this->GraphNode = InNode;
        this->SetCursor(EMouseCursor::CardinalCross);
        this->UpdateGraphNode();
    }

private:
    virtual void CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox) override
    {
        FText Tmp = FText::FromString(TEXT("Add Argument"));
        TSharedRef<SWidget> AddPinButton = AddPinButtonContent(Tmp, Tmp);

        FMargin AddPinPadding = Settings->GetInputPinPadding();
        AddPinPadding.Top += 6.0f;

        InputBox->AddSlot()
            .AutoHeight()
            .VAlign(VAlign_Center)
            .Padding(AddPinPadding)
            [
                AddPinButton
            ];
    }

    virtual FReply OnAddPin() override
    {
        UK2Node_BroadcastEvent* Node = Cast<UK2Node_BroadcastEvent>(this->GraphNode);

        check(Node != nullptr);

        Node->AddDynamicPin();      
        
        UpdateGraphNode();

        return FReply::Handled();
    }
};

static const FName EVENT_NAME("EventTag");
static const FName PARAM_NAME("Param");
static const FName RESULT_NAME("Result");

FText UK2Node_BroadcastEvent::GetTooltipText() const
{
    return FText::FromString(TEXT("Send a global event"));
}

FText UK2Node_BroadcastEvent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString(TEXT("Broadcast Global Event"));
}

FText UK2Node_BroadcastEvent::GetMenuCategory() const
{
    return FText::FromString(TEXT("Global Events"));
}

void UK2Node_BroadcastEvent::GetMenuActions(FBlueprintActionDatabaseRegistrar& actionRegistrar) const
{
    Super::GetMenuActions(actionRegistrar);

    if (!actionRegistrar.IsOpenForRegistration(GetClass()))
    {
        return;
    }

    UBlueprintNodeSpawner* nodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
    actionRegistrar.AddBlueprintAction(GetClass(), nodeSpawner);
}

void UK2Node_BroadcastEvent::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
    Super::GetNodeContextMenuActions(Menu, Context);

    if (Context->bIsDebugging)
    {
        return;
    }

    if (Context->Pin && Context->Pin->Direction == EGPD_Input && !IsDefaultPin(Context->Pin))
    {
        // Add a new section to the context menu
        FToolMenuSection& Section = Menu->AddSection("BroadcastGlobalEvent", FText::FromString("BroadcastGlobalEvent"));

        // Add a custom menu entry for deleting the pin
        Section.AddMenuEntry(
            "Delete Pin",
            FText::FromString("Delete Pin"),
            FText::FromString("Delete the selected pin"),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateUObject((UK2Node_BroadcastEvent*)this, &UK2Node_BroadcastEvent::RemoveDynamicPin, Context->Pin),
                EUIActionRepeatMode::RepeatEnabled
            )
        );
    }
}

void UK2Node_BroadcastEvent::AddDynamicPin()
{
    int Count = 0;
    FString PinName;
    do
    {
        PinName = FString::Printf(TEXT("%s%d"), *PARAM_NAME.ToString(), Count++);

        if (FindPin(*PinName) == nullptr)
        {
            break;
        }
    }while(Count < 1000);

    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, *PinName);

    Modify();
    FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
    
    GetGraph()->NotifyGraphChanged();
}

void UK2Node_BroadcastEvent::RemoveDynamicPin(const UEdGraphPin* pin)
{
    if (!IsDefaultPin(pin))
    {
        RemovePin((UEdGraphPin*)pin);

        Modify();
        FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());

        GetGraph()->NotifyGraphChanged();
    }
}

TSharedPtr<SGraphNode> UK2Node_BroadcastEvent::CreateVisualWidget()
{
    return SNew(SGraphNodeBroadcastScriptEvent, this);
}

UEdGraphPin* UK2Node_BroadcastEvent::FindPin(const TArray<UEdGraphPin*>& InPins, const FName& InPinName)
{
    for (UEdGraphPin* pin : InPins)
    {
        if (pin->PinName == InPinName)
        {
            return pin;
        }
    }

    return nullptr;
}

UEdGraphPin* UK2Node_BroadcastEvent::FindPin(const FName& InPinName) const
{
    return FindPin(Pins, InPinName);
}

void UK2Node_BroadcastEvent::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& oldPins)
{
    Super::ReallocatePinsDuringReconstruction(oldPins);

    {
        UEdGraphPin* oldNamePin = FindPin(oldPins, EVENT_NAME);

        if (oldNamePin != nullptr)
        {
            UEdGraphPin* newNamePin = FindPin(EVENT_NAME);
            if (newNamePin != nullptr)
            {
                newNamePin->DefaultValue = oldNamePin->DefaultValue;
            }
        }
    }

    for (UEdGraphPin* oldPin : oldPins)
    {
        if (IsDefaultPin(oldPin))
        {
            continue;
        }

        UEdGraphPin* pin = FindPin(oldPin->PinName);

        if (pin == nullptr)
        {
            // try recreate it
            pin = CreatePin(EGPD_Input, oldPin->PinType, oldPin->PinName);
            check(pin);
        }

        pin->DefaultValue = oldPin->DefaultValue;
    }
}

void UK2Node_BroadcastEvent::AllocateDefaultPins()
{
    // default flow
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

    // name
    UEdGraphPin* NamePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, FGameplayTag::StaticStruct(), EVENT_NAME);
    checkSlow(NamePin != nullptr);

    NamePin->PinToolTip = TEXT("Select an event name");
    NamePin->bNotConnectable = true;

    // Result pin
    UEdGraphPin* ResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, RESULT_NAME);
    checkSlow(ResultPin != nullptr);
}

void UK2Node_BroadcastEvent::PinDefaultValueChanged(UEdGraphPin* pin)
{
}

void UK2Node_BroadcastEvent::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
    if (IsDefaultPin(Pin))
    {
        return;
    }

    if (Pin && Pin->Direction == EGPD_Input)
    {
        if (Pin->LinkedTo.Num() > 0)
        {
            // 
//             Pin->PinType.PinCategory = Pin->LinkedTo[0]->PinType.PinCategory;
//             Pin->PinType.PinSubCategory = Pin->LinkedTo[0]->PinType.PinSubCategory;
//             Pin->PinType.PinSubCategoryObject = Pin->LinkedTo[0]->PinType.PinSubCategoryObject;
            Pin->PinType = Pin->LinkedTo[0]->PinType;
        }
        else
        {
            Pin->PinType.ResetToDefaults();

            Pin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
            Pin->PinType.PinSubCategory = NAME_None;
            Pin->PinType.PinSubCategoryObject = nullptr;
        }
    }
}

void UK2Node_BroadcastEvent::EarlyValidation(class FCompilerResultsLog& messageLog) const
{
    Super::EarlyValidation(messageLog);

    if (FindPinChecked(UEdGraphSchema_K2::PN_Execute)->LinkedTo.Num() == 0)
    {
        return;
    }

    UEdGraphPin* NamePin = FindPin(EVENT_NAME);
    checkSlow(NamePin);

    // if name is not connected
    if (NamePin->LinkedTo.Num() == 0)
    {
        FName EventName = FName(NamePin->GetDefaultAsString());

        if (EventName == NAME_None || EventName.ToString().TrimStartAndEnd().IsEmpty())
        {
            messageLog.Warning(*FString::Printf(TEXT("[@@]You must provide the event name [%s]"), *NamePin->GetDisplayName().ToString()), this);
        }
    }

    // check all param pins
    for (UEdGraphPin* Pin : Pins)
    {
        if (IsDefaultPin(Pin))
        {
            continue;
        }

        if (Pin->Direction == EGPD_Input && 
            Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
        {
            messageLog.Error(
                *FString::Printf(TEXT("[@@]You need to link Pin[%s] to a Pin of a certain type to determine the type of Pin"), *Pin->GetDisplayName().ToString()), 
                this
            );
        }
    }    
}

bool UK2Node_BroadcastEvent::IsDefaultPin(const UEdGraphPin* pin) const
{
    const FName name = pin->GetFName();

    return name == UEdGraphSchema_K2::PN_Execute ||
        name == UEdGraphSchema_K2::PN_Then ||
        name == EVENT_NAME ||
        name == RESULT_NAME;
}

void UK2Node_BroadcastEvent::AllocateDynamicPins()
{
   
}

void UK2Node_BroadcastEvent::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

    UK2Node_GenericCreateObject* CreateNode = CompilerContext.SpawnIntermediateNode<UK2Node_GenericCreateObject>(this, SourceGraph);
    CreateNode->NodePosX = NodePosX;
    CreateNode->NodePosY = NodePosY;
    CreateNode->AllocateDefaultPins();
    
    Schema->TrySetDefaultObject(*CreateNode->GetClassPin(), UDynamicEventContext::StaticClass());

    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CreateNode, this);

    CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *CreateNode->GetExecPin());

    UK2Node* PreviousNode = CreateNode;

    UFunction* AddParamFunction = UDynamicEventFunctionLibrary::StaticClass()->FindFunctionByName(UDynamicEventFunctionLibrary::PushDynamicEventFunctionName);

    check(AddParamFunction != nullptr);

    for (UEdGraphPin* Pin : Pins)
    {
        if (IsDefaultPin(Pin))
        {
            continue;
        }

        // create a Call function node, used to invoke AddXXX function on UDynamicEventContext
        UK2Node_CallFunction* CallFunctionNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
        
        CallFunctionNode->SetFromFunction(AddParamFunction);
        CallFunctionNode->AllocateDefaultPins();

        CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallFunctionNode, this);

        UEdGraphPin* ContextPin = CallFunctionNode->FindPin(TEXT("Context"), EGPD_Input);
        check(ContextPin != nullptr);

        Schema->TryCreateConnection(CreateNode->GetResultPin(), ContextPin);

        UEdGraphPin* ValuePin = CallFunctionNode->FindPin(TEXT("Value"), EGPD_Input);
        check(ValuePin != nullptr);

        // reset value pin
        ValuePin->PinType = Pin->PinType;

        CompilerContext.MovePinLinksToIntermediate(*Pin, *ValuePin);        

        // connect nodes as a chain...
        Schema->TryCreateConnection(PreviousNode->FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output), CallFunctionNode->GetExecPin());

        PreviousNode = CallFunctionNode;
    }

    UK2Node_CallFunction* CallSendEventNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);    
    CallSendEventNode->SetFromFunction(
        UDynamicEventFunctionLibrary::StaticClass()->FindFunctionByName(
            GET_FUNCTION_NAME_CHECKED(UDynamicEventFunctionLibrary, BroadcastEvent)
        )
    );
    CallSendEventNode->AllocateDefaultPins();

    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallSendEventNode, this);

    Schema->TryCreateConnection(PreviousNode->FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output), CallSendEventNode->GetExecPin());

    UEdGraphPin* ContextPin = CallSendEventNode->FindPin(TEXT("Context"));
    check(ContextPin != nullptr);

    Schema->TryCreateConnection(CreateNode->GetResultPin(), ContextPin);
    
    UEdGraphPin* NamePin = FindPin(EVENT_NAME);
    checkSlow(NamePin);

    UEdGraphPin* EventNamePin = CallSendEventNode->FindPin(EVENT_NAME);
    check(EventNamePin!= nullptr);

    if (NamePin->LinkedTo.Num() > 0)
    {        
        CompilerContext.MovePinLinksToIntermediate(*NamePin, *EventNamePin);
    }
    else
    {
        Schema->TrySetDefaultValue(*EventNamePin, NamePin->GetDefaultAsString());
    }

    UEdGraphPin* ResultValuePin = CallSendEventNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
    check(ResultValuePin);

    UEdGraphPin* ThisResultPin = FindPin(RESULT_NAME);
    check(ThisResultPin);

    CompilerContext.MovePinLinksToIntermediate(*ThisResultPin, *ResultValuePin);

    CompilerContext.MovePinLinksToIntermediate(*UK2Node::FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output), *CallSendEventNode->FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output));

    // remove all 
    BreakAllNodeLinks();
}







