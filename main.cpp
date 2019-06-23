/*
 * * Copyright (c) 2019,zhao xi
 * All rights reserved.
 * Free Dns server for linux
 *
 */
#include <stdio.h>
#include<iostream>
#include <boost/thread.hpp>
#include "net/network.h"
#include <tins/tins.h>
#include <iostream>

using std::cout;
using std::endl;

using namespace Tins;

PacketSender sender;

bool callback(const PDU& pdu) {
    EthernetII eth = pdu.rfind_pdu<EthernetII>();
    IP ip = eth.rfind_pdu<IP>();
    UDP udp = ip.rfind_pdu<UDP>();
    DNS dns = udp.rfind_pdu<RawPDU>().to<DNS>();

    // Is it a DNS query?
    if (dns.type() == DNS::QUERY) {
        // Let's see if there's any query for an "A" record.
        for (const auto& query : dns.queries()) {
            if (query.query_type() == DNS::A) {
                // Here's one! Let's add an answer.
                dns.add_answer(
                    DNS::resource(
                        query.dname(),
                        "127.0.0.1",
                        DNS::A,
                        query.query_class(),
                        // 777 is just a random TTL
                        777
                    )
                );
            }
        }
        // Have we added some answers?
        if (dns.answers_count() > 0) {
            // It's a response now
            dns.type(DNS::RESPONSE);
            // Recursion is available(just in case)
            dns.recursion_available(1);
            // Build our packet
            auto pkt = EthernetII(eth.src_addr(), eth.dst_addr()) /
                       IP(ip.src_addr(), ip.dst_addr()) /
                       UDP(udp.sport(), udp.dport()) /
                       dns;
            // Send it!
            sender.send(pkt);
        }
    }
    return true;
}
using namespace std;
using namespace network;
static std::atomic<unsigned long> packet_times;
void show_status()
{
//    static int s_show_index = 0;
//    if (s_show_index++ % 10 == 0) {
//        printf("  index    conn \n");
//    }
	while(1){
	sleep(1);
    std::cout<<"Send "<<packet_times<<" packets"<<std::endl;
	}
}

// 数据处理函数
// @sess session标识
// @data 收到的数据起始指针
// @bytes 收到的数据长度
// @returns: 返回一个size_t, 表示已处理的数据长度. 当返回-1时表示数据错误, 链接即会被关闭.(for tcp )
size_t Tcp_OnMessage(SessionEntry sess, const char* data, size_t bytes)
{
  //  printf("receive: %.*s\n", (int)bytes, data);
    sess->Send(data, bytes);  // Echo. 将收到的数据原样回传
    return bytes;
}
size_t OnMessage(SessionEntry sess, const char* data, size_t bytes)
{
  //  printf("receive: %.*s\n", (int)bytes, data);
	 ++packet_times;
	  DNS dns(reinterpret_cast<unsigned char *>(const_cast<char *>(data)),bytes);
	  for (const auto& query : dns.queries()) {
	         cout << query.dname() << std::endl;
	   }
	    // Is it a DNS query?
	    if (dns.type() == DNS::QUERY) {
	        // Let's see if there's any query for an "A" record.
	        for (const auto& query : dns.queries()) {
	            if (query.query_type() == DNS::A) {
	            	  cout << " Add query A " << std::endl;
	                // Here's one! Let's add an answer.
	                dns.add_answer(
	                    DNS::resource(
	                        query.dname(),
	                        "127.0.0.1",
	                        DNS::A,
	                        query.query_class(),
	                        // 777 is just a random TTL
	                        777
	                    )
	                );
	            }
	        }
	        // Have we added some answers?
	        if (dns.answers_count() > 0) {
	            // It's a response now
	            dns.type(DNS::RESPONSE);
	            // Recursion is available(just in case)
	            dns.recursion_available(1);
	            // Build our packet
	            auto pkt =  dns;
	            // Send it!
	           // sender.send(pkt);
	            sess->Send(pkt.records_data_._M_data_ptr(), pkt.records_data_.size());
	        }
	    }
	  //  dns.records_data_;

	// sess->Send(data, bytes);
  //  go [sess,data,bytes]{ sess->Send(data, bytes); } ; // Echo. 将收到的数据原样回传
    return 0;
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

	boost::thread_group tg;
	for (int i = 0; i < 4; ++i) {
		tg.create_thread([] {
			//co_sched.RunLoop();
			//	 co_sched.RunUntilNoTask();
		});
	}
//	tg.join_all();
	boost::thread show(&show_status);

    co_sched.RunUntilNoTask();
    show.join();
    return 0;
}
