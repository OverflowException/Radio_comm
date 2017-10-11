#include <queue>
#include <iostream>

int main(int argc, char** argv)
{
  std::queue<int> test_queue;
  test_queue.push(10);
  test_queue.push(11);
  test_queue.push(12);
  test_queue.push(13);
  std::cout << test_queue.front() << " " << test_queue.back() << std::endl;
  test_queue.pop();
  std::cout << test_queue.front() << " " << test_queue.back() << std::endl;
}
