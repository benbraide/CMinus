#pragma once

#include "../logic/function.h"

#include "type_with_storage.h"

namespace cminus::type{
	class class_ : public with_storage{
	public:
		enum class relationship_type{
			nil,
			base,
			ancestor,
			inherited,
		};

		enum class access_type{
			nil,
			public_,
			protected_,
			private_,
		};

		struct base_type_info{
			access_type access;
			std::shared_ptr<type::object> value;
		};

		explicit class_(const std::string &name, logic::storage::object *parent = nullptr);

		virtual ~class_();

		virtual void construct_default(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const override;

		virtual void construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> initialization) const override;

		virtual void print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t compute_base_offset(const type::object &target) const override;

		virtual score_result_type get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> get_default_value(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, cast_type type) const override;

		virtual std::shared_ptr<memory::reference> find(const std::string &name, bool search_tree, const storage_base_type **branch = nullptr) const override;

		virtual bool add_base(access_type access, std::shared_ptr<type::object> value);

		virtual bool add_function(access_type access, std::shared_ptr<logic::function_object> value);

		virtual bool add_declaration(access_type access, std::shared_ptr<logic::declaration> value);

		virtual relationship_type get_relationship(const type::object &target) const;

	protected:
		std::size_t size_ = sizeof(void *);
		std::unordered_map<std::string, base_type_info> base_types_;
	};
}
