#include <iostream>
#include <gtest/gtest.h>
#include <chrono>
#include <boost/thread.hpp>
#include "gtest_exit.h"
#include "coroutine.h"
using namespace std;
using namespace co;

int rollback = 0;

struct A
{
    ~A() {
        ++rollback;
    }
};

TEST(testRollback, testRollback)
{
    EXPECT_EQ(rollback, 0);

    for (int i = 0; i < 10; ++i)
        go []{
            A a;
        };
    co_sched.RunUntilNoTask();
    EXPECT_EQ(rollback, 10);

    for (int i = 0; i < 10; ++i)
        go []{
            A a;
            co_yield;
        };
    co_sched.RunUntilNoTask();
    EXPECT_EQ(rollback, 20);

    for (int i = 0; i < 10; ++i)
        go []{
            A a;
            co_yield;
            A a2;
        };
    co_sched.RunUntilNoTask();
    EXPECT_EQ(rollback, 40);
}
