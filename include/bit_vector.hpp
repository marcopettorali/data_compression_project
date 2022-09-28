#pragma once

#include <cassert>
#include <vector>
#include <iostream>

#include "util.hpp"

struct bit_vector {

    typedef uint64_t size_type;

    bit_vector() : m_num_bits(0) {}

    uint64_t num_bits() const {
        return m_num_bits;
    }

    inline size_t size() const {
        return m_num_bits;
    }

    inline size_type capacity() const {
        return m_num_bits;
    }

    uint64_t bytes() const {
        return sizeof(m_num_bits) + m_bits.size() * 8;
    }

    uint64_t num_64bit_words() const {
        return m_bits.size();
    }

    void swap(bit_vector& other) {
        std::swap(other.m_num_bits, m_num_bits);
        other.m_bits.swap(m_bits);
    }

    uint64_t get_bits(uint64_t pos, uint64_t len) const {
        assert(pos + len <= num_bits());

        if (!len) return 0;

        uint64_t block = pos / 64;
        uint64_t shift = pos % 64;
        uint64_t mask = -(len == 64) | ((1ULL << len) - 1);

        if (shift + len <= 64) return m_bits[block] >> shift & mask;

        return (m_bits[block] >> shift) |
               (m_bits[block + 1] << (64 - shift) & mask);
    }

    uint64_t get_word64(uint64_t pos) const {
        assert(pos < num_bits());

        uint64_t block = pos / 64;
        uint64_t shift = pos % 64;
        uint64_t word = m_bits[block] >> shift;

        if (shift and block + 1 < m_bits.size()) {
            word |= m_bits[block + 1] << (64 - shift);
        }

        return word;
    }

    void save(std::ostream& out) const {
        out.write(reinterpret_cast<char const*>(&m_num_bits),
                  sizeof(m_num_bits));
        out.write(reinterpret_cast<char const*>(m_bits.data()),
                  m_bits.size() * sizeof(uint64_t));
    }

    void load(std::istream& in) {
        in.read(reinterpret_cast<char*>(&m_num_bits), sizeof(m_num_bits));
        m_bits.resize(num_64bit_words_for(m_num_bits));
        in.read(reinterpret_cast<char*>(m_bits.data()),
                m_bits.size() * sizeof(uint64_t));
    }

    std::vector<uint64_t> const& data() const {
        return m_bits;
    }

    friend struct bit_vector_builder;

    struct unary_iterator {
        unary_iterator() : m_data(0), m_position(0), m_buf(0) {}

        unary_iterator(bit_vector const& bv, uint64_t pos = 0) {
            m_data = bv.data().data();
            m_position = pos;
            m_buf = m_data[pos >> 6];
            // clear low bits
            m_buf &= uint64_t(-1) << (pos & 63);
        }

        uint64_t position() const {
            return m_position;
        }

        uint64_t next() {
            unsigned long pos_in_word;
            uint64_t buf = m_buf;
            while (!lsb(buf, pos_in_word)) {
                m_position += 64;
                buf = m_data[m_position >> 6];
            }
            m_buf = buf & (buf - 1);  // clear LSB
            m_position = (m_position & ~uint64_t(63)) + pos_in_word;
            return m_position;
        }

        // skip to the k-th one after the current position
        void skip(uint64_t k) {
            uint64_t skipped = 0;
            uint64_t buf = m_buf;
            uint64_t w = 0;
            while (skipped + (w = popcount(buf)) <= k) {
                skipped += w;
                m_position += 64;
                buf = m_data[m_position / 64];
            }
            assert(buf);
            uint64_t pos_in_word = select_in_word(buf, k - skipped);
            m_buf = buf & (uint64_t(-1) << pos_in_word);
            m_position = (m_position & ~uint64_t(63)) + pos_in_word;
        }

        // skip to the k-th zero after the current position
        void skip0(uint64_t k) {
            uint64_t skipped = 0;
            uint64_t pos_in_word = m_position % 64;
            uint64_t buf = ~m_buf & (uint64_t(-1) << pos_in_word);
            uint64_t w = 0;
            while (skipped + (w = popcount(buf)) <= k) {
                skipped += w;
                m_position += 64;
                buf = ~m_data[m_position / 64];
            }
            assert(buf);
            pos_in_word = select_in_word(buf, k - skipped);
            m_buf = ~buf & (uint64_t(-1) << pos_in_word);
            m_position = (m_position & ~uint64_t(63)) + pos_in_word;
        }

    private:
        uint64_t const* m_data;
        uint64_t m_position;
        uint64_t m_buf;
    };

private:
    uint64_t m_num_bits;
    std::vector<uint64_t> m_bits;
};


struct bit_vector_builder {
    bit_vector_builder() {
        resize(0);
    }

    bit_vector_builder(uint64_t const& size) {
        resize(size);
    }

    uint64_t num_bits() const {
        return m_num_bits;
    }

    void resize(uint64_t num_bits) {
        m_num_bits = num_bits;
        m_bits.resize(num_64bit_words_for(num_bits), 0);
    }

    void reserve(uint64_t num_bits) {
        m_bits.reserve(num_64bit_words_for(num_bits));
    }

    void build(bit_vector& bv) {
        std::swap(m_num_bits, bv.m_num_bits);
        m_bits.swap(bv.m_bits);
    }

    inline void set(uint64_t pos, bool b = true) {
        uint64_t word = pos >> 6;
        uint64_t pos_in_word = pos & 63;
        m_bits[word] &= ~(uint64_t(1) << pos_in_word);
        m_bits[word] |= uint64_t(b) << pos_in_word;
    }

    void set_bits(uint64_t pos, uint64_t x, uint64_t len) {
        assert(pos + len <= num_bits());
        assert(len == 64 or (x >> len) == 0);
        if (len == 0) return;

        uint64_t mask = (len == 64) ? uint64_t(-1) : ((uint64_t(1) << len) - 1);
        uint64_t word = pos / 64;
        uint64_t pos_in_word = pos % 64;

        m_bits[word] &= ~(mask << pos_in_word);
        m_bits[word] |= x << pos_in_word;

        uint64_t stored = 64 - pos_in_word;
        if (stored < len) {
            m_bits[word + 1] &= ~(mask >> stored);
            m_bits[word + 1] |= x >> stored;
        }
    }

    void append_bits(uint64_t x, uint64_t len) {
        assert(len <= 64);
        if (x >> len) {
            std::cout << "--> x=" << x << " len=" << len
                      << " (x>>len)=" << (x >> len) << std::endl;
        }
        assert(len == 64 or (x >> len) == 0);  // no other bits must be set
        if (len == 0) return;

        uint64_t pos_in_word = m_num_bits % 64;
        m_num_bits += len;
        if (pos_in_word == 0) {
            m_bits.push_back(x);
        } else {
            *m_cur_word |= x << pos_in_word;
            if (len > 64 - pos_in_word) {
                m_bits.push_back(x >> (64 - pos_in_word));
            }
        }

        m_cur_word = &m_bits.back();
    }

private:
    uint64_t m_num_bits;
    std::vector<uint64_t> m_bits;
    uint64_t* m_cur_word;
};



struct bit_vector_iterator {
    bit_vector_iterator() : m_bv(nullptr) {}

    bit_vector_iterator(bit_vector const& bv, uint64_t pos = 0) : m_bv(&bv) {
        at(pos);
    }

    void at(uint64_t pos) {
        m_pos = pos;
        m_buf = 0;
        m_avail = 0;
    }

    /* return 1 byte assuming position is aligned to a 8-bit boundary  */
    inline uint64_t take_one_byte() {
        assert(m_pos % 8 == 0);
        if (m_avail == 0) fill_buf();
        uint64_t val = m_buf & 255;
        m_buf >>= 8;
        m_avail -= 8;
        m_pos += 8;
        return val;
    }

    /* return the next l bits from the current position and advance by l bits */
    inline uint64_t take(uint64_t l) {
        assert(l <= 64);
        if (m_avail < l) fill_buf();
        uint64_t val;
        if (l != 64) {
            val = m_buf & ((uint64_t(1) << l) - 1);
            m_buf >>= l;
        } else {
            val = m_buf;
        }
        m_avail -= l;
        m_pos += l;
        return val;
    }

    /* skip all zeros from the current position and
    return the number of skipped zeros */
    inline uint64_t skip_zeros() {
        uint64_t zeros = 0;
        while (m_buf == 0) {
            m_pos += m_avail;
            zeros += m_avail;
            fill_buf();
        }

        uint64_t l = lsb(m_buf);
        m_buf >>= l;
        m_buf >>= 1;
        m_avail -= l + 1;
        m_pos += l + 1;
        return zeros + l;
    }

    inline uint64_t position() const {
        return m_pos;
    }

private:
    inline void fill_buf() {
        m_buf = m_bv->get_word64(m_pos);
        m_avail = 64;
    }

    bit_vector const* m_bv;
    uint64_t m_pos;
    uint64_t m_buf;
    uint64_t m_avail;
};