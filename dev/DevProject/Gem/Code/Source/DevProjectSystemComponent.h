#pragma once

#include <AzCore/Component/Component.h>

#include <DevProject/DevProjectBus.h>

namespace DevProject
{
    class DevProjectSystemComponent
        : public AZ::Component
        , protected DevProjectRequestBus::Handler
    {
    public:
        AZ_COMPONENT(DevProjectSystemComponent, "{E98E0B3D-6115-48D3-878E-BDF4AC0E4365}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // DevProjectRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
