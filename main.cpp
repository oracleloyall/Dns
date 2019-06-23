/*
 * * Copyright (c) 2019,zhao xi
 * All rights reserved.
 * Free Dns server for linux
 *
 */
#include <stdio.h>
#include <boost/thread.hpp>
#include "net/network.h"
using namespace std;
using namespace network;
// 数据处理函数
// @sess session标识
// @data 收到的数据起始指针
// @bytes 收到的数据长度
// @returns: 返回一个size_t, 表示已处理的数据长度. 当返回-1时表示数据错误, 链接即会被关闭.
size_t OnMessage(SessionEntry sess, const char* data, size_t bytes)
{
  //  printf("receive: %.*s\n", (int)bytes, data);
    sess->Send(data, bytes);  ; // Echo. 将收到的数据原样回传
    return bytes;
}

int main()
{
    // Step1: 创建一个Server对象
    Server server;
    server.SetReceiveCb(&OnMessage);
    boost_ec ec = server.goStart("udp://127.0.0.1:53");
    if (ec) {
        printf("server start error %d:%s\n", ec.value(), ec.message().c_str());
        return 1;
    } else {
        printf("server start at %s:%d\n", server.LocalAddr().address().to_string().c_str(),
                server.LocalAddr().port());
    }

//	boost::thread_group tg;
//	for (int i = 0; i < 4; ++i) {
//		tg.create_thread([] {
//			//co_sched.RunLoop();
//				 co_sched.RunUntilNoTask();
//		});
//	}
//	tg.join_all();
   co_sched.RunUntilNoTask();
    return 0;
}
