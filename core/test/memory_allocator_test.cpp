#include <gtest/gtest.h>

#include "runtime/memory_allocator.h"

#include "testutils/prepare_loggers.h"

namespace plc::core::ut {

class MemoryAllocatorTest : public testing::Test {
public:
    void SetUp() override {
        prepareLoggers();
        m_memory_allocator = std::make_shared<plc::core::runtime::MemoryAllocator>(m_heap_base, [](size_t) {
            //skip
        });
    }

protected:
    const int m_heap_base = sizeof(size_t);
    std::shared_ptr<plc::core::runtime::MemoryAllocator> m_memory_allocator;
};

TEST_F(MemoryAllocatorTest, shouldAllocateAndDeallocateMemory) {
    int size = 16;
    auto ptr = m_memory_allocator->allocate(size);
    EXPECT_EQ(m_heap_base, ptr);

    auto deallocated = m_memory_allocator->deallocate(ptr);
    EXPECT_EQ(size, deallocated);
}

TEST_F(MemoryAllocatorTest, shouldAllocateAndDeallocateMemoryWithAllignment) {
    int size = 10;
    int size_after_alignment = sizeof(size_t) * 2; //16
    auto ptr = m_memory_allocator->allocate(size);
    EXPECT_EQ(m_heap_base, ptr);

    auto deallocated = m_memory_allocator->deallocate(ptr);
    EXPECT_EQ(size_after_alignment, deallocated);
}

} //namespace plc::core::ut
