#include "dlibxx.hxx"

#include <dlfcn.h>
#include <stdexcept>

namespace dlibxx
{
	int parse_resolution_policy(resolve_policy rp)
	{
		switch (rp) {
			case resolve_policy::lazy: return int(RTLD_LAZY);
			case resolve_policy::now:  return int(RTLD_NOW);
		}
	}

	handle::handle(resolve_policy rt, const char* file)
	{
		int flags = parse_resolution_policy(rt) | int(RTLD_LOCAL);
		handle_ = ::dlopen(file, flags);

		set_error();
	}

	handle::handle(resolve_policy rt)
	{
		int flags = parse_resolution_policy(rt) | int(RTLD_LOCAL);
		handle_ = ::dlopen(nullptr, flags);

		set_error();
	}

	handle::~handle()
	{
		if (handle_) ::dlclose(handle_);
		handle_ = nullptr;
	}

	bool handle::set_error() const
	{
		char* error_message = ::dlerror();
		if (error_message != nullptr) {
			error_ = std::string(error_message);
			return true;
		}
		return false;
	}

	void* handle::sym_lookup(const char* fname) const {
		return ::dlsym(handle_, fname);
	}

	bool handle::has_library_loaded(const char* file) {
		return ::dlopen(file, RTLD_LAZY | RTLD_NOLOAD) != NULL;
	}
}
