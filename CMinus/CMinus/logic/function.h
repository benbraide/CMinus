#pragma once

#include "../type/function_type.h"
#include "../type/variadic_type.h"

#include "declaration.h"

namespace cminus::logic{
	class function_object{
	public:
		enum class return_interrupt{
			nil,
		};

		function_object(std::shared_ptr<type::object> owner_type, const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<declaration> return_declaration, std::string name, const std::vector<std::shared_ptr<declaration>> &params, std::shared_ptr<node::object> body);

		function_object(std::shared_ptr<type::object> owner_type, std::vector<std::shared_ptr<attributes::object>> &&attributes, std::shared_ptr<declaration> return_declaration, std::string name, std::vector<std::shared_ptr<declaration>> &&params, std::shared_ptr<node::object> body);

		virtual ~function_object();

		virtual type::object::score_result_type get_rank(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual std::shared_ptr<memory::reference> call(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void print(logic::runtime &runtime) const;

		virtual void define(std::shared_ptr<node::object> body);

		virtual bool is_defined() const;

		virtual std::shared_ptr<type::object> get_computed_type() const;

		virtual std::shared_ptr<type::object> get_owner_type() const;

		virtual const std::vector<std::shared_ptr<attributes::object>> &get_attributes() const;

		virtual std::shared_ptr<type::object> get_return_type() const;

		virtual const std::string &get_name() const;

		virtual const std::vector<std::shared_ptr<declaration>> &get_params() const;

		virtual std::shared_ptr<node::object> get_body() const;

		virtual std::size_t get_min_arg_count() const;

		virtual std::size_t get_max_arg_count() const;

	protected:
		void compute_values_();

		virtual void print_attributes_(logic::runtime &runtime) const;

		virtual void print_name_(logic::runtime &runtime) const;

		virtual void print_params_(logic::runtime &runtime) const;

		virtual void print_body_(logic::runtime &runtime) const;

		virtual std::shared_ptr<memory::reference> call_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual std::shared_ptr<memory::reference> get_context_() const;

		std::shared_ptr<type::object> computed_type_;
		std::shared_ptr<type::object> owner_type_;

		std::vector<std::shared_ptr<attributes::object>> attributes_;
		std::shared_ptr<declaration> return_declaration_;
		std::string name_;

		std::vector<std::shared_ptr<declaration>> params_;
		std::shared_ptr<node::object> body_;

		std::size_t min_arg_count_ = 0u;
		std::size_t max_arg_count_ = 0u;
	};
}
