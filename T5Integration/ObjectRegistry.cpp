#include <cassert>
#include <ObjectRegistry.h>

namespace T5Integration {

	ObjectRegistry* ObjectRegistry::_instance = nullptr;

	ObjectRegistry::ObjectRegistry() {
		_instance = this;
	}

	T5Service::Ptr ObjectRegistry::service() {
		assert(_instance);
		return _instance->get_service();
	}

	T5Math::Ptr ObjectRegistry::math() {
		assert(_instance);
		return _instance->get_math();
	}

	Logger::Ptr ObjectRegistry::logger() {
		assert(_instance);
		return _instance->get_logger();
	}

	Scheduler::Ptr ObjectRegistry::scheduler() {
		assert(_instance);
		return _instance->get_scheduler();
	}

	Logger::Ptr ObjectRegistry::get_logger() {
		if (!_logger)
			_logger = std::make_shared<DefaultLogger>();
		return _logger;
	}

	Scheduler::Ptr ObjectRegistry::get_scheduler() {
		if (!_scheduler)
			_scheduler = std::make_shared<Scheduler>();
		return _scheduler;
	}
}