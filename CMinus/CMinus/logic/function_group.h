#pragma once

#include "function_object.h"

namespace cminus::logic{
	class function_group : public naming::single, public function_group_base{
	public:
		function_group(std::string name, naming::parent *parent);

		virtual ~function_group();

		virtual void add(std::shared_ptr<function_object> value) override;

		virtual void replace(function_object &existing_entry, std::shared_ptr<function_object> new_entry) override;

		virtual std::shared_ptr<function_object> find(logic::runtime &runtime, const type::object &type) const override;

		virtual std::shared_ptr<function_object> get_highest_ranked(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

		virtual std::shared_ptr<memory::reference> call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

	protected:
		std::unordered_map<function_object *, std::shared_ptr<function_object>> list_;
	};
}
