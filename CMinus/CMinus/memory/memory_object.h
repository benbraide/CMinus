#pragma once

#include <list>
#include <atomic>
#include <shared_mutex>
#include <unordered_map>

#include "memory_block.h"

namespace cminus::memory{
	class object{
	public:
		std::size_t lock();

		std::size_t unlock();

		bool is_locked() const;

		std::shared_ptr<block> protect_next_block(std::size_t size);

		std::shared_ptr<block> allocate_block(std::size_t size, unsigned int attributes, std::size_t min_free_size = 0u);

		std::shared_ptr<block> reallocate_block(std::size_t address, std::size_t size);

		void deallocate_block(std::size_t address);

		std::size_t compare(int &result, std::size_t destination_address, const std::byte *buffer, std::size_t size) const;

		std::size_t compare(int &result, std::size_t source_address, std::size_t destination_address, std::size_t size) const;

		std::size_t read(std::size_t source_address, std::byte *buffer, std::size_t size) const;

		std::size_t read(std::size_t source_address, io::binary_writer &buffer, std::size_t size) const;

		std::size_t read(std::size_t source_address, std::size_t destination_address, std::size_t size) const;

		std::size_t write(std::size_t destination_address, const std::byte *buffer, std::size_t size);

		std::size_t write(std::size_t destination_address, const io::binary_reader &buffer, std::size_t size);

		std::size_t write(std::size_t source_address, std::size_t destination_address, std::size_t size);

		std::size_t write(std::size_t destination_address, managed_object &object);

		std::size_t set(std::size_t destination_address, std::byte value, std::size_t size);

		std::shared_ptr<block> get_block(std::size_t address) const;

		std::shared_ptr<block> get_next_block(std::size_t address) const;

		std::shared_ptr<block> find_block(std::size_t address) const;

		template <typename target_type>
		bool can_read_scalar(std::size_t address) const{
			std::shared_lock<std::shared_mutex> guard(lock_);
			return can_read_scalar_<target_type>(address);
		}

		template <typename target_type>
		target_type read_scalar(std::size_t address) const{
			auto buffer = target_type();
			if (read(address, reinterpret_cast<std::byte *>(&buffer), sizeof(target_type)) != sizeof(target_type))
				throw exception(error_code::access_protected, address);
			return buffer;
		}

		template <typename target_type>
		std::size_t read_buffer(std::size_t address, target_type *buffer, std::size_t size) const{
			return read(address, reinterpret_cast<std::byte *>(buffer), (sizeof(target_type) * size));
		}

		template <typename target_type>
		bool write_scalar(std::size_t address, target_type buffer){
			return (write(address, reinterpret_cast<const std::byte *>(&buffer), sizeof(target_type)) == sizeof(target_type));
		}

		template <typename target_type>
		std::size_t write_buffer(std::size_t address, const target_type *buffer, std::size_t size){
			return write(address, reinterpret_cast<const std::byte *>(buffer), (sizeof(target_type) * size));
		}

		static const unsigned int allocation_state_none					= (0u << 0x0000u);
		static const unsigned int allocation_state_use_free				= (1u << 0x0000u);
		static const unsigned int allocation_state_composite			= (1u << 0x0001u);

	private:
		std::shared_ptr<block> protect_next_block_(std::size_t size);

		std::shared_ptr<block> allocate_block_(std::size_t size, unsigned int attributes, std::size_t min_free_size);

		std::shared_ptr<block> reallocate_block_(std::size_t address, std::size_t size);

		void deallocate_block_(std::size_t address);

		std::size_t compare_(int &result, std::size_t destination_address, const std::byte *buffer, std::size_t size) const;

		std::size_t compare_(int &result, std::size_t source_address, std::size_t destination_address, std::size_t size) const;

		std::size_t read_(std::size_t source_address, std::byte *buffer, std::size_t size) const;

		std::size_t read_(std::size_t source_address, io::binary_writer &buffer, std::size_t size) const;

		std::size_t read_(std::size_t source_address, std::size_t destination_address, std::size_t size) const;

		std::size_t write_(std::size_t destination_address, const std::byte *buffer, std::size_t size);

		std::size_t write_(std::size_t destination_address, const io::binary_reader &buffer, std::size_t size);

		std::size_t write_(std::size_t source_address, std::size_t destination_address, std::size_t size);

		std::size_t write_(std::size_t destination_address, managed_object &object);

		std::size_t set_(std::size_t destination_address, std::byte value, std::size_t size);

		std::shared_ptr<block> get_block_(std::size_t address) const;

		std::shared_ptr<block> get_next_block_(std::size_t address) const;

		std::shared_ptr<block> find_block_(std::size_t address, std::list<std::shared_ptr<block>>::const_iterator *out_it = nullptr) const;

		template <typename target_type>
		bool can_read_scalar_(std::size_t address) const{
			auto block = find_block_(address);
			return (block != nullptr && block->can_read_scalar<target_type>());
		}

		std::list<std::shared_ptr<block>> blocks_;
		std::size_t next_address_ = 1u;

		std::atomic<std::size_t> lock_count_ = 0u;
		mutable std::shared_mutex lock_;
	};
}
