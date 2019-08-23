#pragma once

#include "../node/named_constant_node.h"
#include "../evaluator/evaluator_id.h"

#include "storage_object.h"
#include "primitive_type.h"

namespace cminus::evaluator{
	class object;
}

namespace cminus::logic::storage{
	class global : public object{
	public:
		global();

		virtual ~global();

		virtual std::shared_ptr<type::object> get_primitve_type(type::primitive::id_type id) const;

		virtual std::shared_ptr<memory::reference> get_named_constant(node::named_constant::constant_type type) const;

		virtual std::shared_ptr<evaluator::object> get_evaluator(evaluator::id id) const;

	protected:
		std::unordered_map<type::primitive::id_type, std::shared_ptr<type::object>> primitive_types_;
		std::unordered_map<node::named_constant::constant_type, std::shared_ptr<memory::reference>> named_constants_;
		std::unordered_map<evaluator::id, std::shared_ptr<evaluator::object>> evaluators_;
	};
}
