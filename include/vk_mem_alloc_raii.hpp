#pragma once

#include <vk_mem_alloc.hpp>

namespace VMA_HPP_NAMESPACE::raii {
    class Allocation {
    public:
        Allocation(const VMA_HPP_NAMESPACE::Allocator& allocator, VMA_HPP_NAMESPACE::Allocation allocation) : m_allocator(allocator), m_allocation(allocation) {}

        Allocation(std::nullptr_t) {}

        ~Allocation() { clear(); }

        Allocation() = delete;
        Allocation(const Allocation&) = delete;
        Allocation(Allocation&& rhs) noexcept : m_allocator(std::exchange(rhs.m_allocator, {})), m_allocation(std::exchange(rhs.m_allocation, {})) {}
        Allocation& operator=(const Allocation&) = delete;
        Allocation& operator=(Allocation&& rhs) noexcept {
            if (this != &rhs) {
                clear();
                m_allocator = std::exchange(rhs.m_allocator, {});
                m_allocation = std::exchange(rhs.m_allocation, {});
            }
            return *this;
        }

        const VMA_HPP_NAMESPACE::Allocation& operator*() const noexcept { return m_allocation; }

        void clear() noexcept {
            if (m_allocation) {
                m_allocator.freeMemory(m_allocation);
            }
            m_allocator = nullptr;
            m_allocation = nullptr;
        }

        [[nodiscard]] VMA_HPP_NAMESPACE::Allocator getAllocator() const { return m_allocator; }

        void swap(Allocation& rhs) noexcept {
            std::swap(m_allocator, rhs.m_allocator);
            std::swap(m_allocation, rhs.m_allocation);
        }

        [[nodiscard]] void* mapMemory() const { return m_allocator.mapMemory(m_allocation); }
        void unmapMemory() const { m_allocator.unmapMemory(m_allocation); }
        void flush(vk::DeviceSize size, vk::DeviceSize offset) const { m_allocator.flushAllocation(m_allocation, size, offset); }

    private:
        VMA_HPP_NAMESPACE::Allocator m_allocator;
        VMA_HPP_NAMESPACE::Allocation m_allocation;
    };

    class Allocator {
    public:
        Allocator(const AllocatorCreateInfo& createInfo, const vk::raii::Device& device) {
            vk::Result result = VMA_HPP_NAMESPACE::createAllocator(&createInfo, &m_allocator);
            if (result != vk::Result::eSuccess) {
                vk::throwResultException(result, "vmaCreateAllocator");
            }
            m_device = &device;
        }

        Allocator(std::nullptr_t) {}

        ~Allocator() { clear(); }

        Allocator() = delete;
        Allocator(const Allocator&) = delete;
        Allocator(Allocator&& rhs) noexcept : m_allocator(std::exchange(rhs.m_allocator, {})), m_device(std::exchange(rhs.m_device, {})) {}
        Allocator& operator=(const Allocator&) = delete;
        Allocator& operator=(Allocator&& rhs) noexcept {
            if (this != &rhs) {
                clear();
                m_allocator = std::exchange(rhs.m_allocator, {});
                m_device = std::exchange(rhs.m_device, {});
            }
            return *this;
        }

        VMA_HPP_NAMESPACE::Allocator const& operator*() const VULKAN_HPP_NOEXCEPT { return m_allocator; }

        void clear() noexcept {
            if (m_allocator) {
                vmaDestroyAllocator(static_cast<VmaAllocator>(m_allocator));
            }
            m_allocator = nullptr;
            m_device = nullptr;
        }

        void swap(Allocator& rhs) noexcept {
            std::swap(m_allocator, rhs.m_allocator);
            std::swap(m_device, rhs.m_device);
        }

        std::pair<vk::raii::Buffer, VMA_HPP_NAMESPACE::raii::Allocation> createBuffer(const vk::BufferCreateInfo& bufferCreateInfo,
                                                                        const VMA_HPP_NAMESPACE::AllocationCreateInfo& allocationCreateInfo,
                                                                        vk::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr) const {
            auto [buffer, allocation] = m_allocator.createBuffer(bufferCreateInfo, allocationCreateInfo, allocationInfo);
            return {vk::raii::Buffer(*m_device, static_cast<VkBuffer>(buffer)), VMA_HPP_NAMESPACE::raii::Allocation(**this, allocation)};
        }

        std::pair<vk::raii::Image, VMA_HPP_NAMESPACE::raii::Allocation> createImage(const vk::ImageCreateInfo& imageCreateInfo,
                                                                      const VMA_HPP_NAMESPACE::AllocationCreateInfo& allocationCreateInfo,
                                                                      vk::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr) const {
            auto [image, allocation] = m_allocator.createImage(imageCreateInfo, allocationCreateInfo, allocationInfo);
            return {vk::raii::Image(*m_device, static_cast<VkImage>(image)), VMA_HPP_NAMESPACE::raii::Allocation(**this, allocation)};
        }

    private:
        VMA_HPP_NAMESPACE::Allocator m_allocator = {};
        const vk::raii::Device* m_device = nullptr;
    };
} // namespace VMA_HPP_NAMESPACE::raii
