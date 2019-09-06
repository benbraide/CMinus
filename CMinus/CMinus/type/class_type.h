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

		struct computed_base_type_info{
			access_type access;
			std::size_t offset;
		};

		class_(logic::runtime &runtime, const std::string &name, logic::storage::object *parent = nullptr);

		virtual ~class_();

		virtual void construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> initialization) const override;

		virtual void destruct(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const;

		virtual void print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t compute_base_offset(const type::object &target) const override;

		virtual bool is_exact(logic::runtime &runtime, const type::object &target) const override;

		virtual score_result_type get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> get_default_value(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, cast_type type) const override;

		virtual std::shared_ptr<evaluator::object> get_evaluator(logic::runtime &runtime) const override;

		using with_storage::find;
		using with_storage::find_operator;

		virtual std::shared_ptr<memory::reference> find(logic::runtime &runtime, const search_options &options) const override;

		virtual std::shared_ptr<memory::reference> find_operator(logic::runtime &runtime, const type::object &target_type, bool is_ref, const namesless_search_options &options) const;

		virtual bool add_base(access_type access, std::shared_ptr<type::object> value);

		virtual bool add_declaration(std::shared_ptr<declaration::object> value);

		virtual void build();

		virtual relationship_type get_relationship(const type::object &target) const;

		virtual bool is_ancestor(const type::object &target) const;

		virtual bool is_base(const type::object &target) const;

		virtual void traverse_non_static_entries(const std::function<void(const std::string &, std::shared_ptr<memory::reference>)> &callback) const;

	protected:
		virtual bool validate_(const declaration::function_base &target) const override;

		virtual void extend_function_group_(logic::runtime &runtime, declaration::function_group_base &group, std::shared_ptr<declaration::function_base> entry) override;

		virtual std::shared_ptr<memory::reference> find_(logic::runtime &runtime, const search_options &options) const;

		virtual void get_computed_base_info_(const type::object &target, computed_base_type_info &info) const;

		virtual void check_access_(const class_ *scope, access_type access, std::shared_ptr<memory::reference> target) const;

		logic::runtime &runtime_;
		std::size_t size_ = sizeof(void *);

		std::size_t static_size_ = 0u;
		std::size_t static_address_ = 0u;

		std::unordered_map<std::string, base_type_info> base_types_;
		std::unordered_map<std::string, std::shared_ptr<memory::reference>> non_static_entries_;

		std::list<std::string> declared_names_;
		std::unordered_map<std::string, std::list<std::shared_ptr<declaration::object>>> declarations_;
	};
}
