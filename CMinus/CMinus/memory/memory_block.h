#pragma once

#include <vector>
#include <memory>
#include <exception>

#include "../io/io_binary_reader.h"
#include "../io/io_binary_writer.h"

namespace cminus::memory{
	class object;

	class exclusive_block;
	class inclusive_block;

	class composite_block;

	enum class error_code{
		nil,
		out_of_address_space,
		allocation_failure,
		access_protected,
		write_protected,
		block_not_found,
		block_misaligned,
		block_not_resizable,
		invalid_size,
		incompatible_types,
		uninitialized_memory,
	};

	class exception : public std::exception{
	public:
		using base_type = std::exception;

		exception(error_code code, std::size_t address);

		error_code get_code() const;

		std::size_t get_address() const;

	private:
		error_code code_;
		std::size_t address_;
	};

	class managed_object{
	public:
		virtual ~managed_object() = default;

		virtual managed_object *clone() const = 0;
	};

	template <class value_type>
	class wrapped_managed_object : public managed_object{
	public:
		using m_value_type = value_type;

		explicit wrapped_managed_object(const m_value_type &value)
			: value_(value){}

		virtual ~wrapped_managed_object() = default;

		virtual managed_object *clone() const override{
			return new wrapped_managed_object(value_);
		}

		virtual const m_value_type &get_value() const{
			return value_;
		}

	protected:
		m_value_type value_;
	};

	template <class value_type>
	class wrapped_shared_managed_object : public wrapped_managed_object<std::shared_ptr<value_type>>{
	public:
		using base_type = wrapped_managed_object<std::shared_ptr<value_type>>;
		using m_value_type = typename base_type::value_type;

		explicit wrapped_shared_managed_object(const m_value_type &value)
			: base_type(value){}

		virtual ~wrapped_shared_managed_object() = default;
	};

	class block{
	public:
		block();

		block(std::size_t address, std::size_t size, unsigned int attributes = attribute_none);

		virtual ~block();

		virtual std::size_t get_address() const;

		virtual std::size_t get_size() const;

		virtual std::byte *get_data(std::size_t offset = 0u) const = 0;

		virtual void set_attributes(unsigned int value);

		virtual unsigned int get_attributes() const;

		virtual std::size_t convert_address_to_offset(std::size_t value) const;

		virtual std::size_t convert_offset_to_address(std::size_t value) const;

		virtual bool hit_test(std::size_t address) const;

		virtual bool hit_test(std::size_t offset, std::size_t size) const;

		virtual bool has_attributes(unsigned int value, bool all = true) const;

		virtual bool is_access_protected() const;

		virtual bool is_write_protected() const;

		virtual bool is_executable() const;

		virtual std::size_t compare(int &result, std::size_t offset, const std::byte *buffer, std::size_t size) const = 0;

		virtual std::size_t compare(int &result, std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset = 0u) const = 0;

		virtual std::size_t read(std::size_t offset, std::byte *buffer, std::size_t size) const = 0;

		virtual std::size_t read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const = 0;

		virtual std::size_t read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset = 0u) const = 0;

		virtual std::size_t write(std::size_t offset, const std::byte *buffer, std::size_t size) = 0;

		virtual std::size_t write(std::size_t offset, const io::binary_reader &buffer, std::size_t size) = 0;

		virtual std::size_t write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset = 0u) = 0;

		virtual std::size_t write(managed_object &object) = 0;

		virtual std::size_t set(std::size_t offset, std::byte value, std::size_t size) = 0;

		template <typename target_type>
		bool can_read_scalar(std::size_t offset) const{
			return hit_test(offset, sizeof(target_type));
		}

		template <typename target_type>
		target_type read_scalar(std::size_t offset) const{
			auto buffer = target_type();
			if (read(offset, reinterpret_cast<std::byte *>(&buffer), sizeof(target_type)) != sizeof(target_type))
				throw exception(error_code::access_protected, (get_address() + offset));
			return buffer;
		}

		template <typename target_type>
		std::size_t read_buffer(std::size_t offset, target_type *buffer, std::size_t size) const{
			return read(offset, reinterpret_cast<std::byte *>(buffer), (sizeof(target_type) * size));
		}

		template <typename target_type>
		bool write_scalar(std::size_t offset, target_type buffer){
			return (write(offset, reinterpret_cast<const std::byte *>(&buffer), sizeof(target_type)) == sizeof(target_type));
		}

		template <typename target_type>
		std::size_t write_buffer(std::size_t offset, const target_type *buffer, std::size_t size){
			return write(offset, reinterpret_cast<const std::byte *>(buffer), (sizeof(target_type) * size));
		}

		static const unsigned int attribute_none					= (0u << 0x0000u);
		static const unsigned int attribute_access_protected		= (1u << 0x0000u);
		static const unsigned int attribute_write_protected			= (1u << 0x0001u);
		static const unsigned int attribute_executable				= (1u << 0x0002u);
		static const unsigned int attribute_has_managed_object		= (1u << 0x0003u);
		static const unsigned int attribute_is_heap					= (1u << 0x0004u);

	protected:
		friend class object;
		friend class exclusive_block;
		friend class inclusive_block;

		std::size_t address_ = 0u;
		std::size_t size_ = 0u;

		unsigned int attributes_ = 0u;
	};

	class protected_block : public block{
	public:
		protected_block();

		protected_block(std::size_t address, std::size_t size);

		virtual ~protected_block();

		virtual std::byte *get_data(std::size_t offset = 0u) const override;

		virtual std::size_t compare(int &result, std::size_t offset, const std::byte *buffer, std::size_t size) const override;

		virtual std::size_t compare(int &result, std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset = 0u) const override;

		virtual std::size_t read(std::size_t offset, std::byte *buffer, std::size_t size) const override;

		virtual std::size_t read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const override;

		virtual std::size_t read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset = 0u) const override;

		virtual std::size_t write(std::size_t offset, const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write(std::size_t offset, const io::binary_reader &buffer, std::size_t size) override;

		virtual std::size_t write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset = 0u) override;

		virtual std::size_t write(managed_object &object) override;

		virtual std::size_t set(std::size_t offset, std::byte value, std::size_t size) override;

	protected:
		friend class object;
	};

	class free_block : public protected_block{
	public:
		free_block();

		free_block(std::size_t address, std::size_t size);

		virtual ~free_block();

		virtual std::size_t get_address() const override;

	protected:
		friend class object;
	};

	class offset_block : public block{
	public:
		offset_block(std::shared_ptr<block> target, std::size_t offset);

		virtual ~offset_block();

		virtual std::byte *get_data(std::size_t offset = 0u) const override;

		virtual unsigned int get_attributes() const override;

		virtual std::size_t compare(int &result, std::size_t offset, const std::byte *buffer, std::size_t size) const override;

		virtual std::size_t compare(int &result, std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset = 0u) const override;

		virtual std::size_t read(std::size_t offset, std::byte *buffer, std::size_t size) const override;

		virtual std::size_t read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const override;

		virtual std::size_t read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset = 0u) const override;

		virtual std::size_t write(std::size_t offset, const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write(std::size_t offset, const io::binary_reader &buffer, std::size_t size) override;

		virtual std::size_t write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset = 0u) override;

		virtual std::size_t write(managed_object &object) override;

		virtual std::size_t set(std::size_t offset, std::byte value, std::size_t size) override;

		virtual std::shared_ptr<block> get_target() const;

		virtual std::size_t get_offset() const;

	protected:
		friend class object;

		std::shared_ptr<block> target_;
		std::size_t offset_;
	};

	class exclusive_block : public block{
	public:
		exclusive_block();

		exclusive_block(std::size_t address, std::size_t size, unsigned int attributes = attribute_none);

		virtual ~exclusive_block();

		virtual std::byte *get_data(std::size_t offset = 0u) const override;

		virtual std::size_t compare(int &result, std::size_t offset, const std::byte *buffer, std::size_t size) const override;

		virtual std::size_t compare(int &result, std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset = 0u) const override;

		virtual std::size_t read(std::size_t offset, std::byte *buffer, std::size_t size) const override;

		virtual std::size_t read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const override;

		virtual std::size_t read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset = 0u) const override;

		virtual std::size_t write(std::size_t offset, const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write(std::size_t offset, const io::binary_reader &buffer, std::size_t size) override;

		virtual std::size_t write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset = 0u) override;

		virtual std::size_t write(managed_object &object) override;

		virtual std::size_t set(std::size_t offset, std::byte value, std::size_t size) override;

	private:
		friend class object;

		std::unique_ptr<std::byte[]> data_;
	};
}
