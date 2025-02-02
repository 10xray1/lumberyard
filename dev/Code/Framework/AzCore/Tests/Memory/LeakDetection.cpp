/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <AzCore/Memory/BestFitExternalMapAllocator.h>
#include <AzCore/Memory/PoolAllocator.h>
#include <AzCore/UnitTest/TestTypes.h>


namespace UnitTest
{
    // Dummy test class
    class TestClass
    {
    public:
        AZ_CLASS_ALLOCATOR(TestClass, AZ::SystemAllocator, 0);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Testing the AllocatorsTestFixture base class. Testing that detects leaks
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class AllocatorsTestFixtureLeakDetectionTest
        : public AllocatorsTestFixture
        , public UnitTest::TraceBusRedirector
    {
    public:
        void SetUp() override
        {
            AllocatorsTestFixture::SetUp();

            AZ::Debug::TraceMessageBus::Handler::BusConnect();
        }
        void TearDown() override
        {
            AllocatorsTestFixture::TearDown();

            EXPECT_EQ(m_leakExpected, m_leakDetected);
            AZ::Debug::TraceMessageBus::Handler::BusDisconnect();

            if (m_leakExpected)
            {
                AZ_TEST_STOP_ASSERTTEST(1);
            }
        }

        void SetLeakExpected() { AZ_TEST_START_ASSERTTEST; m_leakExpected = true; }

    private:
        bool OnPreError(const char* window, const char* file, int line, const char* func, const char* message) override
        {
            AZ_UNUSED(file);
            AZ_UNUSED(line);
            AZ_UNUSED(func);

            if (AZStd::string_view(window) == "Memory"
                && AZStd::string_view(message) == "We still have 1 allocations on record! They must be freed prior to destroy!")
            {
                // Leak detected, flag it so we validate on tear down that this happened. We also will 
                // mark this test since it will assert
                m_leakDetected = true;
                return true;
            }
            return false;
        }

        bool OnPrintf(const char* window, const char* message) override
        {
            AZ_UNUSED(window);
            AZ_UNUSED(message);
            // Do not print the error message twice. The error message will already be printed by the TraceBusRedirector
            // in UnitTest.h. Here we override it to prevent it from printing twice.
            return true;
        }

        AZ::Debug::DrillerManager* m_drillerManager = nullptr;
        bool m_leakDetected = false;
        bool m_leakExpected = false;
    };

    TEST_F(AllocatorsTestFixtureLeakDetectionTest, Leak)
    {
        SetLeakExpected();

        TestClass* leakyObject = aznew TestClass();
        AZ_UNUSED(leakyObject);
    }

    TEST_F(AllocatorsTestFixtureLeakDetectionTest, NoLeak)
    {
        TestClass* leakyObject = aznew TestClass();
        delete leakyObject;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Testing Allocator leaks
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Create a dummy allocator so unit tests can leak it
    class TestAllocator
        : public AZ::AllocatorBase<AZ::ChildAllocatorSchema<AZ::SystemAllocator>>
    {
    public:
        AZ_TYPE_INFO(TestAllocator, "{186B6E32-344D-4322-820A-4C3E4F30650B}");

        using Base = AZ::AllocatorBase<AZ::ChildAllocatorSchema<AZ::SystemAllocator>>;
        using Schema = Base::Schema;
        using Descriptor = Base::Descriptor;

        TestAllocator()
            : TestAllocator(TYPEINFO_Name(), "TestAllocator")
        {
        }

        TestAllocator(const char* name, const char* desc)
            : Base(name, desc)
        {
            m_schema = new (&m_schemaStorage) Schema(Descriptor());
        }

        ~TestAllocator() override = default;
    };

    class AllocatorsTestFixtureLeakDetectionDeathTest
        : public ::testing::Test
    {
    public:
        void TestAllocatorLeak()
        {
            TraceBusHook traceBusHook;
            traceBusHook.SetupEnvironment();

            AZ::AllocatorInstance<TestAllocator>::Create();

            traceBusHook.TeardownEnvironment();
        }
    };
   
    TEST_F(AllocatorsTestFixtureLeakDetectionDeathTest, AllocatorLeak)
    {
        // testing that the TraceBusHook will fail on cause the test to die
        EXPECT_DEATH(TestAllocatorLeak(), "");
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Testing ScopedAllocatorSetupFixture. Testing that detects leaks
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class AllocatorSetupLeakDetectionTest
        : public ::testing::Test
    {
        // Internal class to manage the bus redirector so we can use construction/destruction order to detect leaks triggered
        // by AllocatorSetup destructor.
        class BusRedirector
            : public UnitTest::TraceBusRedirector
        {
        public:
            BusRedirector()
            {
                AZ::Debug::TraceMessageBus::Handler::BusConnect();
            }
            ~BusRedirector()
            {
                EXPECT_EQ(m_leakExpected, m_leakDetected);
                AZ::Debug::TraceMessageBus::Handler::BusDisconnect();

                if (m_leakExpected)
                {
                    // The macro AZ_TEST_STOP_ASSERTTEST contains a return statement, therefore we cannot use it in a destructor, 
                    // to overcome that, we wrap it in a lambda so we can drop the returned value.
                    auto stopAsserts = [] {
                        AZ_TEST_STOP_ASSERTTEST(1);
                    };
                    stopAsserts();
                }
            }

            bool OnPreError(const char* window, const char* file, int line, const char* func, const char* message) override
            {
                AZ_UNUSED(file);
                AZ_UNUSED(line);
                AZ_UNUSED(func);

                if (AZStd::string_view(window) == "Memory"
                    && AZStd::string_view(message) == "We still have 1 allocations on record! They must be freed prior to destroy!")
                {
                    // Leak detected, flag it so we validate on tear down that this happened. We also will 
                    // mark this test since it will assert
                    m_leakDetected = true;
                    return true;
                }
                return false;
            }

            bool OnPrintf(const char* window, const char* message) override
            {
                AZ_UNUSED(window);
                AZ_UNUSED(message);
                // Do not print the error message twice. The error message will already be printed by the TraceBusRedirector
                // in UnitTest.h. Here we override it to prevent it from printing twice.
                return true;
            }

            bool m_leakDetected = false;
            bool m_leakExpected = false;
        };

        // Inheriting to add default implementations for the virtual abstract methods.
        class AllocatorSetup : public ScopedAllocatorSetupFixture
        {
        public:
            void SetUp() override {}
            void TearDown() override {}
            void TestBody() override {}
        };

    public:
        ~AllocatorSetupLeakDetectionTest()
        {
            EXPECT_EQ(m_busRedirector.m_leakExpected, UnitTest::TestRunner::Instance().m_isAssertTest);
        }

        void SetLeakExpected() { AZ_TEST_START_ASSERTTEST; m_busRedirector.m_leakExpected = true; }

    private:
        BusRedirector m_busRedirector;
        // We need the BusRedirector to be destroyed before the AllocatorSetup is, therefore we cannot have this test fixture inheriting 
        // from AllocatorSetup which would be the default implementation. 
        AllocatorSetup m_allocatorSetup;
    };

    TEST_F(AllocatorSetupLeakDetectionTest, Leak)
    {
        SetLeakExpected();

        TestClass* leakyObject = aznew TestClass();
        AZ_UNUSED(leakyObject);
    }

    TEST_F(AllocatorSetupLeakDetectionTest, NoLeak)
    {
        TestClass* leakyObject = aznew TestClass();
        delete leakyObject;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Testing Allocators, testing that the different allocator types detect leaks
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename AllocatorType>
    class AllocatorTypeLeakDetectionTest
        : public ::testing::Test
    {
    public:
        AllocatorTypeLeakDetectionTest()
            : m_busRedirector(m_leakDetected)
        {}

        void SetUp() override
        {
            m_drillerManager = AZ::Debug::DrillerManager::Create();
            m_drillerManager->Register(aznew AZ::Debug::MemoryDriller);

            if (azrtti_typeid<AllocatorType>() != azrtti_typeid<AZ::SystemAllocator>()) // simplifies instead of template specialization
            {
                // Other allocators need the SystemAllocator in order to work
                AZ::AllocatorInstance<AZ::SystemAllocator>::Create();
            }
            AZ::AllocatorInstance<AllocatorType>::Create();
            AZ::Debug::AllocationRecords* records = AZ::AllocatorInstance<AllocatorType>::Get().GetRecords();
            if (records)
            {
                records->SetMode(AZ::Debug::AllocationRecords::RECORD_FULL);
            }

            m_busRedirector.BusConnect();
        }

        void TearDown() override
        {
            AZ::AllocatorInstance<AllocatorType>::Destroy();
            if (azrtti_typeid<AllocatorType>() != azrtti_typeid<AZ::SystemAllocator>()) // simplifies instead of template specialization
            {
                // Other allocators need the SystemAllocator in order to work
                AZ::AllocatorInstance<AZ::SystemAllocator>::Destroy();
            }
            AZ::Debug::DrillerManager::Destroy(m_drillerManager);

            m_busRedirector.BusDisconnect();
            EXPECT_EQ(m_leakExpected, m_leakDetected);

            if (m_leakExpected)
            {
                AZ_TEST_STOP_ASSERTTEST(1);
            }
        }

        void SetLeakExpected() { AZ_TEST_START_ASSERTTEST; m_leakExpected = true; }

        // Dummy test class that uses AllocatorType
        class ThisAllocatorTestClass
        {
        public:
            AZ_CLASS_ALLOCATOR(ThisAllocatorTestClass, AllocatorType, 0);
        };

    private:
        class BusRedirector
            : public UnitTest::TraceBusRedirector
        {
        public:
            BusRedirector(bool& leakDetected)
                : m_leakDetected(leakDetected)
            {}

            bool OnPreError(const char* window, const char* file, int line, const char* func, const char* message) override
            {
                AZ_UNUSED(file);
                AZ_UNUSED(line);
                AZ_UNUSED(func);

                if (AZStd::string_view(window) == "Memory"
                    && AZStd::string_view(message) == "We still have 1 allocations on record! They must be freed prior to destroy!")
                {
                    // Leak detected, flag it so we validate on tear down that this happened. We also will 
                    // mark this test since it will assert
                    m_leakDetected = true;
                    return true;
                }
                return false;
            }

            bool OnPrintf(const char* window, const char* message) override
            {
                AZ_UNUSED(window);
                AZ_UNUSED(message);
                // Do not print the error message twice. The error message will already be printed by the TraceBusRedirector
                // in UnitTest.h. Here we override it to prevent it from printing twice.
                return true;
            }

        private:
            bool& m_leakDetected;
        };

        BusRedirector m_busRedirector;
        AZ::Debug::DrillerManager* m_drillerManager = nullptr;
        bool m_leakDetected = false;
        bool m_leakExpected = false;
    };

    using AllocatorTypes = ::testing::Types<
        AZ::SystemAllocator,
        AZ::PoolAllocator,
        AZ::ThreadPoolAllocator
    >;
    TYPED_TEST_CASE(AllocatorTypeLeakDetectionTest, AllocatorTypes);

    TYPED_TEST(AllocatorTypeLeakDetectionTest, Leak)
    {
        TestFixture::SetLeakExpected();

        typename TestFixture::ThisAllocatorTestClass* leakyObject = aznew typename TestFixture::ThisAllocatorTestClass();
        AZ_UNUSED(leakyObject);
    }

    TYPED_TEST(AllocatorTypeLeakDetectionTest, NoLeak)
    {
        typename TestFixture::ThisAllocatorTestClass* leakyObject = aznew typename TestFixture::ThisAllocatorTestClass();
        delete leakyObject;
    }
}


