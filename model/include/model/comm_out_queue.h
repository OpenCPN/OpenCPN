#include <mutex>
#include <string>
#include <vector>

/**
 *  Queue of NMEA0183 messages which only holds a limited amount
 *  of each message type.
 */
class CommOutQueue {
public:
  /**
   * Insert valid line of NMEA0183 data in buffer.
   * @return false on errors including invalid input, else true.
   */
  virtual bool push_back(const std::string& line);

  /**
   * Return  next line to send and remove it from buffer,
   * throws exception if empty.
   */
  std::string pop();

  /** Return number of lines in queue. */
  int size() const;

  /**
   * Create a buffer which stores at most message_count items of each
   * message.
   */
  CommOutQueue(int message_count);

  CommOutQueue() : CommOutQueue(1) {}

  // Disable copying and assignment
  CommOutQueue(const CommOutQueue& other) = delete;
  CommOutQueue& operator=(const CommOutQueue&) = delete;

protected:
  struct BufferItem {
    long type;
    std::string line;
    BufferItem(const std::string& line);
    BufferItem(const BufferItem& other);
  };

  std::vector<BufferItem> m_buffer;
  mutable std::mutex m_mutex;
  int m_size;
};

/** A  CommOutQueue limited to one message of each kind. */
class CommOutQueueSingle : public CommOutQueue {
public:
  CommOutQueueSingle() : CommOutQueue(1) {}

  /** Insert line of NMEA0183 data in buffer. */
  bool push_back(const std::string& line);
};

/** Add unit test measurements to CommOutQueue. */
class MeasuredCommOutQueue : public CommOutQueue {
public:
  MeasuredCommOutQueue(int size) : CommOutQueue(size), push_time(0) {}

  bool push_back(const std::string& line);

  double push_time;
};
