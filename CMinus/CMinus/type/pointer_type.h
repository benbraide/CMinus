#pragma once

#include "../logic/runtime.h"

namespace cminus::type{
	class raw_pointer : public object{
	public:
		explicit raw_pointer(object *raw_base_type);

		virtual ~raw_pointer();

		virtual bool converts_auto(const object &target) const override;

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual void print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const override;

		virtual std::size_t get_size() const override;

		virtual bool is_exact(logic::runtime &runtime, const type::object &target) const override;

		virtual score_result_type get_score(logic::runtime &runtime, const object &target, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> get_default_value(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, cast_type type) const override;

		virtual std::shared_ptr<evaluator::object> get_evaluator(logic::runtime &runtime) const override;

		virtual object *get_raw_base_type() const;

	protected:
		object *raw_base_type_;
	};

	class pointer : public raw_pointer{
	public:
		explicit pointer(std::shared_ptr<object> base_type);

		virtual ~pointer();

		virtual std::shared_ptr<object> get_base_type() const;

	protected:
		std::shared_ptr<object> base_type_;
	};
}
