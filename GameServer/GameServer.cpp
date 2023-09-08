#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>


int64 Calculate()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100000; i++)
	{
		sum += i;
	}

	return sum;
}

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("Secret Message");
}

void TaskWorker(std::packaged_task<int64(void)>&& task)
{
	task();
}

int main()
{
	// 동기(synchronous) 실행
	//int64 sum = Calculate();

	//cout << sum << endl;

	// std::future
	{
		// 1) deferred -> lazy evaluation 지연해서 실행하세요
		// 2) asnyc -> 별도의 스레드를 만들어서 실행하세요
		// 3) deferred | async -> 둘 중 알아서 골라주세요

		std::future<int64> future = std::async(std::launch::async, Calculate);

		// TODO
		std::future_status status = future.wait_for(1ms); // 작업상태

		int64 sum = future.get(); // 결과물이 필요할때 Get으로 가져와서 사용
	}

	// std::promise
	{
		// 미래(std::future)에 결과물을 반환해줄꺼라 약속
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, std::move(promise));

		string message = future.get();
		cout << message << endl;

		t.join();
	}

	// std::packaged_task
	{
		std::packaged_task<int64(void)> task(Calculate);
		std::future<int64> future = task.get_future();

		std::thread t(TaskWorker, std::move(task));

		int64 sum = future.get();
		cout << sum << endl;

		t.join();
	}

	// 결론)
	// mutex, condition_valuable까지 가지 않고 단순한 애들을 처리
	// 특히나 한 번 발생하는 이벤트에 유용하다

	// 1)async
	// 원하는 함수를 비동기적으로 실행
	// 2)promise
	// 결과물을 promise를 통해 future로 받아줌
	// 3) packaged_task
	// 원하는 함수의 실행 결과를 packaged_task를 통해 future로 받아줌
}

