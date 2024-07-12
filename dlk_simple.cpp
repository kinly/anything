
template<class value_tt>
class thread_local_helper final {
private:
  static inline thread_local value_tt _value{};

public:
  thread_local_helper() = default;
  ~thread_local_helper() = default;

  static void set(value_tt &&v) { _value = std::move(v); }
  static void set(const value_tt &v) { _value = v; }
  static value_tt get() { return _value; }
};

class dlk_helper final {
  sw::redis::Redis _redis;

public:
  dlk_helper(std::string_view uri) : _redis(sw::redis::Redis(uri.data())) {}

  inline bool lock(std::string_view key_sv) {

    static constexpr std::chrono::milliseconds ttl{5000};

    const auto uuid = util::uuid_snowflake::generator::inst_mt().nextid();
    const auto uuid_string = std::to_string(uuid);
    thread_local_helper<std::string>::set(uuid_string);

    return _redis.set(key_sv, uuid_string, ttl,
                      sw::redis::UpdateType::NOT_EXIST);
  }

  inline bool unlock(std::string_view key_sv) {
    auto value = thread_local_helper<std::string>::get();

    std::string_view script =
        "if redis.call('get', KEYS[1]) == ARGV[1] then return "
        "redis.call('del', KEYS[1]) else return 0 end";

    const auto result = _redis.eval<long long>(
        script, std::initializer_list<std::string_view>{key_sv},
        std::initializer_list<std::string_view>{value});
    return 1 == result;
  }
};

int main() {
  std::string_view dlk_key = "123";

  std::pair<int, int> count;

  std::thread t1([dlk_key, &count] {
    dlk_helper dlk(
        "redis://"
        "123456@127.0.0.1:56379?socket_timeout=1s&connect_timeout=1s&keep_alive=true");

    for (int i = 0; i < 1000; ++i) {
      while (dlk.lock(dlk_key) == false)
        std::this_thread::sleep_for(1ms);

      count.first += 1;
      std::cout << "thread 1 locked...." << util::datetime::current_timestamp()
                << " count: " << count.first << ":" << count.second
                << std::endl;
      // std::this_thread::sleep_for(10ms);
      dlk.unlock(dlk_key);
    }
  });

  std::thread t2([dlk_key, &count] {
    dlk_helper dlk(
        "redis://"
        "123456@127.0.0.1:56379?socket_timeout=1s&connect_timeout=1s&keep_alive=true");
    for (int i = 0; i < 1000; ++i) {
      while (dlk.lock(dlk_key) == false)
        std::this_thread::sleep_for(1ms);

      count.second += 1;
      std::cout << "thread 2 locked...." << util::datetime::current_timestamp()
                << " count: " << count.first << ":" << count.second
                << std::endl;
      // std::this_thread::sleep_for(10ms);
      dlk.unlock(dlk_key);
    }
  });

  t1.detach();
  t2.detach();

  while (true) {
    std::this_thread::sleep_for(1s);
  }

  return 0;
}
