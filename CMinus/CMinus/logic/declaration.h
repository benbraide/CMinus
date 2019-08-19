#pragma once

#include "../node/node_object.h"

namespace cminus::logic{
	class declaration{
	public:
		declaration(unsigned int attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization);

		virtual ~declaration();

		virtual void evaluate(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const;

		virtual void print(logic::runtime &runtime) const;

		virtual unsigned int get_attributes() const;

		virtual std::shared_ptr<type::object> get_type() const;

		virtual const std::string &get_name() const;

		virtual std::shared_ptr<node::object> get_initialization() const;

		static const unsigned int attribute_none				= (0u << 0x0000u);
		static const unsigned int attribute_static				= (1u << 0x0000u);
		static const unsigned int attribute_thread_local		= (1u << 0x0001u);

	protected:
		virtual std::shared_ptr<memory::reference> allocate_memory_(logic::runtime &runtime) const;

		virtual void evaluate_initialization_(logic::runtime &runtime, memory::reference &reference) const;

		virtual void print_attributes_(logic::runtime &runtime) const;

		virtual void print_type_(logic::runtime &runtime) const;

		virtual void print_name_(logic::runtime &runtime) const;

		virtual void print_initialization_(logic::runtime &runtime) const;

		unsigned int attributes_;
		std::shared_ptr<type::object> type_;
		std::string name_;
		std::shared_ptr<node::object> initialization_;
	};
}
