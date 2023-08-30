#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>

void HelloThread()
{
    cout << "Hello Thread" << endl;
}

void HelloThread2(INT32 num)
{
    cout << num << endl;
}

int main()
{
    //std::thread t(HelloThread2, 10); //새로운 스레드에서 해당 함수 실행 (메인스레드와 동시에 실행)

    vector<std::thread> v;

    for (INT32 i = 0; i < 10; i++)
    {
        v.push_back(std::thread(HelloThread2, i));
    }

    for (INT32 i = 0; i < 10; i++)
    {
        if (v[i].joinable()) // std::thread 객체에서 스레드가 사용할 수 있는 상태인지 체크
        {
            v[i].join(); // 스레드가 종료할 때까지 기다림
        }
    }

    //INT32 count = t.hardware_concurrency(); // cpu 코어 개수?
    //auto id = t.get_id(); // 스레드마다 부여되는 id

    //t.detach(); // std::thread 객체에서 실제 스레드를 분리
    
    cout << "Hello Main" << endl;

}

