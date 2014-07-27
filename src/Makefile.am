bin_PROGRAMS = webCrawler

noinst_LIBRARIES = liblogger.a
liblogger_a_SOURCES = logger/logger.cxx

webCrawler_SOURCES = main.cpp http_client.cpp http_request.cpp
webCrawler_LDADD = $(LUA_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(GUMBO_LIBS) liblogger.a
webCrawler_LDFLAGS = $(BOOST_LDFLAGS)
webCrawler_CPPFLAGS = $(LUA_INCLUDE) $(BOOST_CPPFLAGS) $(GUMBO_INCLUDE) -pthread -Wall
