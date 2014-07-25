#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost;

class printer 
{

private:
  asio::strand strand_;
  asio::deadline_timer timer1_;
  asio::deadline_timer timer2_;
  int count_;
  
public:

  printer(asio::io_service& io) : strand_(io),
  timer1_(io, posix_time::seconds(1)),
  timer2_(io, posix_time::seconds(1)),
  count_(0)
  {
    timer1_.async_wait(strand_.wrap(bind(&printer::print1, this)));
    timer2_.async_wait(strand_.wrap(bind(&printer::print2, this)));
  }
  
  ~printer()
  {
    std::cout << "Final count is " << count_ << std::endl;
  }
  
  void print1()
  {
    if(count_ < 100)
    {
      std::cout << "ID: " << this_thread::get_id();
      std::cout << "  Timer 1: " << count_ << "\n";
      ++count_;
      
      timer1_.expires_at(timer1_.expires_at() + posix_time::seconds(1));
      timer1_.async_wait(strand_.wrap(bind(&printer::print1, this)));
    }
  }


  void print2()
  {
    if(count_ < 100)
    {
      std::cout << "ID: " << this_thread::get_id();
      std::cout << "  Timer 2: " << count_ << "\n";
      ++count_;
      
      timer2_.expires_at(timer1_.expires_at() + posix_time::seconds(1));
      timer2_.async_wait(strand_.wrap(bind(&printer::print2, this)));
    }
  }
};

int main()
{
  boost::thread_group tgroup;
  boost::asio::io_service io;
  printer p(io);
  
  for(size_t i = 0; i < 5; i++)
    tgroup.create_thread(boost::bind(&boost::asio::io_service::run, &io));
    
  io.run();
  tgroup.join_all();
}
