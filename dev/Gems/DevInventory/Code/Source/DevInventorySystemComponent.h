#pragma once

#include <AzCore/Component/Component.h>

#include <DevInventory/DevInventoryBus.h>

namespace DevInventory
{
	class DevInventorySystemComponent
        : public AZ::Component
        , protected DevInventoryRequestBus::Handler
    {
    public:
        AZ_COMPONENT(DevInventorySystemComponent, "{0CEA8256-DDB4-44EA-BD23-9CE15B9E1797}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

//		static void CmdGivePlayerItem(IConsoleCmdArgs* pCmdArgs);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // DevInventoryRequestBus interface implementation
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
