#pragma once

#include "../logic/naming.h"

namespace cminus::memory{
	class object;
	class reference;
}

namespace cminus::evaluator{
	class object;
}

namespace cminus::node{
	class object;
}

namespace cminus::type{
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

		enum class cast_type{
			nil,
			static_,
			reinterpret,
			dynamic,
			ref_static,
			ref_dynamic,
			rval_static,
		};

		virtual ~object();

		virtual bool is_auto() const;

		virtual void construct_default(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const;

		virtual void construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> initialization) const;

		virtual void destruct_construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const;

		virtual void print(logic::runtime &runtime, bool is_qualified) const = 0;

		virtual void print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const = 0;

		virtual std::size_t get_size() const = 0;

		virtual std::size_t compute_base_offset(const object &target) const;

		virtual bool is_exact(logic::runtime &runtime, const object &target) const;

		virtual score_result_type get_score(logic::runtime &runtime, const object &target, bool is_ref) const = 0;

		virtual std::shared_ptr<memory::reference> get_default_value(logic::runtime &runtime) const = 0;

		virtual std::shared_ptr<memory::reference> cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, cast_type type) const = 0;

		virtual std::shared_ptr<evaluator::object> get_evaluator(logic::runtime &runtime) const = 0;

		static int get_score_value(score_result_type score);
	};

	class named_object : public object, public logic::naming::single{
	public:
		named_object(const std::string &value, logic::naming::parent *parent);

		virtual ~named_object();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual std::string get_qualified_naming_value() const override;
	};
}
