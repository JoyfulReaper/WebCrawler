#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "http_client.hpp"
#include "http_request.hpp"
#include <gumbo.h>
#include <cassert>

int main(int argc, char **argv)
{
  asio::io_service io_service;

  boost::thread_group threads; 
   
  http_client hc(io_service);
  http_request rq("www.google.com", "/");
  http_request rq2("reddit.com");
  rq.set_request_type(RequestType::GET);
  hc.make_request(rq);
  //hc.make_request(rq2);
  
  for(std::size_t i = 0; i < 2; i++)
    threads.create_thread(boost::bind(&asio::io_service::run, &io_service));
  
  io_service.run();
  threads.join_all();
  
  std::cout << "\n";
  auto headers = rq.get_headers();
  for(auto &header : headers)
    std::cout << header;
  std::cout << "\n";
    
  std::cout << "\n";
  headers = rq2.get_headers();
  for(auto &header : headers)
    std::cout << header;
  std::cout << "\n";
  
  //std::cout << rq.get_data() << std::endl;
  
  GumboOutput *output = gumbo_parse(rq.get_data().c_str());
  GumboNode *root = output->root;
  
  const GumboVector *root_children = &root->v.element.children;
  GumboNode *head = NULL;
  for(int i = 0; i < root_children->length; ++i)
  {
    GumboNode *child = static_cast<GumboNode*>(root_children->data[i]);
    if(child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_HEAD)
    {
      head = child;
      break;
    }
  }
  
  assert(head != NULL);
  
  GumboVector* head_children = &head->v.element.children;
  for(int i = 0; i < head_children->length; i++)
  {
    GumboNode *child = static_cast<GumboNode*>(head_children->data[i]);
    if(child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_TITLE)
    {
      if(child->v.element.children.length != 1)
        std::cout << "No title" << std::endl;
      GumboNode *title_text = static_cast<GumboNode*>(child->v.element.children.data[0]);
      assert(title_text->type == GUMBO_NODE_TEXT);
      std::cout << "Title: " << title_text->v.text.text << "\n";
    }
  }
  
  return 0;
}
