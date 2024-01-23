#include  <mutex>
#include  <string>
#include  <vector>

/**
 *  Queue of NMEA0183 messages which only holds a limited amount
 *  of each message type.
 */
class CommOutQueue {
public:

  /** Insert line of NMEA0183 data in buffer. */
  virtual void push_back(const std::string& line);

  /** Return copy of next line to send, throws exception if empty. */
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
  void push_back(const std::string& line);
};
