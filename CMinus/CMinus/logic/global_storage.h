#pragma once

#include "../node/named_constant_node.h"
#include "../evaluator/evaluator_id.h"

#include "storage_object.h"
#include "../type/primitive_type.h"

namespace cminus::evaluator{
	class object;
}

namespace cminus::logic::storage{
	class global : public object{
	public:
		explicit global();

		virtual ~global();

		virtual void init(logic::runtime &runtime);

		virtual std::shared_ptr<type::object> get_primitve_type(type::primitive::id_type id) const;

		virtual std::shared_ptr<type::object> get_string_type() const;

		virtual std::shared_ptr<memory::reference> get_named_constant(node::named_constant::constant_type type) const;

		virtual std::shared_ptr<evaluator::object> get_evaluator(evaluator::id id) const;

		virtual std::shared_ptr<memory::reference> create_string(logic::runtime &runtime, std::size_t size, const char *value, bool lvalue = false) const;

		virtual std::shared_ptr<memory::reference> create_string(logic::runtime &runtime, const char *value, bool lvalue = false) const;

		virtual std::shared_ptr<memory::reference> create_string(logic::runtime &runtime, const std::string &value, bool lvalue = false) const;

		virtual std::shared_ptr<memory::reference> get_member_reference(logic::runtime &runtime, std::shared_ptr<memory::reference> object, const std::string &name) const;

		virtual const char *get_string_data(logic::runtime &runtime, std::shared_ptr<memory::reference> object) const;

		template <typename value_type>
		std::shared_ptr<memory::reference> create_scalar(value_type value, std::shared_ptr<type::object> target_type = nullptr) const{
			return std::make_shared<memory::scalar_reference<value_type>>(
				((target_type == nullptr) ? get_primitve_type(type::primitive_id<value_type>::template get()) : target_type),
				value
			);
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> create_nan_scalar(value_type value) const{
			auto created = create_scalar(value);
			if (created != nullptr)
				created->add_attribute(find_attribute("#NaN#", false));

			return created;
		}

	protected:
		std::unordered_map<type::primitive::id_type, std::shared_ptr<type::object>> primitive_types_;
		std::unordered_map<node::named_constant::constant_type, std::shared_ptr<memory::reference>> named_constants_;
		std::unordered_map<evaluator::id, std::shared_ptr<evaluator::object>> evaluators_;
	};
}
