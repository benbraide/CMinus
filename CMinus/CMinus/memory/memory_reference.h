#pragma once

#include <list>

#include "../logic/type_object.h"

#include "memory_object.h"

namespace cminus::memory{
	class reference{
	public:
		reference(std::shared_ptr<logic::type::object> type, unsigned int attributes);

		virtual ~reference();

		virtual void write(logic::runtime &runtime, std::shared_ptr<reference> source);

		virtual void write(logic::runtime &runtime, const std::byte *source, std::shared_ptr<logic::type::object> type);

		virtual std::shared_ptr<logic::type::object> get_type() const;

		virtual void set_attributes(unsigned int value);

		virtual unsigned int get_attributes() const;

		virtual std::size_t get_address() const;

		virtual const std::byte *get_data() const = 0;

		static const unsigned int attribute_none				= (0u << 0x0000u);
		static const unsigned int attribute_lvalue				= (1u << 0x0000u);
		static const unsigned int attribute_uninitialized		= (1u << 0x0001u);
		static const unsigned int attribute_nan					= (1u << 0x0002u);

	protected:
		std::shared_ptr<logic::type::object> type_;
		unsigned int attributes_;
	};

	class hard_reference : public reference{
	public:
		hard_reference(logic::runtime &runtime, std::shared_ptr<logic::type::object> type, unsigned int attributes);

		virtual ~hard_reference();

		virtual void write(logic::runtime &runtime, std::shared_ptr<reference> source) override;

		virtual void write(logic::runtime &runtime, const std::byte *source, std::shared_ptr<logic::type::object> type) override;

		virtual std::size_t get_address() const override;

		virtual const std::byte *get_data() const override;

	protected:
		memory::object &object_;
		std::size_t address_ = 0u;
	};

	template <class value_type>
	class reference_with_value : public reference{
	public:
		using m_value_type = value_type;

		reference_with_value(std::shared_ptr<logic::type::object> type, unsigned int attributes, const m_value_type &value)
			: reference(type, attributes), value_(value){
			attributes_ &= ~(attribute_uninitialized | attribute_lvalue);
		}

		virtual ~reference_with_value() = default;

		virtual const std::byte *get_data() const override{
			return reinterpret_cast<const std::byte *>(&value_);
		}

		virtual void set_value(const m_value_type &value){
			value_ = value;
		}

		virtual const m_value_type &get_value() const{
			return value_;
		}

	protected:
		m_value_type value_;
	};

	template <class value_type>
	class reference_with_list : public reference{
	public:
		using m_value_type = value_type;
		using m_list_type = std::list<m_value_type>;

		reference_with_list(std::shared_ptr<logic::type::object> type, unsigned int attributes)
			: reference(type, attributes){
			attributes_ &= ~attribute_uninitialized;
		}

		reference_with_list(std::shared_ptr<logic::type::object> type, unsigned int attributes, const m_value_type &item)
			: reference(type, attributes){
			attributes_ &= ~attribute_uninitialized;
			list_.push_back(item);
		}

		reference_with_list(std::shared_ptr<logic::type::object> type, unsigned int attributes, const m_list_type &list)
			: reference(type, attributes), list_(list){
			attributes_ &= ~attribute_uninitialized;
		}

		virtual ~reference_with_list() = default;

		virtual const std::byte *get_data() const override{
			return nullptr;
		}

		virtual void add_item(const m_value_type &item){
			list_.push_back(item);
		}

		virtual const m_list_type &get_list() const{
			return list_;
		}

		virtual m_list_type &get_list(){
			return list_;
		}

	protected:
		m_list_type list_;
	};
}
