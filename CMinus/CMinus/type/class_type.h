#pragma once

#include "../declaration/function_declaration_group.h"

#include "type_with_storage.h"

namespace cminus::type{
	class class_ : public with_storage{
	public:
		enum class relationship_type{
			nil,
			self,
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

		explicit class_(logic::runtime &runtime, const std::string &name, logic::storage::object *parent = nullptr);

		virtual ~class_();

		virtual void construct_default(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const override;

		virtual void construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> initialization) const override;

		virtual void print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t compute_base_offset(const type::object &target) const override;

		virtual bool is_exact(logic::runtime &runtime, const type::object &target) const override;

		virtual score_result_type get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> get_default_value(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, cast_type type) const override;

		virtual std::shared_ptr<memory::reference> find(logic::runtime &runtime, const std::string &name, bool search_tree, const storage_base_type **branch = nullptr) const override;

		virtual bool add_base(logic::runtime &runtime, access_type access, std::shared_ptr<type::object> value);

		virtual bool add_declaration(logic::runtime &runtime, access_type access, std::shared_ptr<declaration::variable> value);

		virtual relationship_type get_relationship(const type::object &target) const;

		virtual bool is_ancestor(const type::object &target) const;

		virtual bool is_base(const type::object &target) const;

	protected:
		virtual bool validate_(const declaration::function_base &target) const override;

		virtual void extend_function_group_(logic::runtime &runtime, declaration::function_group_base &group, std::shared_ptr<declaration::function_base> entry) override;

		std::size_t size_ = sizeof(void *);
		std::unordered_map<std::string, base_type_info> base_types_;
		std::shared_ptr<logic::storage::object> this_storage_;
	};
}
