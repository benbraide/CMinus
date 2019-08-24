#pragma once

#include "../logic/runtime.h"

namespace cminus::type{
	class variadic : public object{
	public:
		explicit variadic(std::shared_ptr<object> base_type);

		virtual ~variadic();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t compute_base_offset(const object &target) const override;

		virtual score_result_type get_score(logic::runtime &runtime, const object &target, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> get_default_value(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> convert_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> convert_value(logic::runtime &runtime, const std::byte *data, std::shared_ptr<object> target_type) const override;

		virtual std::shared_ptr<evaluator::object> get_evaluator(logic::runtime &runtime) const override;

		virtual std::shared_ptr<object> get_base_type() const;

	protected:
		std::shared_ptr<object> base_type_;
	};
}
