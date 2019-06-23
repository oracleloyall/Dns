/*
 * pasetime.h
 *
 *  Created on: 2017年12月14日
 *      Author: zhaoxi
 */

#ifndef PASETIME_HPP_
#define PASETIME_HPP_
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
//精度微妙
class CPassedTime
{
public:
	CPassedTime(void) {}
	~CPassedTime(void) {}

	inline void Start()
	{
		gettimeofday(&m_tvStart, NULL);
	}

	inline unsigned int End()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return ((tv.tv_sec - m_tvStart.tv_sec) * 1000000 + (tv.tv_usec - m_tvStart.tv_usec));
	}

private:
	struct timeval m_tvStart;
};


#endif /* PASETIME_HPP_ */
