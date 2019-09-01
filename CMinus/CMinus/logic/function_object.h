#pragma once

#include "../type/function_type.h"
#include "../type/variadic_type.h"

#include "declaration.h"
#include "specialized_storage.h"

namespace cminus::logic{
	class function_group_base{
	public:
		virtual ~function_group_base() = default;

		virtual void add(std::shared_ptr<function_object> value) = 0;

		virtual void replace(function_object &existing_entry, std::shared_ptr<function_object> new_entry) = 0;

		virtual std::shared_ptr<function_object> find(logic::runtime &runtime, const type::object &type) const = 0;

		virtual std::shared_ptr<function_object> get_highest_ranked(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const = 0;

		virtual std::shared_ptr<memory::reference> call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const = 0;
	};

	class function_object : public naming::single, public function_group_base, public std::enable_shared_from_this<function_object>{
	public:
		function_object(std::string name, naming::parent *parent, const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<declaration> return_declaration, const std::vector<std::shared_ptr<declaration>> &params, std::shared_ptr<node::object> body);

		function_object(std::string name, naming::parent *parent, std::vector<std::shared_ptr<attributes::object>> &&attributes, std::shared_ptr<declaration> return_declaration, std::vector<std::shared_ptr<declaration>> &&params, std::shared_ptr<node::object> body);

		virtual ~function_object();

		virtual void add(std::shared_ptr<function_object> value) override;

		virtual void replace(function_object &existing_entry, std::shared_ptr<function_object> new_entry) override;

		virtual std::shared_ptr<function_object> find(logic::runtime &runtime, const type::object &type) const override;

		virtual std::shared_ptr<function_object> get_highest_ranked(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

		virtual std::shared_ptr<memory::reference> call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

		virtual type::object::score_result_type get_rank(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void print(logic::runtime &runtime) const;

		virtual void define(std::shared_ptr<node::object> body);

		virtual bool is_defined() const;

		virtual std::shared_ptr<type::object> get_computed_type() const;

		virtual const std::vector<std::shared_ptr<attributes::object>> &get_attributes() const;

		virtual std::shared_ptr<type::object> get_return_type() const;

		virtual const std::vector<std::shared_ptr<declaration>> &get_params() const;

		virtual std::shared_ptr<node::object> get_body() const;

		virtual std::size_t get_min_arg_count() const;

		virtual std::size_t get_max_arg_count() const;

	protected:
		friend class function_group;

		void compute_values_();

		virtual void print_attributes_(logic::runtime &runtime) const;

		virtual void print_name_(logic::runtime &runtime) const;

		virtual void print_params_(logic::runtime &runtime) const;

		virtual void print_body_(logic::runtime &runtime) const;

		virtual std::shared_ptr<memory::reference> call_(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void copy_args_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual std::shared_ptr<memory::reference> copy_return_value_(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const;

		std::shared_ptr<type::object> computed_type_;
		std::vector<std::shared_ptr<attributes::object>> attributes_;
		std::shared_ptr<declaration> return_declaration_;

		std::vector<std::shared_ptr<declaration>> params_;
		std::shared_ptr<node::object> body_;

		std::size_t min_arg_count_ = 0u;
		std::size_t max_arg_count_ = 0u;
	};
}
