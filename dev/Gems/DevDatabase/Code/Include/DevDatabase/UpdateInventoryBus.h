#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Math/Uuid.h>

namespace DevDatabase
{
    class UpdateInventoryRequests
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        // Put your public methods here
		virtual void AddItemToInventory(Aws::String& accountId, Aws::String& playerID, Aws::String& itemId, Aws::String& amount, Aws::String& slotId) = 0;

    };
    using UpdateInventoryRequestBus = AZ::EBus<UpdateInventoryRequests>;
} // namespace DevDatabase
