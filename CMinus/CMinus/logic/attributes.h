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

		virtual bool applies_to_function() const;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const = 0;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const;

		virtual std::shared_ptr<object> get_pointer_target(logic::runtime &runtime) const;

	protected:
		virtual bool prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const;

		virtual std::string get_default_message_() const;

		virtual void throw_error_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args, std::size_t message_index) const;

		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void handle_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void handle_before_ref_assign_(logic::runtime &runtime, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;
	};

	class read_guard{
	public:
		read_guard(logic::runtime &runtime, memory::reference *target, bool include_context);

		~read_guard();

	private:
		std::function<void()> callback_;
	};

	class write_guard{
	public:
		write_guard(logic::runtime &runtime, memory::reference *target, bool include_context);

		~write_guard();

	private:
		std::function<void()> callback_;
	};

	class collection{
	public:
		using list_type = std::vector<std::shared_ptr<object>>;
		using optimised_list_type = std::unordered_map<object *, std::shared_ptr<object>>;

		collection();

		explicit collection(const list_type &list);

		explicit collection(const optimised_list_type &list);

		explicit collection(optimised_list_type &&list);

		void add(std::shared_ptr<object> value);

		void remove(const std::string &name, bool global_only);

		void remove(const logic::naming::object &name);

		std::shared_ptr<object> find(const std::string &name, bool global_only) const;

		std::shared_ptr<object> find(const logic::naming::object &name) const;

		bool has(const std::string &name, bool global_only) const;

		bool has(const naming::object &name) const;

		const optimised_list_type &get_list() const;

		void traverse(logic::runtime &runtime, const std::function<void(std::shared_ptr<object>)> &callback, object::stage_type stage) const;

		void traverse(const std::function<void(std::shared_ptr<object>)> &callback) const;

		void call(logic::runtime &runtime, object::stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		void call(logic::runtime &runtime, object::stage_type stage, std::shared_ptr<memory::reference> target) const;

		void print(logic::runtime &runtime) const;

	private:
		optimised_list_type list_;
	};

	class pointer_object : public object{
	public:
		explicit pointer_object(std::shared_ptr<object> target);

		virtual ~pointer_object();

		virtual bool is_same(const naming::object &target) const override;

		virtual bool applies_to_function() const override;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

		virtual std::shared_ptr<object> get_pointer_target(logic::runtime &runtime) const override;

	protected:
		std::shared_ptr<object> target_;
	};

	class bound_object : public object{
	public:
		bound_object(std::shared_ptr<object> target, const std::vector<std::shared_ptr<memory::reference>> &args);

		bound_object(std::shared_ptr<object> target, std::vector<std::shared_ptr<memory::reference>> &&args);

		virtual ~bound_object();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

		virtual std::shared_ptr<object> get_pointer_target(logic::runtime &runtime) const override;

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

	class final : public external{
	public:
		final();

		virtual ~final();

		virtual bool applies_to_function() const override;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

	protected:
		virtual bool prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const override;

		virtual std::string get_default_message_() const override;
	};

	class read_only : public external{
	public:
		read_only();

		virtual ~read_only();

		virtual bool applies_to_function() const override;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

	protected:
		virtual bool prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const override;

		virtual std::string get_default_message_() const override;
	};

	class read_only_context : public external{
	public:
		read_only_context();

		virtual ~read_only_context();

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;
	};

	class write_only : public external{
	public:
		write_only();

		virtual ~write_only();

		virtual bool applies_to_function() const override;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

	protected:
		virtual bool prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const override;

		virtual std::string get_default_message_() const override;
	};

	class not_null : public external{
	public:
		not_null();

		virtual ~not_null();

		virtual bool applies_to_function() const override;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

	protected:
		virtual bool prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const override;

		virtual std::string get_default_message_() const override;
	};

	class ref : public external{
	public:
		ref();

		virtual ~ref();

		virtual bool applies_to_function() const override;

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

	protected:
		virtual bool prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const override;

		virtual std::string get_default_message_() const override;
	};

	class deprecated : public external{
	public:
		deprecated();

		virtual ~deprecated();

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;

	protected:
		virtual void handle_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;
	};

	class special : public external{
	public:
		explicit special(const std::string &name);

		virtual ~special();

		virtual bool handles_stage(logic::runtime &runtime, stage_type value) const override;
	};
}
