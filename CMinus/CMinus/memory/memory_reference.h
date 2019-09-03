#pragma once

#include <list>
#include <vector>
#include <functional>

#include "../logic/attributes.h"
#include "../type/type_object.h"

#include "memory_object.h"

namespace cminus::declaration{
	class function_group_base;
}

namespace cminus::memory{
	class reference{
	public:
		using attribute_list_type = logic::attributes::collection::list_type;
		using optimised_attribute_list_type = logic::attributes::collection::optimised_list_type;

		reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		reference(std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context);

		reference(std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context);

		virtual ~reference();

		virtual reference *get_non_raw() const;

		virtual void set_type(std::shared_ptr<type::object> value);

		virtual std::shared_ptr<type::object> get_type() const;

		virtual bool is_nan() const;

		virtual void set_context(std::shared_ptr<reference> value);

		virtual std::shared_ptr<reference> bound_context(logic::runtime &runtime, std::shared_ptr<reference> value, std::size_t offset) const;

		virtual std::shared_ptr<reference> get_context() const;

		virtual void add_attribute(std::shared_ptr<logic::attributes::object> value);

		virtual void remove_attribute(const std::string &name, bool global_only);

		virtual void remove_attribute(const logic::naming::object &name);

		virtual std::shared_ptr<logic::attributes::object> find_attribute(const std::string &name, bool global_only, bool include_context) const;

		virtual std::shared_ptr<logic::attributes::object> find_attribute(const logic::naming::object &name, bool include_context) const;

		virtual bool has_attribute(const std::string &name, bool global_only, bool include_context) const;

		virtual bool has_attribute(const logic::naming::object &name, bool include_context) const;

		virtual const logic::attributes::collection &get_attributes() const;

		virtual void traverse_attributes(logic::runtime &runtime, const std::function<void(std::shared_ptr<logic::attributes::object>)> &callback, logic::attributes::object::stage_type stage, bool include_context) const;

		virtual void call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context) const;

		virtual std::shared_ptr<reference> apply_offset(std::size_t value) const = 0;

		virtual bool is_lvalue() const = 0;

		virtual std::size_t get_address() const = 0;

		virtual std::byte *get_data(logic::runtime &runtime) const = 0;

		virtual std::size_t read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const = 0;

		virtual std::size_t read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const = 0;

		virtual std::size_t read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const = 0;

		virtual std::size_t read(logic::runtime &runtime, reference &buffer, std::size_t size) const = 0;

		virtual std::size_t write(logic::runtime &runtime, const std::byte *buffer, std::size_t size) = 0;

		virtual std::size_t write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size) = 0;

		virtual std::size_t write(logic::runtime &runtime, std::size_t buffer, std::size_t size) = 0;

		virtual std::size_t write(logic::runtime &runtime, const reference &buffer, std::size_t size) = 0;

		virtual std::size_t write(logic::runtime &runtime, managed_object &object) = 0;

		virtual std::size_t set(logic::runtime &runtime, std::byte value, std::size_t size) = 0;

		virtual int compare(logic::runtime &runtime, const std::byte *buffer, std::size_t size) const;

		virtual int compare(logic::runtime &runtime, std::size_t buffer, std::size_t size) const;

		virtual int compare(logic::runtime &runtime, const reference &buffer, std::size_t size) const;

		template <typename target_type>
		bool can_read_scalar() const{
			return (sizeof(target_type) <= get_type()->get_size());
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

	protected:
		std::shared_ptr<type::object> type_;
		logic::attributes::collection attributes_;
		std::shared_ptr<reference> context_;
		std::size_t size_ = 0u;
		std::function<void()> deallocator_;
	};

	class placeholder_reference : public reference{
	public:
		placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type, const attribute_list_type &attributes);

		placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes);

		placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes);

		placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type);

		virtual ~placeholder_reference();

		virtual std::shared_ptr<reference> bound_context(logic::runtime &runtime, std::shared_ptr<reference> value, std::size_t offset) const override;

		virtual std::shared_ptr<reference> apply_offset(std::size_t value) const override;

		virtual bool is_lvalue() const override;

		virtual std::size_t get_address() const override;

		virtual std::byte *get_data(logic::runtime &runtime) const override;

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

		virtual void set_relative_offset(std::size_t value);

		virtual std::size_t get_relative_offset() const;

	protected:
		std::size_t relative_offset_;
	};

	class raw_reference : public reference{
	public:
		explicit raw_reference(reference *target);

		virtual ~raw_reference();

		virtual reference *get_non_raw() const override;

		virtual void set_type(std::shared_ptr<type::object> value) override;

		virtual std::shared_ptr<type::object> get_type() const override;

		virtual bool is_nan() const override;

		virtual void set_context(std::shared_ptr<reference> value) override;

		virtual std::shared_ptr<reference> bound_context(logic::runtime &runtime, std::shared_ptr<reference> value, std::size_t offset) const override;

		virtual std::shared_ptr<reference> get_context() const override;

		virtual void add_attribute(std::shared_ptr<logic::attributes::object> value) override;

		virtual void remove_attribute(const std::string &name, bool global_only) override;

		virtual void remove_attribute(const logic::naming::object &name) override;

		virtual std::shared_ptr<logic::attributes::object> find_attribute(const std::string &name, bool global_only, bool include_context) const override;

		virtual std::shared_ptr<logic::attributes::object> find_attribute(const logic::naming::object &name, bool include_context) const override;

		virtual bool has_attribute(const std::string &name, bool global_only, bool include_context) const override;

		virtual bool has_attribute(const logic::naming::object &name, bool include_context) const override;

		virtual const logic::attributes::collection &get_attributes() const override;

		virtual void traverse_attributes(logic::runtime &runtime, const std::function<void(std::shared_ptr<logic::attributes::object>)> &callback, logic::attributes::object::stage_type stage, bool include_context) const override;

		virtual void call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context) const;

		virtual std::shared_ptr<reference> apply_offset(std::size_t value) const override;

		virtual bool is_lvalue() const override;

		virtual std::size_t get_address() const override;

		virtual std::byte *get_data(logic::runtime &runtime) const override;

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

		virtual int compare(logic::runtime &runtime, const std::byte *buffer, std::size_t size) const override;

		virtual int compare(logic::runtime &runtime, std::size_t buffer, std::size_t size) const override;

		virtual int compare(logic::runtime &runtime, const reference &buffer, std::size_t size) const override;

		virtual reference *get_target() const;

	protected:
		reference *target_;
	};

	class lval_reference : public reference{
	public:
		lval_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		lval_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context);

		lval_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context);

		lval_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, std::shared_ptr<reference> context);

		lval_reference(std::size_t address, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		lval_reference(std::size_t address, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context);

		lval_reference(std::size_t address, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context);

		lval_reference(std::size_t address, std::shared_ptr<type::object> type, std::shared_ptr<reference> context);

		virtual ~lval_reference();

		virtual std::shared_ptr<reference> apply_offset(std::size_t value) const override;

		virtual bool is_lvalue() const override;

		virtual std::size_t get_address() const override;

		virtual std::byte *get_data(logic::runtime &runtime) const override;

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
		void allocate_memory_(logic::runtime &runtime, std::size_t size);

		std::size_t address_ = 0u;
	};

	class ref_reference : public lval_reference{
	public:
		ref_reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		ref_reference(std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context);

		ref_reference(std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context);

		ref_reference(std::shared_ptr<type::object> type, std::shared_ptr<reference> context);

		virtual ~ref_reference();

		virtual void write_address(std::size_t value);
	};

	class function_reference : public lval_reference{
	public:
		function_reference(logic::runtime &runtime, std::size_t address, declaration::function_group_base *value);

		function_reference(std::size_t address, std::shared_ptr<type::object> type, declaration::function_group_base *value, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		function_reference(std::size_t address, std::shared_ptr<type::object> type, declaration::function_group_base *value, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context);

		function_reference(std::size_t address, std::shared_ptr<type::object> type, declaration::function_group_base *value, const logic::attributes::collection &attributes, std::shared_ptr<reference> context);

		virtual ~function_reference();

		virtual std::shared_ptr<reference> bound_context(logic::runtime &runtime, std::shared_ptr<reference> value, std::size_t offset) const override;

		virtual std::shared_ptr<reference> apply_offset(std::size_t value) const override;

		virtual declaration::function_group_base *get_value() const;

	protected:
		declaration::function_group_base *value_;
	};

	class rval_reference : public reference{
	public:
		rval_reference(std::byte *data, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		rval_reference(std::byte *data, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context);

		rval_reference(std::byte *data, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context);

		rval_reference(std::byte *data, std::shared_ptr<type::object> type, std::shared_ptr<reference> context);

		virtual ~rval_reference();

		virtual std::shared_ptr<reference> apply_offset(std::size_t value) const override;

		virtual bool is_lvalue() const override;

		virtual std::size_t get_address() const override;

		virtual std::byte *get_data(logic::runtime &runtime) const override;

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
		std::byte *data_;
	};

	class data_reference : public rval_reference{
	public:
		data_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		data_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context);

		data_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context);

		data_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, std::shared_ptr<reference> context);

		data_reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context);

		data_reference(std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context);

		data_reference(std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context);

		data_reference(std::shared_ptr<type::object> type, std::shared_ptr<reference> context);

		virtual ~data_reference();

		virtual std::shared_ptr<reference> apply_offset(std::size_t value) const override;

	protected:
		std::unique_ptr<std::byte[]> value_;
	};

	template <class value_type>
	class scalar_reference : public rval_reference{
	public:
		using m_value_type = value_type;

		scalar_reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context, const m_value_type &value)
			: rval_reference(reinterpret_cast<std::byte *>(&value_), type, attributes, context), value_(value){}

		scalar_reference(std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context, const m_value_type &value)
			: rval_reference(reinterpret_cast<std::byte *>(&value_), type, attributes, context), value_(value){}

		scalar_reference(std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context, const m_value_type &value)
			: rval_reference(reinterpret_cast<std::byte *>(&value_), type, attributes, context), value_(value){}

		scalar_reference(std::shared_ptr<type::object> type, std::shared_ptr<reference> context, const m_value_type &value)
			: scalar_reference(type, attribute_list_type{}, context, value){}

		scalar_reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context, m_value_type &&value)
			: rval_reference(reinterpret_cast<std::byte *>(&value_), type, attributes, context), value_(std::move(value)){}

		scalar_reference(std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context, m_value_type &&value)
			: rval_reference(reinterpret_cast<std::byte *>(&value_), type, attributes, context), value_(std::move(value)){}

		scalar_reference(std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context, m_value_type &&value)
			: rval_reference(reinterpret_cast<std::byte *>(&value_), type, attributes, context), value_(std::move(value)){}

		scalar_reference(std::shared_ptr<type::object> type, std::shared_ptr<reference> context, m_value_type &&value)
			: scalar_reference(type, attribute_list_type{}, context, std::move(value)){}

		virtual ~scalar_reference() = default;

		virtual std::shared_ptr<reference> apply_offset(std::size_t value) const override{
			return ((value == 0u) ? std::make_shared<scalar_reference>(type_, attributes_, context_, value_) : rval_reference::apply_offset(value));
		}

		virtual void set_value(const m_value_type &value){
			value_ = value;
		}

		virtual void set_value(m_value_type &&value){
			value_ = std::move(value);
		}

		virtual const m_value_type &get_value() const{
			return value_;
		}

	protected:
		m_value_type value_;
	};

	template <class value_type>
	class nan_scalar_reference : public scalar_reference<value_type>{
	public:
		using base_type = scalar_reference<value_type>;

		template <typename runtime_type, typename... args_types>
		explicit nan_scalar_reference(runtime_type &runtime, args_types &&... args)
			: base_type(std::forward<args_types>(args)...){
			base_type::template add_attribute(runtime.global_storage->find_attribute("#NaN#", false));
		}
	};
}
