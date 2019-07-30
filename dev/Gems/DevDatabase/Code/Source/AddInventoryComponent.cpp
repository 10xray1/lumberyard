
#include <AddInventoryComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

// Buses we send stuff to
#include <DevDatabase/DevDBBus.h>

namespace DevDatabase
{
    void AddInventoryComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<AddInventoryComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<AddInventoryComponent>("DevDatabase", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }
    void AddInventoryComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("AddInventoryService"));
    }
    void AddInventoryComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("AddInventoryService"));
    }
    void AddInventoryComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
		AZ_UNUSED(required);
    }
    void AddInventoryComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }
	void AddInventoryComponent::Init()
    {
    }
    void AddInventoryComponent::Activate()
    {
		AddInventoryRequestBus::Handler::BusConnect();
		CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler::BusConnect();
    }
    void AddInventoryComponent::Deactivate()
    {
		CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler::BusDisconnect();
		AddInventoryRequestBus::Handler::BusDisconnect();
    }

	void AddInventoryComponent::AddInventory(Aws::String& accountId, Aws::String& playerId)
	{
		AZ_TracePrintf("DevDatabase - AddInventory", "Creating item for player: %s", playerId);
		
		Aws::String sortKeyValue = "Player # " + playerId + " # Inventory";

		// Set table
		AwsModel::PutItemRequest request;
		request.SetTableName(m_tableName);

		// Set Keys
		request.AddItem(m_primaryKeyName, AwsModel::AttributeValue().SetS(accountId));
		request.AddItem(m_sortKeyName, AwsModel::AttributeValue().SetS(sortKeyValue));
		
		// Add attribute "Inventory" as a list
		// Not working yet, can't add null item
		//Aws::Vector<std::shared_ptr<Aws::DynamoDB::Model::AttributeValue>> emptyList;
		//request.AddItem("Inventory", AwsModel::AttributeValue().SetL(emptyList));

		DevDBRequestBus::Broadcast(&DevDBRequestBus::Events::AddItem, request);
	}

	void AddInventoryComponent::OnPostInitialization()
	{
		AZ_TracePrintf("DevDatabase - GetInventory", "AWS Initialization Complete.");
	}
}
