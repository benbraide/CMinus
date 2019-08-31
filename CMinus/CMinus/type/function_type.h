#pragma once

#include "../logic/runtime.h"

namespace cminus::type{
	class function : public object{
	public:
		struct type_info{
			std::shared_ptr<object> value;
			std::vector<std::shared_ptr<logic::attributes::object>> attributes;
		};

		function(const type_info &return_type, const std::vector<type_info> &parameter_types);

		function(const type_info &return_type, std::vector<type_info> &&parameter_types);

		virtual ~function();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual void print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const override;

		virtual std::size_t get_size() const override;

		virtual score_result_type get_score(logic::runtime &runtime, const object &target, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> get_default_value(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, cast_type type) const override;

		virtual std::shared_ptr<evaluator::object> get_evaluator(logic::runtime &runtime) const override;

		virtual score_result_type get_params_score(logic::runtime &runtime, const object &target, bool is_ref) const;

		virtual const type_info &get_return_type() const;

		virtual const std::vector<type_info> &get_parameter_types() const;

		virtual void traverse_parameter_types(const std::function<void(const type_info &)> &callback) const;

		static std::shared_ptr<logic::attributes::object> find_attribute(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, const std::string &name);

		static bool has_attribute(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, const std::string &name);

		static bool has_attribute(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, std::shared_ptr<logic::naming::object> name);

	protected:
		virtual score_result_type get_exact_score_(logic::runtime &runtime, const type_info &left, const type_info &right) const;

		virtual void print_(logic::runtime &runtime, const type_info &info) const;

		type_info return_type_;
		std::vector<type_info> parameter_types_;
	};
}
