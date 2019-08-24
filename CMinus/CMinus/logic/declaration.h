#pragma once

#include "runtime.h"

namespace cminus::logic{
	class declaration{
	public:
		declaration(const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization);

		virtual ~declaration();

		virtual void evaluate(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const;

		virtual void print(logic::runtime &runtime) const;

		virtual const std::vector<std::shared_ptr<attributes::object>> &get_attributes() const;

		virtual std::shared_ptr<type::object> get_type() const;

		virtual const std::string &get_name() const;

		virtual std::shared_ptr<node::object> get_initialization() const;

		virtual std::shared_ptr<memory::reference> allocate_memory(logic::runtime &runtime) const;

		virtual void initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<memory::reference> value) const;

	protected:
		virtual void print_attributes_(logic::runtime &runtime) const;

		virtual void print_type_(logic::runtime &runtime) const;

		virtual void print_name_(logic::runtime &runtime) const;

		virtual void print_initialization_(logic::runtime &runtime) const;

		std::vector<std::shared_ptr<attributes::object>> attributes_;
		std::shared_ptr<type::object> type_;
		std::string name_;
		std::shared_ptr<node::object> initialization_;
		mutable std::shared_ptr<memory::reference> static_value_;
	};
}
