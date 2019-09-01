#pragma once

#include "../logic/runtime.h"

namespace cminus::declaration{
	class variable{
	public:
		using attribute_list_type = logic::attributes::collection::list_type;
		using optimised_attribute_list_type = logic::attributes::collection::optimised_list_type;

		variable(const attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization);

		variable(const optimised_attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization);

		variable(const logic::attributes::collection &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization);

		virtual ~variable();

		virtual void evaluate(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const;

		virtual std::shared_ptr<memory::reference> evaluate_class_member(logic::runtime &runtime, std::size_t relative_offset) const;

		virtual void print(logic::runtime &runtime) const;

		virtual const logic::attributes::collection &get_attributes() const;

		virtual logic::attributes::collection &get_attributes();

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

		logic::attributes::collection attributes_;
		std::shared_ptr<type::object> type_;
		std::string name_;
		std::shared_ptr<node::object> initialization_;
		mutable std::shared_ptr<memory::reference> static_value_;
	};

	class contructed_variable : public variable{
	public:
		contructed_variable(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization);

		virtual ~contructed_variable();

		virtual std::shared_ptr<memory::reference> allocate_memory(logic::runtime &runtime) const override;

		virtual void initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<memory::reference> value) const override;

	protected:
		virtual void print_initialization_(logic::runtime &runtime) const override;
	};

	class uniform_contructed_variable : public contructed_variable{
	public:
		uniform_contructed_variable(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization);

		virtual ~uniform_contructed_variable();

	protected:
		virtual void print_initialization_(logic::runtime &runtime) const override;
	};
}
