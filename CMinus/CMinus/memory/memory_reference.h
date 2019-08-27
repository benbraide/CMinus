#pragma once

#include <list>
#include <vector>
#include <functional>

#include "../logic/attributes.h"
#include "../type/type_object.h"

#include "memory_object.h"

namespace cminus::memory{
	class reference{
	public:
		using attribute_list_type = std::vector<std::shared_ptr<logic::attributes::object>>;
		using optimised_attribute_list_type = std::unordered_map<logic::attributes::object *, std::shared_ptr<logic::attributes::object>>;

		reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		virtual ~reference();

		virtual std::shared_ptr<reference> clone(const attribute_list_type &attributes, bool inherit_attributes) const;

		virtual void set_type(std::shared_ptr<type::object> value);

		virtual std::shared_ptr<type::object> get_type() const;

		virtual void set_context(std::shared_ptr<reference> value);

		virtual std::shared_ptr<reference> get_context() const;

		virtual void add_attribute(std::shared_ptr<logic::attributes::object> value);

		virtual void remove_attribute(const std::string &name, bool global_only);

		virtual void remove_attribute(std::shared_ptr<logic::naming::object> name);

		virtual std::shared_ptr<logic::attributes::object> find_attribute(const std::string &name, bool global_only, bool include_context) const;

		virtual std::shared_ptr<logic::attributes::object> find_attribute(std::shared_ptr<logic::naming::object> name, bool include_context) const;

		virtual const optimised_attribute_list_type get_attributes() const;

		virtual void traverse_attributes(logic::runtime &runtime, const std::function<void(std::shared_ptr<logic::attributes::object>)> &callback, logic::attributes::object::stage_type stage, bool include_context) const;

		virtual bool is_lvalue() const;

		virtual bool is_ref() const;

		virtual bool is_nan() const;

		virtual void set_address(std::size_t value);

		virtual std::size_t get_address() const;

		virtual const std::byte *get_data(logic::runtime &runtime) const;

		virtual int compare(logic::runtime &runtime, const std::byte *buffer, std::size_t size) const;

		virtual int compare(logic::runtime &runtime, std::size_t buffer, std::size_t size) const;

		virtual int compare(logic::runtime &runtime, const reference &buffer, std::size_t size) const;

		virtual std::size_t read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const;

		virtual std::size_t read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const;

		virtual std::size_t read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const;

		virtual std::size_t read(logic::runtime &runtime, reference &buffer, std::size_t size) const;

		virtual std::size_t write(logic::runtime &runtime, const std::byte *buffer, std::size_t size);

		virtual std::size_t write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size);

		virtual std::size_t write(logic::runtime &runtime, std::size_t buffer, std::size_t size);

		virtual std::size_t write(logic::runtime &runtime, const reference &buffer, std::size_t size);

		virtual std::size_t write(logic::runtime &runtime, managed_object &object);

		virtual std::size_t set(logic::runtime &runtime, std::byte value, std::size_t size);

		template <typename target_type>
		bool can_read_scalar() const{
			return (sizeof(target_type) <= type_->get_size());
		}

		template <typename target_type>
		target_type read_scalar(logic::runtime &runtime) const{
			auto buffer = target_type();
			if (read(runtime, reinterpret_cast<std::byte *>(&buffer), sizeof(target_type)) != sizeof(target_type))
				throw exception(error_code::access_protected, get_address());
			return buffer;
		}

		template <typename target_type>
		std::size_t read_buffer(logic::runtime &runtime, target_type *buffer, std::size_t size) const{
			return read(runtime, reinterpret_cast<std::byte *>(buffer), (sizeof(target_type) * size));
		}

		template <typename target_type>
		bool write_scalar(logic::runtime &runtime, target_type buffer){
			return (write(runtime, reinterpret_cast<const std::byte *>(&buffer), sizeof(target_type)) == sizeof(target_type));
		}

		template <typename target_type>
		std::size_t write_buffer(logic::runtime &runtime, const target_type *buffer, std::size_t size){
			return write(runtime, reinterpret_cast<const std::byte *>(buffer), (sizeof(target_type) * size));
		}

		static void call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args);

		static void call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context, std::shared_ptr<memory::reference> target);

	protected:
		virtual std::shared_ptr<reference> clone_(const attribute_list_type &attributes) const = 0;

		std::shared_ptr<type::object> type_;
		optimised_attribute_list_type attributes_;
		std::shared_ptr<reference> context_;
	};

	class hard_reference : public reference{
	public:
		hard_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		hard_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, std::shared_ptr<reference> context);

		hard_reference(std::size_t address, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		hard_reference(std::size_t address, std::shared_ptr<type::object> type, std::shared_ptr<reference> context);

		hard_reference(std::shared_ptr<reference> base, std::shared_ptr<reference> context, std::size_t address_offset = 0u);

		virtual ~hard_reference();

		virtual bool is_ref() const override;

		virtual void set_address(std::size_t value) override;

		virtual std::size_t get_address() const override;

		virtual const std::byte *get_data(logic::runtime &runtime) const override;

		virtual std::size_t read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const override;

		virtual std::size_t read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const override;

		virtual std::size_t read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const override;

		virtual std::size_t read(logic::runtime &runtime, reference &buffer, std::size_t size) const override;

		virtual std::size_t write(logic::runtime &runtime, const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size) override;

		virtual std::size_t write(logic::runtime &runtime, std::size_t buffer, std::size_t size) override;

		virtual std::size_t write(logic::runtime &runtime, const reference &buffer, std::size_t size) override;

		virtual std::size_t write(logic::runtime &runtime, managed_object &object) override;

		virtual std::size_t set(logic::runtime &runtime, std::byte value, std::size_t size) override;

	protected:
		virtual std::shared_ptr<reference> clone_(const attribute_list_type &attributes) const override;

		std::size_t address_ = 0u;
		std::function<void()> deallocator_;
	};

	template <class value_type>
	class reference_with_value : public reference{
	public:
		using m_value_type = value_type;

		reference_with_value(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context, const m_value_type &value)
			: reference(type, attributes, context), value_(value){}

		reference_with_value(std::shared_ptr<type::object> type, std::shared_ptr<reference> context, const m_value_type &value)
			: reference_with_value(type, attribute_list_type{}, context, value){}

		reference_with_value(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context, m_value_type &&value)
			: reference(type, attributes, context), value_(std::move(value)){}

		reference_with_value(std::shared_ptr<type::object> type, std::shared_ptr<reference> context, m_value_type &&value)
			: reference_with_value(type, attribute_list_type{}, context, std::move(value)){}

		virtual ~reference_with_value() = default;

		virtual const std::byte *get_data(logic::runtime &runtime) const override{
			return reinterpret_cast<const std::byte *>(&value_);
		}

		virtual void set_value(const m_value_type &value){
			value_ = value;
		}

		virtual const m_value_type &get_value() const{
			return value_;
		}

	protected:
		virtual std::shared_ptr<reference> clone_(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes) const override{
			return std::make_shared<reference_with_value>(type_, attributes, context_, value_);
		}

		m_value_type value_;
	};
}
