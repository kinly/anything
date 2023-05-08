#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <queue>

namespace inlay::base {
    class work_threads final {

        struct worker final {
            std::thread _thread;
            std::queue<std::function<void()>> _tasks;
            std::mutex _queue_mutex;
            std::condition_variable _condition;
            std::atomic_bool _stop;

            worker()
                : _thread([this] {
                    for (;;) {
                        std::function<void()> task;

                        // namespace
                        {
                            std::unique_lock<std::mutex> lock(this->_queue_mutex);
                            this->_condition.wait(lock, [this] { return this->_stop || !this->_tasks.empty(); });
                            if (this->_stop && this->_tasks.empty()) return;
                            task = std::move(this->_tasks.front());
                            this->_tasks.pop();
                        }

                        task();
                    }
                })
                , _stop(false) {
            }

            ~worker() {
                stop();
            }

            void stop() {
                _stop.store(true);
                _condition.notify_one();
                if (_thread.joinable()) {
                    _thread.join();
                }
            }

            template<class F, class... Args>
            auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>... >> {
                using return_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>... >;

                auto task = std::make_shared<std::packaged_task<return_type()>>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

                std::future<return_type> res = task->get_future();
                {
                    std::unique_lock<std::mutex> lock(_queue_mutex);

                    // don't allow submit after stopping the pool
                    if (_stop) {
                        throw std::runtime_error("stopped....");
                    }

                    _tasks.emplace([task]() { (*task)(); });
                }
                _condition.notify_one();
                return res;
            }
        };
    private:
        // need to keep track of threads so we can join them
        std::vector<std::shared_ptr<worker>> _workers;
        std::atomic_size_t _workers_count;
    public:
        work_threads(size_t cnt = std::thread::hardware_concurrency());

        template<class F, class... Args>
        auto submit(std::size_t index, F&& f, Args&&... args) -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>... >>;

        ~work_threads();

        static work_threads& instance() {
            static work_threads inst;
            return inst;
        }
    };

    // the constructor just launches some amount of _workers
    inline work_threads::work_threads(size_t threads) {

        size_t const hardware = std::thread::hardware_concurrency();
        size_t const hardware_default = 4;
        size_t re_threads = std::min(hardware != 0 ? hardware : hardware_default, threads);

        for (size_t i = 0; i < re_threads; ++i) {
            _workers.emplace_back(new worker());
        }
        _workers_count = _workers.size();
    }

    // add new worker item to the pool
    template<class F, class... Args>
    auto work_threads::submit(std::size_t index, F&& f, Args&&... args) -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>... >> {
        if (index >= _workers_count) {
            throw std::runtime_error("index out of workers range");
        }

        return _workers.at(index)->submit(std::forward<F>(f), std::forward<Args>(args)...);
    }

    // the destructor joins all threads
    inline work_threads::~work_threads() {
    }
}; // end namespace inlay


/*
 *
 * https://github.com/bshoshany/thread-pool
BS::synced_stream sync_out;
BS::thread_pool pool;
while (true) {
    for (int index = 0; index < 5; ++index) {
        pool.submit([index, &sync_out]() {
            inlay::base::work_threads::instance().submit(index, [index, &sync_out]() {
            sync_out.println("Task no. ", index, " executing. thread. ", std::this_thread::get_id(), " ",
                             std::chrono::duration_cast<std::chrono::milliseconds>(
                                 std::chrono::system_clock::now().time_since_epoch()).count());
        });
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
 */
