/*#include <ctime>
  #include <chrono>*/
#include <string>
#include <sstream>
#include <sys/time.h>


namespace rfcom
{
  std::string getTimeStr()
    {
      timeval curTime;
      gettimeofday(&curTime, NULL);
      int milli = curTime.tv_usec / 1000;

      char buffer [80];
      strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

      std::ostringstream oss;
      oss << buffer;
      oss << ":" << milli;

      
      char currentTime[84] = "";
      sprintf(currentTime, "%s:%d", buffer, milli);
      return oss.str();

/*using std::chrono::system_clock;
    system_clock::time_point current = system_clock::now();
    std::time_t tt;
    tt = system_clock::to_time_t(current);
    return std::string(ctime(&tt));*/
    }
}
