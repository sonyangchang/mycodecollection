/*
 * ns3taskqueue.cc
 *
 *  Created on: 2018.6.19
 *      Author: zsy
 *      email 865678017@qq.com
 */
#include "ns3taskqueue.h"
namespace ns3
{
bool Ns3TaskQueue::IsCurrent() const{return true;}
void Ns3TaskQueue::Start()
{
	if(m_running)
	{
		m_taskEvent=Simulator::ScheduleNow(&Ns3TaskQueue::Process,
				this);
	}
}
void Ns3TaskQueue::Process()
{

	if(m_taskEvent.IsExpired())
	{
	    while(!m_pending.empty())
	    {
	        std::unique_ptr<QueuedTask> task=std::move(m_pending.front());//move ownership
	        m_pending.pop_front();
	        task->Run();
	        //task.release();// this will cause memory leakgy
	    }
	    uint64_t time_ms=Simulator::Now().GetMilliSeconds();
	    for(auto it=m_delayTasks.begin();it!=m_delayTasks.end();)
	    {
	    	if(it->first<=time_ms)
	    	{
	    		std::unique_ptr<QueuedTask> task=std::move(it->second);
	    		task->Run();
	    		m_delayTasks.erase(it++);
	    	}else{
	    		break;
	    	}
	    }
	    Time next=MicroSeconds(m_gap);
		m_taskEvent=Simulator::Schedule(next,&Ns3TaskQueue::Process,
				this);
	}
}
void Ns3TaskQueue::PostTask(std::unique_ptr<QueuedTask>task)
{
	m_pending.push_back(std::move(task));
    if(!m_running)
    {
    	m_running=true;
    	Start();
    }
}
void Ns3TaskQueue::PostDelayedTask(std::unique_ptr<QueuedTask> task, uint32_t time_ms)
{

    uint64_t now=Simulator::Now().GetMilliSeconds();
    uint64_t future=now+time_ms;
    m_delayTasks.insert(std::make_pair(future,std::move(task)));
    if(!m_running)
    {
    	m_running=true;
    	Start();
    }
}
}



