#pragma once

#include "naming.h"

namespace cminus::memory{
	class reference;
}

namespace cminus::logic::attributes{
	class object : public naming::single{
	public:
		enum class stage_type{
			nil,
			after_uninitialized_declaration,
			before_ref_assign,
			before_lookup,
			after_lookup,
			before_read,
			after_read,
			before_write,
			after_write,
			before_delete,
			after_delete,
			before_inheritance,
			after_inheritance,
		};

		object(const std::string &name, naming::parent *parent);

		virtual ~object();

		virtual bool is_same(const naming::object &target) const override;

		virtual std::shared_ptr<object> get_non_pointer_object() const;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const = 0;

		virtual bool is_included_in_comparison(logic::runtime &runtime) const;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const;

	protected:
		virtual bool prohibits_stage_(stage_type value) const;

		virtual std::string get_default_message_() const;

		virtual void throw_error_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args, std::size_t message_index) const;

		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void handle_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void handle_before_ref_assign_(logic::runtime &runtime, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;
	};

	class pointer_object : public object{
	public:
		explicit pointer_object(std::shared_ptr<object> target);

		virtual ~pointer_object();

		virtual bool is_same(const naming::object &target) const override;

		virtual std::shared_ptr<object> get_non_pointer_object() const override;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

		virtual bool is_included_in_comparison(logic::runtime &runtime) const override;

		virtual std::shared_ptr<object> get_target() const;

	protected:
		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

		std::shared_ptr<object> target_;
	};

	class bound_object : public object{
	public:
		bound_object(std::shared_ptr<object> target, const std::vector<std::shared_ptr<memory::reference>> &args);

		bound_object(std::shared_ptr<object> target, std::vector<std::shared_ptr<memory::reference>> &&args);

		virtual ~bound_object();

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

		virtual bool is_included_in_comparison(logic::runtime &runtime) const override;

		virtual std::shared_ptr<object> get_target() const;

		virtual const std::vector<std::shared_ptr<memory::reference>> &get_args() const;

	protected:
		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

		std::shared_ptr<object> target_;
		std::vector<std::shared_ptr<memory::reference>> args_;
	};

	class external : public object{
	public:
		explicit external(const std::string &name);

		virtual ~external();
	};

	class read_only : public external{
	public:
		read_only();

		virtual ~read_only();

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

		virtual bool is_included_in_comparison(logic::runtime &runtime) const override;

	protected:
		virtual bool prohibits_stage_(stage_type value) const override;

		virtual std::string get_default_message_() const override;
	};

	class write_only : public external{
	public:
		write_only();

		virtual ~write_only();

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

		virtual bool is_included_in_comparison(logic::runtime &runtime) const override;

	protected:
		virtual bool prohibits_stage_(stage_type value) const override;

		virtual std::string get_default_message_() const override;
	};

	class not_null : public external{
	public:
		not_null();

		virtual ~not_null();

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

		virtual bool is_included_in_comparison(logic::runtime &runtime) const override;

	protected:
		virtual bool prohibits_stage_(stage_type value) const override;

		virtual std::string get_default_message_() const override;

		virtual void handle_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;
	};
}
