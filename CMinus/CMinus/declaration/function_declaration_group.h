#pragma once

#include "function_declaration.h"

namespace cminus::declaration{
	class function_group : public function_group_base{
	public:
		function_group(std::string name, logic::naming::parent *parent);

		virtual ~function_group();

		virtual const logic::attributes::collection &get_attributes() const override;

		virtual void add(std::shared_ptr<function_base> value) override;

		virtual void replace(function_base &existing_entry, std::shared_ptr<function_base> new_entry) override;

		virtual function_base *find(logic::runtime &runtime, const function_base &tmpl) const override;

		virtual function_base *get_highest_ranked(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args, std::size_t *count) const override;

		virtual std::shared_ptr<memory::reference> call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

	protected:
		cminus::logic::attributes::collection attributes_;
		std::unordered_map<function_base *, std::shared_ptr<function_base>> list_;
	};
}
