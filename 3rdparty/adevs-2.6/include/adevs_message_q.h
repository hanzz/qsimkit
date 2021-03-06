/***************
Copyright (C) 2008 by James Nutaro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Bugs, comments, and questions can be sent to nutaro@gmail.com
***************/
#ifndef __adevs_message_q_h_
#define __adevs_message_q_h_
#include "adevs_models.h"
#include "adevs_time.h"
#include <omp.h>
#include <list>
#include <cassert>

namespace adevs
{

template <typename X, class T> class LogicalProcess;

template <typename X, class T = double> struct Message
{
	typedef enum { OUTPUT, EIT } msg_type_t;
	Time<T> t;
	LogicalProcess<X,T> *src;
	Devs<X,T>* target;
	X value;
	msg_type_t type;
	// Default constructor
	Message():value(){}
	// Create a message with a particular value
	Message(const X& value):value(value){}
	// Copy constructor
	Message(const Message& other):
		t(other.t),
		src(other.src),
		target(other.target),
		value(other.value),
		type(other.type)
	{
	}
	// Assignment operator
	const Message<X,T>& operator=(const Message<X,T>& other)
	{
		t = other.t;
		src = other.src;
		target = other.target;
		value = other.value;
		type = other.type;
		return *this;
	}
	// Sort by time stamp, smallest time stamp first in the STL priority_queue
	bool operator<(const Message<X,T>& other) const
	{
		return other.t < t;
	}
	~Message(){}
};

template <class X, class T = double> class MessageQ
{
	public:
		MessageQ()
		{
			omp_init_lock(&lock);
			qshare_empty = true;
			qsafe = &q1;
			qshare = &q2;
		}
		void insert(const Message<X,T>& msg)
		{
			omp_set_lock(&lock);
			qshare->push_back(msg);
			qshare_empty = false;
			omp_unset_lock(&lock);
		}
		bool empty() const { return qsafe->empty() && qshare_empty; }
		Message<X,T> remove()
		{
			if (qsafe->empty())
			{
				std::list<Message<X,T> > *tmp = qshare;
				omp_set_lock(&lock);
				qshare = qsafe;
				qshare_empty = true;
				omp_unset_lock(&lock);
				qsafe = tmp;
			}
			Message<X,T> msg(qsafe->front());
			qsafe->pop_front();
			return msg;
		}
		~MessageQ()
		{
			omp_destroy_lock(&lock);
		}
	private:
		omp_lock_t lock;
		std::list<Message<X,T> > q1, q2;
		std::list<Message<X,T> > *qsafe, *qshare;
		volatile bool qshare_empty;
};

} // end of namespace

#endif
