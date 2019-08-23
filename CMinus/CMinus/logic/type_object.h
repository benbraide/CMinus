#pragma once

#include "naming.h"

namespace cminus::memory{
	class object;
	class reference;
}

namespace cminus::evaluator{
	class object;
}

namespace cminus::logic::type{
	class object{
	public:
		enum class score_result_type{
			nil,
			exact,
			assignable,
			ancestor,
			offspring,
			compatible,
			class_compatible,
			widened,
			shortened,
			too_widened,
			too_shortened,
		};

		virtual ~object();

		virtual void print(logic::runtime &runtime, bool is_qualified) const = 0;

		virtual std::size_t get_size() const = 0;

		virtual std::size_t compute_base_offset(const object &target) const = 0;

		virtual score_result_type get_score(const object &target, bool is_ref) const = 0;

		virtual std::shared_ptr<memory::reference> convert_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, bool is_ref) const = 0;

		virtual std::shared_ptr<memory::reference> convert_value(logic::runtime &runtime, const std::byte *data, std::shared_ptr<object> target_type) const = 0;

		virtual std::shared_ptr<evaluator::object> get_evaluator(logic::runtime &runtime) const = 0;

		static int get_score_value(score_result_type score);
	};

	class named_object : public object, public naming::single{
	public:
		named_object(const std::string &value, naming::parent *parent);

		virtual ~named_object();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual std::string get_qualified_naming_value() const override;
	};
}
