#pragma once

#include <AzCore/Component/Component.h>

#include <DevUI/DevUIBus.h>

namespace DevUI
{
    class DevUISystemComponent
        : public AZ::Component
        , protected DevUIRequestBus::Handler
    {
    public:
        AZ_COMPONENT(DevUISystemComponent, "{17A3082F-E1F2-45C1-818B-B2CE9CE00E13}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // DevUIRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
