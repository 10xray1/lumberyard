
#include <UpdateInventoryComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

namespace DevDatabase
{
    void UpdateInventoryComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<UpdateInventoryComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<UpdateInventoryComponent>("DevDatabase", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }
    void UpdateInventoryComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("UpdateInventoryService"));
    }
    void UpdateInventoryComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("UpdateInventoryService"));
    }
    void UpdateInventoryComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
		AZ_UNUSED(required);
    }
    void UpdateInventoryComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }
	void UpdateInventoryComponent::Init()
    {
    }
    void UpdateInventoryComponent::Activate()
    {
		UpdateInventoryRequestBus::Handler::BusConnect();
		//CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler::BusConnect();
		AZ::EntityBus::Handler::BusConnect(GetEntityId());
    }
    void UpdateInventoryComponent::Deactivate()
    {
		AZ::EntityBus::Handler::BusDisconnect(GetEntityId());
		//CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler::BusDisconnect();
		UpdateInventoryRequestBus::Handler::BusDisconnect();
    }
	void UpdateInventoryComponent::OnEntityActivated(const AZ::EntityId& entityId)
	{
		AZ_UNUSED(entityId);
		m_pDevRequests = DevDBRequestBus::FindFirstHandler();
	}
	void UpdateInventoryComponent::OnEntityDeactivated(const AZ::EntityId& entityId)
	{
		AZ_UNUSED(entityId);
		m_pDevRequests = nullptr;
	}

	void UpdateInventoryComponent::AddItemToInventory(Aws::String& accountId, Aws::String& playerId, Aws::String& itemId, Aws::String& amount, Aws::String& slotId)
	{
		Aws::String sortKeyValue = "Player # " + playerId + " # Inventory";
		AZ_TracePrintf("DevDatabase - UpdateInventory", "Adding item for acct / sort: %s / %s", accountId, sortKeyValue);

		// Set table name
		AwsModel::UpdateItemRequest request;
		request.SetTableName(m_tableName);

		// Set keys
		Aws::Map<Aws::String, AwsModel::AttributeValue> keys;
		keys[m_primaryKeyName] = AwsModel::AttributeValue().SetS(accountId);
		keys[m_sortKeyName] = AwsModel::AttributeValue().SetS(sortKeyValue);
		request.SetKey(keys);

		//OLD list type
		// Make item map
		//AwsModel::AttributeValue valueMapAttribute;
		//valueMapAttribute.AddMEntry("itemId", Aws::MakeShared<AwsModel::AttributeValue>("UpdateInventoryItemList", itemId));
		//valueMapAttribute.AddMEntry("itemAmount", Aws::MakeShared<AwsModel::AttributeValue>("UpdateInventoryItemList", amount));
		//valueMapAttribute.AddMEntry("inventorySlot", Aws::MakeShared<AwsModel::AttributeValue>("UpdateInventoryItemList", slotId));

		// Add map to inventory list
		//AwsModel::AttributeValue finalAttribute;
		//finalAttribute.AddLItem(Aws::MakeShared<AwsModel::AttributeValue>("UpdateInventoryItemList", valueMapAttribute));

		//Aws::String updateExpression = "SET #inventory = list_append(#inventory, :vals)";  //append, not replace

		// NEW -- adds item, or replaces current item
		// Make item map
		AwsModel::AttributeValue finalAttribute;
		finalAttribute.AddMEntry("itemAmount", Aws::MakeShared<AwsModel::AttributeValue>("UpdateInventoryItemList", amount));
		finalAttribute.AddMEntry("inventorySlot", Aws::MakeShared<AwsModel::AttributeValue>("UpdateInventoryItemList", slotId));
		
		// Set Update Expression (how the map is added)
		Aws::String updateExpression = "SET #inventory.#item = :vals";  // added for map version
		request.SetUpdateExpression(updateExpression);

		// Expression Names
		Aws::Map<Aws::String, Aws::String> expressionAttributeNames;
		expressionAttributeNames["#inventory"] = "Inventory";
		expressionAttributeNames["#item"] = itemId;  // added for map version
		request.SetExpressionAttributeNames(expressionAttributeNames);

		// Expression Values
		Aws::Map<Aws::String, AwsModel::AttributeValue> expressionAttributeValues;
		expressionAttributeValues[":vals"] = finalAttribute;
		request.SetExpressionAttributeValues(expressionAttributeValues);

		//DevDBRequestBus::Broadcast(&DevDBRequestBus::Events::UpdateItem, request);
		m_pDevRequests->UpdateItem(request);
	}

	void UpdateInventoryComponent::OnPostInitialization()
	{
		AZ_TracePrintf("DevDatabase - UpdateInventory", "AWS Initialization Complete.");
	}
}
