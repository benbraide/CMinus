#pragma once

#include "../logic/type_object.h"

#include "memory_object.h"

namespace cminus::memory{
	class reference{
	public:
		reference(memory::object &object, std::shared_ptr<logic::type::object> type, unsigned int attributes);

		virtual ~reference();

		virtual void write(const reference &source);

		virtual void write(std::size_t address, const logic::type::object &type);

		virtual std::shared_ptr<logic::type::object> get_type() const;

		virtual std::size_t get_address() const;

		virtual unsigned int get_attributes() const;

		static const unsigned int attribute_none				= (0u << 0x0000u);
		static const unsigned int attribute_constant			= (1u << 0x0000u);
		static const unsigned int attribute_lvalue				= (1u << 0x0001u);
		static const unsigned int attribute_uninitialized		= (1u << 0x0002u);

	protected:
		memory::object &object_;
		std::size_t address_ = 0u;
		std::shared_ptr<logic::type::object> type_;
		unsigned int attributes_;
	};

	template <class value_type>
	class reference_with_value : public reference{
	public:
		using m_value_type = value_type;

		reference_with_value(memory::object &object, std::shared_ptr<logic::type::object> type, unsigned int attributes, m_value_type value)
			: reference(object, type, attributes), value_(value){}

		virtual ~reference_with_value() = default;

		virtual m_value_type get_value() const{
			return value_;
		}

	protected:
		m_value_type value_;
	};
}
