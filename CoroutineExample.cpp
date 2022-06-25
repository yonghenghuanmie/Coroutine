#include <atomic>
#include <string>
#include <chrono>
#include <iterator>
#include <iostream>
#include <optional>
#include <algorithm>
#include <coroutine>
#include "cpr/cpr.h"
#include "metalang99.h"
#include "ConstraintType.h"

using namespace std::chrono_literals;

template<typename Response>
struct promise_;

template<typename Response>
struct coroutine :std::coroutine_handle<promise_<Response>>
{
	using promise_type = promise_<Response>;
	~coroutine() { this->destroy(); }
};

template<typename AsyncResponse>
struct awaitable
{
	AsyncResponse& response;

	bool await_ready() { return false; }
	bool await_suspend(std::coroutine_handle<> handle)
	{
		if (response.wait_for(0s) == std::future_status::timeout)
			return true;
		else
			return false;
	}
	std::optional<decltype(response.get())> await_resume()
	{
		if (response.wait_for(0s) == std::future_status::ready)
			return { response.get() };
		else
			return {};
	}
};

namespace ConstraintType
{
	AddTypeLayer(0, awaitable);
	ConstructEligibleType(AwaitableType, 1, 0, Any<void>);
}

template<typename Response>
struct promise_
{
	Response response;
	coroutine<Response> get_return_object()
	{
		return { coroutine<Response>::from_promise(*this) };
	}

	std::suspend_never initial_suspend() noexcept { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }

	void return_value(const Response& response) noexcept { this->response = std::move(response); }

	template<typename Awaitable, ConstraintType::AwaitableType Allow = std::remove_cvref_t<Awaitable>>
	Awaitable await_transform(Awaitable&& awaiter) { return std::forward<Awaitable>(awaiter); }
	template<typename AsyncResponse>
	awaitable<AsyncResponse> await_transform(AsyncResponse&& response) { return { response }; }

	void unhandled_exception() {}
};

coroutine<cpr::Response> GetResponse()
{
	auto future = cpr::GetCallback(
		[&](cpr::Response res) {
			std::this_thread::sleep_for(1s);
			return res; },
		cpr::Url{ "http://www.httpbin.org/get" });
	while (true)
	{
		auto result = co_await future;
		if (result)
			co_return result.value();
	}
}

int main()
{
	auto c = GetResponse();
	int i = 0;
	while (!c.done())
	{
		++i;
		c();
	}
	std::cout << "count i=" << i << std::endl;

	const auto& response = c.promise().response;
	std::ostream_iterator<std::string> os_iter(std::cout, ",\n");
	std::cout << "header:" << std::endl;
	std::for_each(
		response.header.begin(), response.header.end(),
		[&os_iter](auto&& pair) {
			*os_iter++ = "\t" + pair.first + ":" + pair.second;
		}
	);
	std::cout << "text: " << response.text << std::endl;
	std::cout << "error: " << response.error.message << std::endl;
	return 0;
}