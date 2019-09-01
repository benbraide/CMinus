#pragma once

#include "../logic/specialized_storage.h"

#include "function_declaration_base.h"

namespace cminus::declaration{
	class function : public function_base{
	public:
		function(std::string name, logic::naming::parent *parent, const attribute_list_type &attributes, std::shared_ptr<variable> return_declaration, const std::vector<std::shared_ptr<variable>> &params, std::shared_ptr<node::object> body);

		virtual ~function();

		virtual bool is_exact(logic::runtime &runtime, const function_base &tmpl) const override;

		virtual type::object::score_result_type get_rank(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

		virtual void print(logic::runtime &runtime) const override;

		virtual void define(std::shared_ptr<node::object> body) override;

		virtual bool is_defined() const override;

		virtual void traverse_params(const std::function<void(std::shared_ptr<variable>)> &callback) const override;

		virtual std::shared_ptr<type::object> get_return_type() const override;

		virtual std::shared_ptr<node::object> get_body() const override;

		virtual std::size_t get_min_arg_count() const override;

		virtual std::size_t get_max_arg_count() const override;

	protected:
		virtual std::shared_ptr<memory::reference> call_(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

		virtual void compute_values_();

		virtual bool is_exact_(logic::runtime &runtime, std::shared_ptr<variable> first, std::shared_ptr<variable> second) const;

		virtual void print_attributes_(logic::runtime &runtime) const;

		virtual void print_name_(logic::runtime &runtime) const;

		virtual void print_params_(logic::runtime &runtime) const;

		virtual void print_body_(logic::runtime &runtime) const;

		virtual void copy_args_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual std::shared_ptr<memory::reference> copy_return_value_(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const;

		std::shared_ptr<variable> return_declaration_;
		std::vector<std::shared_ptr<variable>> params_;
		std::shared_ptr<node::object> body_;

		std::size_t min_arg_count_ = 0u;
		std::size_t max_arg_count_ = 0u;
	};
}
