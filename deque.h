#pragma once

#include <initializer_list>
#include <algorithm>
#include <memory>

const int kBlocksize = 128;

class Deque {
public:
    Deque()
        : cnt_blocks_(1),
          first_block_(0),
          last_block_(0),
          pos_begin_(0),
          pos_end_(0),
          data_(new std::unique_ptr<int[]>[1]) {
        std::unique_ptr<int[]> cur_ptr(new int[kBlocksize]);
        data_[0].swap(cur_ptr);
    }

    Deque(const Deque& rhs)
        : cnt_blocks_(rhs.cnt_blocks_),
          first_block_(rhs.first_block_),
          last_block_(rhs.last_block_),
          pos_begin_(rhs.pos_begin_),
          pos_end_(rhs.pos_end_),
          data_(new std::unique_ptr<int[]>[rhs.cnt_blocks_]) {
        for (int i = 0; i < rhs.cnt_blocks_; i++) {
            std::unique_ptr<int[]> cur_ptr(new int[kBlocksize]);
            for (int e = 0; e < kBlocksize; e++) {
                cur_ptr[e] = rhs.data_[i][e];
            }
            data_[i].swap(cur_ptr);
        }
    }

    Deque(Deque&& rhs)
        : cnt_blocks_(rhs.cnt_blocks_),
          first_block_(rhs.first_block_),
          last_block_(rhs.last_block_),
          pos_begin_(rhs.pos_begin_),
          pos_end_(rhs.pos_end_) {
        data_.swap(rhs.data_);
    }

    explicit Deque(size_t size)
        : cnt_blocks_((size + kBlocksize - 1) / kBlocksize),
          first_block_(0),
          last_block_((size + kBlocksize - 1) / kBlocksize - 1),
          pos_begin_(0),
          pos_end_((size + kBlocksize - 1) % kBlocksize + 1),
          data_(new std::unique_ptr<int[]>[(size + kBlocksize - 1) / kBlocksize]) {
        int cnt_elems = size;
        for (int i = 0; i < cnt_blocks_; i++) {
            std::unique_ptr<int[]> cur_ptr(new int[kBlocksize]);
            for (int e = 0; e < std::min(cnt_elems, kBlocksize); e++) {
                cur_ptr[e] = 0;
            }
            cnt_elems -= kBlocksize;
            data_[i].swap(cur_ptr);
        }
    }

    Deque(std::initializer_list<int> list)
        : cnt_blocks_((list.size() + kBlocksize - 1) / kBlocksize),
          first_block_(0),
          last_block_((list.size() + kBlocksize - 1) / kBlocksize - 1),
          pos_begin_(0),
          pos_end_((list.size() + kBlocksize - 1) % kBlocksize + 1),
          data_(new std::unique_ptr<int[]>[(list.size() + kBlocksize - 1) / kBlocksize]) {
        int cnt_elems = list.size();
        auto list_ptr = list.begin();
        for (int i = 0; i < cnt_blocks_; i++) {
            std::unique_ptr<int[]> cur_ptr(new int[kBlocksize]);
            for (int e = 0; e < std::min(cnt_elems, kBlocksize); e++) {
                cur_ptr[e] = *list_ptr;
                list_ptr++;
            }
            cnt_elems -= kBlocksize;
            data_[i].swap(cur_ptr);
        }
    }

    Deque& operator=(Deque rhs) {
        Swap(rhs);
        return *this;
    }

    void Swap(Deque& rhs) {
        std::swap(cnt_blocks_, rhs.cnt_blocks_);
        std::swap(first_block_, rhs.first_block_);
        std::swap(last_block_, rhs.last_block_);
        std::swap(pos_begin_, rhs.pos_begin_);
        std::swap(pos_end_, rhs.pos_end_);
        data_.swap(rhs.data_);
    }

    void PushBack(int value) {
        if (pos_end_ != kBlocksize) {
            data_[last_block_][pos_end_] = value;
            pos_end_++;
            return;
        }
        if ((last_block_ + 1) % cnt_blocks_ != first_block_) {
            last_block_ = (last_block_ + 1) % cnt_blocks_;
            data_[last_block_][0] = value;
            pos_end_ = 1;
            return;
        }
        std::unique_ptr<std::unique_ptr<int[]>[]> new_data(
            new std::unique_ptr<int[]>[cnt_blocks_ + 1]);
        for (int i = 0; i < cnt_blocks_; i++) {
            new_data[i].swap(data_[(first_block_ + i) % cnt_blocks_]);
        }
        std::unique_ptr<int[]> cur(new int[kBlocksize]);
        new_data[cnt_blocks_].swap(cur);
        new_data[cnt_blocks_][0] = value;
        first_block_ = 0;
        last_block_ = cnt_blocks_;
        cnt_blocks_++;
        pos_end_ = 1;
        data_.swap(new_data);
    }

    void PopBack() {
        pos_end_--;
        if (pos_end_ == 0) {
            if (first_block_ != last_block_) {
                last_block_ = (last_block_ + cnt_blocks_ - 1) % cnt_blocks_;
                pos_end_ = kBlocksize;
            } else {
                Clear();
            }
        }
    }

    void PushFront(int value) {
        if (pos_begin_ != 0) {
            pos_begin_--;
            data_[first_block_][pos_begin_] = value;
            return;
        }
        if ((first_block_ + cnt_blocks_ - 1) % cnt_blocks_ != last_block_) {
            first_block_ = (first_block_ + cnt_blocks_ - 1) % cnt_blocks_;
            pos_begin_ = kBlocksize - 1;
            data_[first_block_][pos_begin_] = value;
            return;
        }
        std::unique_ptr<std::unique_ptr<int[]>[]> new_data(
            new std::unique_ptr<int[]>[cnt_blocks_ + 1]);
        for (int i = 0; i < cnt_blocks_; i++) {
            new_data[i + 1].swap(data_[(first_block_ + i) % cnt_blocks_]);
        }
        std::unique_ptr<int[]> cur(new int[kBlocksize]);
        new_data[0].swap(cur);
        new_data[0][kBlocksize - 1] = value;
        first_block_ = 0;
        last_block_ = cnt_blocks_;
        cnt_blocks_++;
        pos_begin_ = kBlocksize - 1;
        data_.swap(new_data);
    }

    void PopFront() {
        pos_begin_++;
        if (pos_begin_ == kBlocksize) {
            if (first_block_ != last_block_) {
                first_block_ = (first_block_ + 1) % cnt_blocks_;
                pos_begin_ = 0;
            } else {
                Clear();
            }
        }
    }

    int& operator[](size_t ind) {
        int cur_block = (first_block_ + (pos_begin_ + ind) / kBlocksize) % cnt_blocks_;
        int cur_pos = (pos_begin_ + ind) % kBlocksize;
        return data_[cur_block][cur_pos];
    }

    int operator[](size_t ind) const {
        int cur_block = (first_block_ + (pos_begin_ + ind) / kBlocksize) % cnt_blocks_;
        int cur_pos = (pos_begin_ + ind) % kBlocksize;
        return data_[cur_block][cur_pos];
    }

    size_t Size() const {
        if (first_block_ == last_block_) {
            return pos_end_ - pos_begin_;
        }
        return ((last_block_ - first_block_ + cnt_blocks_) % cnt_blocks_ + 1) * kBlocksize -
               pos_begin_ - (kBlocksize - pos_end_);
    }

    void Clear() {
        first_block_ = 0;
        last_block_ = 0;
        pos_begin_ = 0;
        pos_end_ = 0;
    }

private:
    int cnt_blocks_;
    int first_block_;
    int last_block_;
    int pos_begin_;
    int pos_end_;
    std::unique_ptr<std::unique_ptr<int[]>[]> data_;
};
