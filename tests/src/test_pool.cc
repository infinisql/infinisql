#include <gtest/gtest.h>
#include "Message.h"
#include "Pool.h"

TEST(PoolTest, Acquire) {
	Pool<Message> msg_pool;

	auto v = msg_pool.acquire();
	ASSERT_TRUE((bool)v);
}

/*TEST(PoolTest, Release) {
	Pool<Message> msg_pool;

	auto v = msg_pool.acquire();
	ASSERT_TRUE((bool)v);
	msg_pool.release(v);
}*/
