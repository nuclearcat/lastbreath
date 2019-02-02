#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <boost/circular_buffer.hpp>
#include <boost/assert.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class Main {
public:
   boost::circular_buffer<std::string> *cb;
   std::string logpath;
   int timestamp;
} m;

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void
signal_callback_handler(int signum)
{
   if (signum)
      printf("Caught signal %d\n", signum);
   time_t t = time(NULL);
   std::ofstream log;
   // Form a filename
   std::string filename = m.logpath;
   if (m.timestamp == 1) {
      filename.append(std::to_string(t));
   }
   filename.append(".txt");
   
   // Write log
   log.open(filename.c_str());
   for (std::string s : *m.cb) {
      log << s << std::endl;
   }
   log.close();
   // Bye bye
   exit(signum);
}

int main(int ac, char* av[]) {
   std::string intermediate;
   try {
      std::string default_logdump = "/tmp/crashdump_";
      po::options_description desc("Allowed options");
      desc.add_options()
      ("help", "produce help message")
      ("logfile", po::value<std::string>()->default_value(default_logdump), "path and prefix of crash log (/tmp/crashdump_)")
      ("timestamp", po::value<int>()->default_value(0)->implicit_value(1), "add timestamp")
      ;

      po::variables_map vm;
      po::store(po::parse_command_line(ac, av, desc), vm);
      if (vm.count("help")) {
         std::cout << desc << std::endl;

      }
      m.logpath = vm["logfile"].as<std::string>();
      m.timestamp = vm["timestamp"].as<int>();
      po::notify(vm);
   }
   catch (std::exception& e) {
      std::cerr << "error: " << e.what() << "\n";
      return 1;
   }
   catch (...) {
      std::cerr << "Exception of unknown type!\n";
   }

   m.cb = new boost::circular_buffer<std::string>(256);
   
   signal(SIGINT, signal_callback_handler);

   while (getline(std::cin, intermediate)) {
      m.cb->push_back(intermediate);
   }
   signal_callback_handler(0);
   return EXIT_SUCCESS;
}
