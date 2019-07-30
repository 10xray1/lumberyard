
#include <DevInventorySystemComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

//#include "D:\Lumberyard\1.19.0.0\dev\Code\CryEngine\CryCommon\IConsole.h"
//#include "D:\Lumberyard\1.19.0.0\dev\Code\CryEngine\CryCommon\ISystem.h"

namespace DevInventory
{
    void DevInventorySystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<DevInventorySystemComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<DevInventorySystemComponent>("DevInventory", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }
    void DevInventorySystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("DevInventoryService"));
    }
    void DevInventorySystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("DevInventoryService"));
    }
    void DevInventorySystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        AZ_UNUSED(required);
    }
    void DevInventorySystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }
    void DevInventorySystemComponent::Init()
    {
		//IConsole* pConsole = gEnv->pSystem->GetIConsole();
		//pConsole->AddCommand("g_test", CmdGivePlayerItem);

		//REGISTER_COMMAND("g_test", CmdGivePlayerItem, VF_CHEAT, "Ummmmmmmmmm");
    }
    void DevInventorySystemComponent::Activate()
    {
        DevInventoryRequestBus::Handler::BusConnect();
    }
    void DevInventorySystemComponent::Deactivate()
    {
        DevInventoryRequestBus::Handler::BusDisconnect();
    }

	//void DevInventorySystemComponent::CmdGivePlayerItem(IConsoleCmdArgs* pCmdArgs)
	//{
	//}
}
