#pragma once

#include "../type/variadic_type.h"

#include "variable_declaration.h"

namespace cminus::declaration{
	class function_base;
	class function_group;

	class function_group_base : public logic::naming::single{
	public:
		function_group_base(std::string name, logic::naming::parent *parent);

		virtual ~function_group_base();

		virtual void add(std::shared_ptr<function_base> value) = 0;

		virtual void replace(function_base &existing_entry, std::shared_ptr<function_base> new_entry) = 0;

		virtual std::shared_ptr<function_base> find(logic::runtime &runtime, const function_base &tmpl) const = 0;

		virtual std::shared_ptr<function_base> get_highest_ranked(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const = 0;

		virtual std::shared_ptr<memory::reference> call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const = 0;
	};

	class function_base : public logic::naming::single{
	public:
		using attribute_list_type = logic::attributes::collection::list_type;
		using optimised_attribute_list_type = logic::attributes::collection::optimised_list_type;

		function_base(std::string name, logic::naming::parent *parent, const attribute_list_type &attributes);

		virtual ~function_base();

		virtual std::shared_ptr<memory::reference> call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual const logic::attributes::collection &get_attributes() const;

		virtual logic::attributes::collection &get_attributes();

		virtual bool is_exact(logic::runtime &runtime, const function_base &tmpl) const = 0;

		virtual type::object::score_result_type get_rank(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const = 0;

		virtual void print(logic::runtime &runtime) const = 0;

		virtual void define(std::shared_ptr<node::object> body) = 0;

		virtual bool is_defined() const = 0;

		virtual void traverse_params(const std::function<void(std::shared_ptr<variable>)> &callback) const = 0;

		virtual std::shared_ptr<type::object> get_return_type() const = 0;

		virtual std::shared_ptr<node::object> get_body() const = 0;

		virtual std::size_t get_min_arg_count() const = 0;

		virtual std::size_t get_max_arg_count() const = 0;

	protected:
		friend class function_group;

		virtual std::shared_ptr<memory::reference> call_(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const = 0;

		logic::attributes::collection attributes_;
	};
}
