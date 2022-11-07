#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

template <typename T>
class DoubleBuffer {
 public:
    using UpdaterFunc = std::function<void(T& data)>;

 public:
    DoubleBuffer(std::shared_ptr<T> read_buffer, std::shared_ptr<T> write_buffer) {
        buffers_[0] = read_buffer;
        buffers_[1] = write_buffer;
        read_idx_ = 0;
    }

    explicit DoubleBuffer(std::shared_ptr<T> data_sptr) {
        buffers_[0] = data_sptr;
        buffers_[1] = std::make_shared<T>(*data_sptr.get());
        read_idx_ = 0;
    }

    explicit DoubleBuffer(const T& data) {
        buffers_[0] = std::make_shared<T>(data);
        buffers_[1] = std::make_shared<T>(data);
        read_idx_ = 0;
    }

 public:
    /**
     * @brief Get read_buffer and you can read it without restriction
     * 
     * @return std::shared_ptr<T> 
     */
    std::shared_ptr<T> Load() {
        return buffers_[read_idx_];
    }

    /**
     * @brief It will block until it update two buffer successfully
     *        
     */
    void Update(const UpdaterFunc& updater) {
        this->update_write_buffer(updater);
        this->update_write_buffer(updater);
    }

    /**
     * @brief Reset the value of write buffer
     * 
     * @param data 
     */
    void Reset(const T& data) {
        this->reset_write_buffer(data);
        this->reset_write_buffer(data);
    }

 private:
    // try to get write buffer exclusively multiple writer thread scenarios
    std::shared_ptr<T> try_monopolizer_writer_buffer() {
        int write_idx = 1 - read_idx_;
        if (buffers_[write_idx].use_count() == 1) {
            // reference count of write_buffer will add 1
            // so other writer threads cannot obtain the write_buffer
            return buffers_[write_idx];
        }
        return nullptr;
    }

    // Only once writer thread can perform updater function at the same time
    // Lock by the reference count of write_buffer
    void update_write_buffer(const UpdaterFunc& updater) {
        std::shared_ptr<T> write_buffer = try_monopolizer_writer_buffer();
        if (write_buffer != nullptr) {
            updater(*write_buffer.get());
            swap_buffer();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            this->update_write_buffer(updater);
        }
    }

    void reset_write_buffer(const T& data) {
        std::shared_ptr<T> write_buffer = try_monopolizer_writer_buffer();
        if (write_buffer != nullptr) {
            *write_buffer.get() = data;
            swap_buffer();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            this->reset_write_buffer(data);
        }
    }

    void swap_buffer() {
        read_idx_ = 1 - read_idx_;
    }

 private:
    std::shared_ptr<T> buffers_[2];
    std::atomic<int> read_idx_;
};