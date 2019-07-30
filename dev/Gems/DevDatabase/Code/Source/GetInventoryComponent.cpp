
#include <GetInventoryComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <Include/DevDatabase/DevDBBus.h>

namespace DevDatabase
{
    void GetInventoryComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<GetInventoryComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<GetInventoryComponent>("DevDatabase", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }
    void GetInventoryComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("GetInventoryService"));
    }
    void GetInventoryComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("GetInventoryService"));
    }
    void GetInventoryComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
		AZ_UNUSED(required);
    }
    void GetInventoryComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }
	void GetInventoryComponent::Init()
    {
    }
    void GetInventoryComponent::Activate()
    {
		GetInventoryRequestBus::Handler::BusConnect();
		CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler::BusConnect();
    }
    void GetInventoryComponent::Deactivate()
    {
		CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler::BusDisconnect();
		GetInventoryRequestBus::Handler::BusDisconnect();
    }

	void GetInventoryComponent::GetInventoryList(Aws::String& accountId, Aws::String& playerId)
	{
		Aws::String sortKeyValue = "Player # " + playerId + " # Inventory";

		AZ_TracePrintf("DevDatabase - GetInventory", "Getting items for acct / sort: %s / %s", accountId.c_str(), sortKeyValue.c_str());

		// Set Table
		AwsModel::GetItemRequest request;
		request.SetTableName(m_tableName);

		// Set Keys
		Aws::Map<Aws::String, AwsModel::AttributeValue> keys;
		keys[m_primaryKeyName] = AwsModel::AttributeValue().SetS(accountId);
		keys[m_sortKeyName] = AwsModel::AttributeValue().SetS(sortKeyValue);
		request.SetKey(keys);

		request.SetProjectionExpression(Aws::String("Inventory")); // Optional - only grabs value from columns/attributes listed

		DevDBRequestBus::Broadcast(&DevDBRequestBus::Events::GetItem, request);
	}

	void GetInventoryComponent::OnPostInitialization()
	{
		AZ_TracePrintf("DevDatabase - GetInventory", "AWS Initialization Complete.");
	}
}
